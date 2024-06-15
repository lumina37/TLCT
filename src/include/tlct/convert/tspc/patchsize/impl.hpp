#pragma once

#include <algorithm>
#include <cmath>
#include <numeric>
#include <ranges>
#include <set>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/quality.hpp>

#include "neighbors.hpp"
#include "tlct/common/defines.h"
#include "tlct/config/tspc/layout.hpp"
#include "tlct/convert/helper/direction.hpp"
#include "tlct/convert/helper/roi.hpp"
#include "tlct/convert/tspc/state.hpp"
#include "wrapper.hpp"

namespace tlct::cvt::tspc {

namespace rgs = std::ranges;

namespace _hp {

using namespace tlct::cvt::_hp;

constexpr int INVALID_PSIZE = -1;

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

static inline double stdvar(const std::vector<double>& arr) noexcept
{
    const double sum = std::accumulate(arr.begin(), arr.end(), 0.0);
    const double mean = sum / (double)arr.size();
    double var = 0.0;
    for (const double elem : arr) {
        var += (elem - mean) * (elem - mean);
    }
    var /= (double)arr.size();
    const double stdvar = std::sqrt(var);
    return stdvar;
}

static inline double calcMetricWithPsize(const cfg::tspc::Layout& layout, const cv::Mat& gray_src,
                                         const cv::Point index, const NeibMIIndices& neighbors, const int psize,
                                         const double ksize) noexcept
{
    const cv::Point2d curr_center = layout.getMICenter(index);

    const auto match_shifts = MatchShifts::fromDiamAndKsize(layout.getDiameter(), ksize);

    double max_metric = std::numeric_limits<double>::min();

    auto calcWithNeib = [&]<Direction direction>() mutable {
        if (neighbors.hasNeighbor<direction>()) {
            const cv::Point2d anchor_shift = -match_shifts.getMatchShift<direction>();
            const cv::Point2d match_step = MatchSteps::getMatchStep<direction>();
            const cv::Point2d cmp_shift = anchor_shift + match_step * psize;

            const cv::Point2d anchor_center = curr_center + anchor_shift;
            const cv::Point2d neib_center = layout.getMICenter(neighbors.getNeighbor<direction>());
            const cv::Point2d cmp_center = neib_center + cmp_shift;

            const auto& anchor = AnchorWrapper::fromRoi(getRoiImageByCenter(gray_src, anchor_center, ksize));
            const auto& rhs = getRoiImageByCenter(gray_src, cmp_center, ksize);

            const double metric = anchor.compare(rhs);
            if (metric > max_metric) {
                max_metric = metric;
            }
        }
    };

    calcWithNeib.template operator()<Direction::LEFT>();
    calcWithNeib.template operator()<Direction::RIGHT>();
    calcWithNeib.template operator()<Direction::UPLEFT>();
    calcWithNeib.template operator()<Direction::UPRIGHT>();
    calcWithNeib.template operator()<Direction::DOWNLEFT>();
    calcWithNeib.template operator()<Direction::DOWNRIGHT>();

    return max_metric;
}

static inline int estimatePatchsizeOverFullMatch(const cfg::tspc::Layout& layout, const cv::Mat& gray_src,
                                                 const cv::Point index, const NeibMIIndices& neighbors,
                                                 const double ksize, Inspector& inspector) noexcept
{
    const cv::Point2d curr_center = layout.getMICenter(index);

    if (Inspector::PATTERN_ENABLED) {
        Inspector::saveMI(inspector, getRoiImageByCenter(gray_src, curr_center, layout.getDiameter()), index);
    }

    const auto match_shifts = MatchShifts::fromDiamAndKsize(layout.getDiameter(), ksize);
    const double safe_radius = layout.getRadius() * 0.9;
    const double half_ksize = ksize / 2.0;
    const int max_shift =
        (int)(match_shifts.getRight().x + std::sqrt(safe_radius * safe_radius - half_ksize * half_ksize) - half_ksize);

    double weighted_psize = 0.0;
    double total_weight = 0.0;
    std::vector<double> psizes, weights;

    auto calcWithNeib = [&]<Direction direction>() mutable {
        if (neighbors.hasNeighbor<direction>()) {
            const cv::Point2d anchor_shift = -match_shifts.getMatchShift<direction>();
            const cv::Point2d anchor_center = curr_center + anchor_shift;
            const cv::Point2d match_step = MatchSteps::getMatchStep<direction>();
            const auto& anchor = AnchorWrapper::fromRoi(getRoiImageByCenter(gray_src, anchor_center, ksize));

            if (Inspector::PATTERN_ENABLED) {
                Inspector::saveAnchor(inspector, getRoiImageByCenter(gray_src, anchor_center, ksize), index, direction);
            }

            const cv::Point2d neib_center = layout.getMICenter(neighbors.getNeighbor<direction>());
            cv::Point2d cmp_shift = anchor_shift;

            int min_metric_psize = INVALID_PSIZE;
            double min_metric = std::numeric_limits<double>::max();
            std::vector<double> metrics;
            metrics.reserve(max_shift - 1);
            for (const int psize : rgs::views::iota(1, max_shift)) {
                cmp_shift += match_step;
                const auto& rhs = getRoiImageByCenter(gray_src, neib_center + cmp_shift, ksize);
                const double metric = anchor.compare(rhs);
                metrics.push_back(metric);

                if (Inspector::PATTERN_ENABLED) {
                    Inspector::saveCmpPattern(inspector, getRoiImageByCenter(gray_src, neib_center + cmp_shift, ksize),
                                              index, direction, psize, metric);
                }

                if (metric < min_metric) {
                    min_metric = metric;
                    min_metric_psize = psize;
                }
            }

            constexpr double max_valid_metric = -0.7;
            constexpr double min_metric_stdvar = 0.1;
            if (min_metric < max_valid_metric && stdvar(metrics) > min_metric_stdvar) {
                weighted_psize += anchor.getWeight() * min_metric_psize;
                total_weight += anchor.getWeight();
                psizes.push_back(min_metric_psize);
                weights.push_back(anchor.getWeight());
            }
        }
    };

    calcWithNeib.template operator()<Direction::LEFT>();
    calcWithNeib.template operator()<Direction::RIGHT>();
    calcWithNeib.template operator()<Direction::UPLEFT>();
    calcWithNeib.template operator()<Direction::UPRIGHT>();
    calcWithNeib.template operator()<Direction::DOWNLEFT>();
    calcWithNeib.template operator()<Direction::DOWNRIGHT>();

    if (Inspector::METRIC_REPORT_ENABLED) {
        inspector.appendMetricReport(index, psizes, weights);
    }

    if (total_weight == 0.0)
        return INVALID_PSIZE;

    const int psize = (int)std::round(weighted_psize / total_weight);
    return psize;
}

static inline int estimatePatchsize(const cfg::tspc::Layout& layout, const cv::Mat& gray_src, const cv::Mat& psizes,
                                    const cv::Mat& prev_psizes, const cv::Point index, Inspector& inspector)
{
    const int ksize = (int)(25.0 / 70.0 * layout.getDiameter());
    constexpr double ref_metric_threshold = -0.9;

    const auto neighbors = NeibMIIndices::fromLayoutAndIndex(layout, index);

    const int prev_psize = prev_psizes.at<int>(index);
    const double prev_metric = calcMetricWithPsize(layout, gray_src, index, neighbors, prev_psize, ksize);
    if (prev_metric < ref_metric_threshold) {
        return prev_psize;
    }

    const std::vector<int>& ref_psizes = getRefPsizes(psizes, neighbors);
    double min_ref_metric = std::numeric_limits<double>::max();
    int min_ref_psize = 0;
    for (const int ref_psize : ref_psizes) {
        const double ref_metric = calcMetricWithPsize(layout, gray_src, index, neighbors, prev_psize, ksize);
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
        const double prev_ref_metric = calcMetricWithPsize(layout, gray_src, index, neighbors, prev_psize, ksize);
        if (prev_ref_metric < min_prev_ref_metric) {
            min_prev_ref_metric = prev_ref_metric;
            min_prev_ref_psize = prev_ref_psize;
        }
    }
    if (min_prev_ref_metric < ref_metric_threshold) {
        return min_prev_ref_psize;
    }

    const int psize = estimatePatchsizeOverFullMatch(layout, gray_src, index, neighbors, ksize, inspector);

    if (psize == INVALID_PSIZE) {
        return prev_psize;
    } else {
        return psize;
    }
}

} // namespace _hp

TLCT_API inline cv::Mat estimatePatchsizes(State& state)
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
            const cv::Point index{col, row};
            const int psize =
                _hp::estimatePatchsize(layout, state.gray_src_, psizes, prev_psizes, index, state.inspector_);
            psizes.at<int>(index) = psize;
        }
    }
    psizes.col(layout.getMIMinCols() - 2).copyTo(psizes.col(layout.getMIMinCols() - 1));

    return std::move(psizes);
}

} // namespace tlct::cvt::tspc