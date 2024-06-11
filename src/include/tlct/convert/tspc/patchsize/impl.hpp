#pragma once

#include <algorithm>
#include <numbers>
#include <numeric>
#include <ranges>
#include <set>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/quality.hpp>

#include "neighbors.hpp"
#include "tlct/common/defines.h"
#include "tlct/config/tspc/layout.hpp"
#include "tlct/convert/common/direction.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/convert/tspc/state.hpp"

namespace tlct::cvt::tspc {

namespace rgs = std::ranges;

namespace _hp {

using namespace tlct::cvt::_hp;

constexpr int INVALID_PSIZE = -1;

static inline double calcMetric(const cv::Mat& lhs, const cv::Mat& rhs) noexcept
{
    const auto ssims = cv::quality::QualitySSIM::compute(lhs, rhs, cv::noArray());
    const double metric = -ssims[0];
    return metric;
}

static inline double calcMetricWithTwoPoints(const cv::Mat& gray_src, const cv::Point2d lhs_center,
                                             const cv::Point2d rhs_center, const double upsampled_ksize) noexcept
{
    const auto& lhs_roi = getRoiImageByCenter(gray_src, lhs_center, upsampled_ksize);
    const auto& rhs_roi = getRoiImageByCenter(gray_src, rhs_center, upsampled_ksize);
    return calcMetric(lhs_roi, rhs_roi);
}

static inline bool isSafeShift(const cv::Point2d shift, const double half_inscribed_sqr_size,
                               const double upsampled_ksize) noexcept
{
    const double half_ksize = upsampled_ksize / 2.0;
    if (shift.x < -half_inscribed_sqr_size + half_ksize) {
        return false;
    }
    if (shift.y < -half_inscribed_sqr_size + half_ksize) {
        return false;
    }
    if (shift.x > half_inscribed_sqr_size - half_ksize) {
        return false;
    }
    if (shift.y > half_inscribed_sqr_size - half_ksize) {
        return false;
    }
    return true;
}

static inline double calcSADWithPsize(const cfg::tspc::Layout& layout, const cv::Mat& gray_src,
                                      const cv::Point2d curr_center, const NeibMIIndices& neighbors, const int psize,
                                      const double ksize) noexcept
{
    const double half_inscribed_sqr_size = layout.getRadius() / std::numbers::sqrt2;
    const auto match_shifts = MatchShifts::fromDiameter(layout.getDiameter());

    std::array<double, DIRECTION_NUM> metrics{};
    std::fill(metrics.begin(), metrics.end(), INVALID_PSIZE);
    int metric_num = 0;

    auto calcWithNeib = [&]<Direction direction>() mutable {
        if (neighbors.hasNeighbor<direction>()) {
            const cv::Point2d anchor_shift = -match_shifts.getMatchShift<direction>();
            const cv::Point2d match_step = MatchSteps::getMatchStep<direction>();
            const cv::Point2d cmp_shift = anchor_shift + match_step * psize;

            if (isSafeShift(cmp_shift, half_inscribed_sqr_size, ksize)) {
                const cv::Point2d anchor = curr_center + anchor_shift;
                const cv::Point2d neib_center = layout.getMICenter(neighbors.getNeighbor<direction>());
                const cv::Point2d cmp_center = neib_center + cmp_shift;

                const double metric = calcMetricWithTwoPoints(gray_src, anchor, cmp_center, ksize);
                metrics[metric_num] = metric;
                metric_num++;
            }
        }
    };

    calcWithNeib.template operator()<Direction::LEFT>();
    calcWithNeib.template operator()<Direction::RIGHT>();
    calcWithNeib.template operator()<Direction::UPLEFT>();
    calcWithNeib.template operator()<Direction::UPRIGHT>();
    calcWithNeib.template operator()<Direction::DOWNLEFT>();
    calcWithNeib.template operator()<Direction::DOWNRIGHT>();

    std::sort(metrics.begin(), metrics.begin() + metric_num);
    const double mid_metric = metrics[metric_num / 2];
    return mid_metric;
}

template <bool left_and_up_only = true>
static inline std::vector<int> getRefPsizes(const cv::Mat& psizes, const NeibMIIndices& neighbors)
{
    std::set<int> ref_psizes_set;

    if (neighbors.hasLeft()) {
        const int psize = psizes.at<int>(neighbors.getLeft());
        ref_psizes_set.insert(psize);
    }
    if (neighbors.hasUpLeft()) {
        const int psize = psizes.at<int>(neighbors.getUpLeft());
        ref_psizes_set.insert(psize);
    }
    if (neighbors.hasUpRight()) {
        const int psize = psizes.at<int>(neighbors.getUpRight());
        ref_psizes_set.insert(psize);
    }

    if constexpr (!left_and_up_only) {
        if (neighbors.hasRight()) {
            const int psize = psizes.at<int>(neighbors.getRight());
            ref_psizes_set.insert(psize);
        }
        if (neighbors.hasDownLeft()) {
            const int psize = psizes.at<int>(neighbors.getDownLeft());
            ref_psizes_set.insert(psize);
        }
        if (neighbors.hasDownRight()) {
            const int psize = psizes.at<int>(neighbors.getDownRight());
            ref_psizes_set.insert(psize);
        }
    }

    std::vector<int> ref_psizes(ref_psizes_set.begin(), ref_psizes_set.end());
    return std::move(ref_psizes);
}

static inline int estimatePatchsizeOverFullMatch(const cfg::tspc::Layout& layout, const cv::Mat& gray_src,
                                                 const cv::Point2d curr_center, const NeibMIIndices& neighbors,
                                                 const double ksize, const double metric_thre) noexcept
{
    const double half_inscribed_sqr_size = layout.getRadius() / std::numbers::sqrt2;
    const int max_valid_psize = (int)half_inscribed_sqr_size;
    const auto match_shifts = MatchShifts::fromDiameter(layout.getDiameter());

    std::array<int, DIRECTION_NUM> psizes{};
    std::fill(psizes.begin(), psizes.end(), INVALID_PSIZE);
    int metric_num = 0;

    auto calcWithNeib = [&]<Direction direction>() mutable {
        if (neighbors.hasNeighbor<direction>()) {
            const cv::Point2d anchor_shift = -match_shifts.getMatchShift<direction>();
            const cv::Point2d anchor = curr_center + anchor_shift;
            const cv::Point2d match_step = MatchSteps::getMatchStep<direction>();

            const cv::Point2d neib_center = layout.getMICenter(neighbors.getNeighbor<direction>());
            cv::Point2d cmp_shift = anchor_shift;

            int min_metric_psize = INVALID_PSIZE;
            double min_metric = std::numeric_limits<double>::max();
            for (const int psize : rgs::views::iota(1, max_valid_psize)) {
                cmp_shift += match_step;
                if (!isSafeShift(cmp_shift, half_inscribed_sqr_size, ksize)) {
                    break;
                }
                const double metric = calcMetricWithTwoPoints(gray_src, anchor, neib_center + cmp_shift, ksize);
                if (metric < metric_thre && metric < min_metric) {
                    min_metric = metric;
                    min_metric_psize = psize;
                }
            }

            if (min_metric_psize != INVALID_PSIZE) {
                psizes[metric_num] = min_metric_psize;
                metric_num++;
            }
        }
    };

    calcWithNeib.template operator()<Direction::LEFT>();
    calcWithNeib.template operator()<Direction::RIGHT>();
    calcWithNeib.template operator()<Direction::UPLEFT>();
    calcWithNeib.template operator()<Direction::UPRIGHT>();
    calcWithNeib.template operator()<Direction::DOWNLEFT>();
    calcWithNeib.template operator()<Direction::DOWNRIGHT>();

    std::sort(psizes.begin(), psizes.begin() + metric_num);
    const int mid_psize = psizes[metric_num / 2];
    return mid_psize;
}

static inline int estimatePatchsize(const cfg::tspc::Layout& layout, const cv::Mat& gray_src, const cv::Mat& psizes,
                                    const cv::Mat& prev_psizes, const PixHeaps& features, const cv::Point index)
{
    const int ksize = (int)(9.0 / 70.0 * layout.getDiameter());
    constexpr double ref_metric_threshold = -0.875;

    const cv::Point2d curr_center = layout.getMICenter(index);
    const auto neighbors = NeibMIIndices::fromLayoutAndIndex(layout, index);

    const int prev_psize = prev_psizes.at<int>(index);
    const double prev_metric = calcSADWithPsize(layout, gray_src, curr_center, neighbors, prev_psize, ksize);
    if (prev_metric < ref_metric_threshold) {
        return prev_psize;
    }

    const std::vector<int>& ref_psizes = getRefPsizes(psizes, neighbors);
    double min_ref_metric = std::numeric_limits<double>::max();
    int min_ref_psize = 0;
    for (const int ref_psize : ref_psizes) {
        const double ref_metric = calcSADWithPsize(layout, gray_src, curr_center, neighbors, prev_psize, ksize);
        if (ref_metric < min_ref_metric) {
            min_ref_metric = ref_metric;
            min_ref_psize = ref_psize;
        }
    }
    if (min_ref_metric < ref_metric_threshold) {
        return min_ref_psize;
    }

    const std::vector<int>& prev_ref_psizes = getRefPsizes<false>(prev_psizes, neighbors);
    double min_prev_ref_metric = std::numeric_limits<double>::max();
    int min_prev_ref_psize = 0;
    for (const int prev_ref_psize : prev_ref_psizes) {
        const double prev_ref_metric = calcSADWithPsize(layout, gray_src, curr_center, neighbors, prev_psize, ksize);
        if (prev_ref_metric < min_prev_ref_metric) {
            min_prev_ref_metric = prev_ref_metric;
            min_prev_ref_psize = prev_ref_psize;
        }
    }
    if (min_prev_ref_metric < ref_metric_threshold) {
        return min_prev_ref_psize;
    }

    const int psize = estimatePatchsizeOverFullMatch(layout, gray_src, curr_center, neighbors, ksize, -0.875);

    if (psize == INVALID_PSIZE) {
        return prev_psize;
    } else {
        return psize;
    }
}

} // namespace _hp

cv::Mat estimatePatchsizes(const State& state)
{
    const auto& layout = state.layout_;

    cv::Mat psizes = cv::Mat::ones(layout.getMIRows(), layout.getMIMaxCols(), CV_32SC1);
    cv::Mat prev_psizes;
    if (!state.prev_patchsizes_.empty()) {
        prev_psizes = state.prev_patchsizes_;
    } else {
        prev_psizes = psizes.clone();
    }

    for (const int row : rgs::views::iota(0, layout.getMIRows())) {
        for (const int col : rgs::views::iota(0, layout.getMICols(row) - 1)) {
            const cv::Point2d neib_center = layout.getMICenter(row, col + 1);
            if (neib_center.x == 0.0 or neib_center.y == 0.0)
                continue;

            const cv::Point index{col, row};
            const int psize =
                _hp::estimatePatchsize(layout, state.gray_src_, psizes, prev_psizes, state.features_, index);
            psizes.at<int>(index) = psize;
        }
    }
    psizes.col(layout.getMIMinCols() - 2).copyTo(psizes.col(layout.getMIMinCols() - 1));

    return std::move(psizes);
}

} // namespace tlct::cvt::tspc