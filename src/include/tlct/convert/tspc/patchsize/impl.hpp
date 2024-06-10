#pragma once

#include <algorithm>
#include <cmath>
#include <numbers>
#include <numeric>
#include <ranges>
#include <set>
#include <vector>

#include <opencv2/imgproc.hpp>

#include "neighbors.hpp"
#include "tlct/common/defines.h"
#include "tlct/config/tspc/layout.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/convert/tspc/state.hpp"
#include "tlct/helper/static_math.hpp"

namespace tlct::cvt::tspc {

namespace rgs = std::ranges;

namespace _hp {

using namespace tlct::cvt::_hp;

constexpr int INVALID_PSIZE = -1;

static inline double calcSAD(const cv::Mat& lhs, const cv::Mat& rhs) noexcept
{
    const auto sad = cv::sum(cv::abs(lhs - rhs));
    const double mean_sad = sad[0] / (double)lhs.size().area();
    return mean_sad;
}

static inline double calcSADWithTwoPoints(const cv::Mat& gray_src, const cv::Point2d lhs_center,
                                          const cv::Point2d rhs_center, const double upsampled_ksize) noexcept
{
    const auto& lhs_roi = getRoiImageByCenter(gray_src, lhs_center, upsampled_ksize);
    const auto& rhs_roi = getRoiImageByCenter(gray_src, rhs_center, upsampled_ksize);
    return calcSAD(lhs_roi, rhs_roi);
}

static inline bool isSafeShift(const cv::Point2d shift, const double radius, const double upsampled_ksize) noexcept
{
    const double half_ksize = upsampled_ksize / 2.0;
    if (shift.x < -radius + half_ksize) {
        return false;
    }
    if (shift.y < -radius + half_ksize) {
        return false;
    }
    if (shift.x > radius - half_ksize) {
        return false;
    }
    if (shift.y > radius - half_ksize) {
        return false;
    }
    return true;
}

static inline double calcSADWithPsize(const cfg::tspc::Layout& layout, const cv::Mat& gray_src,
                                      const cv::Point2d curr_center, const cv::Point2d curr_shift,
                                      const NeibMIIndices& neighbors, const int psize,
                                      const double upsampled_ksize) noexcept
{
    constexpr double x_unit_shift = 0.5;
    constexpr double y_unit_shift = std::numbers::sqrt3 / 2.0;
    const cv::Point2d anchor = curr_center + curr_shift;

    std::array<double, NeibMIIndices::NEIB_NUM> sads{};
    std::fill(sads.begin(), sads.end(), INVALID_PSIZE);
    int sad_num = 0;
    if (neighbors.hasLeft()) {
        cv::Point2d cmp_shift = curr_shift;
        cmp_shift.x -= (double)psize;
        if (isSafeShift(cmp_shift, layout.getRadius(), upsampled_ksize)) {
            const cv::Point2d cmp_center = layout.getMICenter(neighbors.left) + cmp_shift;
            const double sad = calcSADWithTwoPoints(gray_src, anchor, cmp_center, upsampled_ksize);
            sads[sad_num] = sad;
            sad_num++;
        }
    }
    if (neighbors.hasRight()) {
        cv::Point2d cmp_shift = curr_shift;
        cmp_shift.x += (double)psize;
        if (isSafeShift(cmp_shift, layout.getRadius(), upsampled_ksize)) {
            const cv::Point2d cmp_center = layout.getMICenter(neighbors.right) + cmp_shift;
            const double sad = calcSADWithTwoPoints(gray_src, anchor, cmp_center, upsampled_ksize);
            sads[sad_num] = sad;
            sad_num++;
        }
    }
    if (neighbors.hasUpLeft()) {
        cv::Point2d cmp_shift = curr_shift;
        cmp_shift.x -= (double)psize * x_unit_shift;
        cmp_shift.y -= (double)psize * y_unit_shift;
        if (isSafeShift(cmp_shift, layout.getRadius(), upsampled_ksize)) {
            const cv::Point2d cmp_center = layout.getMICenter(neighbors.upleft) + cmp_shift;
            const double sad = calcSADWithTwoPoints(gray_src, anchor, cmp_center, upsampled_ksize);
            sads[sad_num] = sad;
            sad_num++;
        }
    }
    if (neighbors.hasUpRight()) {
        cv::Point2d cmp_shift = curr_shift;
        cmp_shift.x += (double)psize * x_unit_shift;
        cmp_shift.y -= (double)psize * y_unit_shift;
        if (isSafeShift(cmp_shift, layout.getRadius(), upsampled_ksize)) {
            const cv::Point2d cmp_center = layout.getMICenter(neighbors.upright) + cmp_shift;
            const double sad = calcSADWithTwoPoints(gray_src, anchor, cmp_center, upsampled_ksize);
            sads[sad_num] = sad;
            sad_num++;
        }
    }
    if (neighbors.hasDownLeft()) {
        cv::Point2d cmp_shift = curr_shift;
        cmp_shift.x -= (double)psize * x_unit_shift;
        cmp_shift.y += (double)psize * y_unit_shift;
        if (isSafeShift(cmp_shift, layout.getRadius(), upsampled_ksize)) {
            const cv::Point2d cmp_center = layout.getMICenter(neighbors.downleft) + cmp_shift;
            const double sad = calcSADWithTwoPoints(gray_src, anchor, cmp_center, upsampled_ksize);
            sads[sad_num] = sad;
            sad_num++;
        }
    }
    if (neighbors.hasDownRight()) {
        cv::Point2d cmp_shift = layout.getMICenter(neighbors.downright);
        cmp_shift.x += (double)psize * x_unit_shift;
        cmp_shift.y += (double)psize * y_unit_shift;
        if (isSafeShift(cmp_shift, layout.getRadius(), upsampled_ksize)) {
            const cv::Point2d cmp_center = layout.getMICenter(neighbors.downright) + cmp_shift;
            const double sad = calcSADWithTwoPoints(gray_src, anchor, cmp_center, upsampled_ksize);
            sads[sad_num] = sad;
            sad_num++;
        }
    }

    std::sort(sads.begin(), sads.begin() + sad_num);
    const double mid_sad = sads[sad_num / 2];
    return mid_sad;
}

template <bool left_and_up_only = true>
static inline std::vector<int> getRefPsizes(const cv::Mat& psizes, const NeibMIIndices& neighbors)
{
    std::set<int> ref_psizes_set;

    if (neighbors.hasLeft()) {
        const int psize = psizes.at<int>(neighbors.left);
        ref_psizes_set.insert(psize);
    }
    if (neighbors.hasUpLeft()) {
        const int psize = psizes.at<int>(neighbors.upleft);
        ref_psizes_set.insert(psize);
    }
    if (neighbors.hasUpRight()) {
        const int psize = psizes.at<int>(neighbors.upright);
        ref_psizes_set.insert(psize);
    }

    if constexpr (!left_and_up_only) {
        if (neighbors.hasRight()) {
            const int psize = psizes.at<int>(neighbors.right);
            ref_psizes_set.insert(psize);
        }
        if (neighbors.hasDownLeft()) {
            const int psize = psizes.at<int>(neighbors.downleft);
            ref_psizes_set.insert(psize);
        }
        if (neighbors.hasDownRight()) {
            const int psize = psizes.at<int>(neighbors.downright);
            ref_psizes_set.insert(psize);
        }
    }

    std::vector<int> ref_psizes(ref_psizes_set.begin(), ref_psizes_set.end());
    return std::move(ref_psizes);
}

static inline int estimatePatchsizeOverFullMatch(const cfg::tspc::Layout& layout, const cv::Mat& gray_src,
                                                 const cv::Point2d curr_center, const cv::Point2d curr_shift,
                                                 const NeibMIIndices& neighbors, const double upsampled_ksize,
                                                 const double sad_threshold) noexcept
{
    constexpr double lean_xstep = 0.5;
    constexpr double lean_ystep = std::numbers::sqrt3 / 2.0;
    const int max_valid_psize = (int)(layout.getRadius() / std::numbers::sqrt2);
    const cv::Point2d anchor = curr_center + curr_shift;

    std::array<double, NeibMIIndices::NEIB_NUM> sads{};
    std::fill(sads.begin(), sads.end(), INVALID_PSIZE);
    int sad_num = 0;

    if (neighbors.hasLeft()) {
        const cv::Point2d cmp_center = layout.getMICenter(neighbors.left);
        cv::Point2d cmp_shift = curr_shift;
        const cv::Point2d cmp_shift_step{-1.0, 0.0};

        int min_sad_psize = INVALID_PSIZE;
        double min_sad = std::numeric_limits<double>::max();
        for (const int psize : rgs::views::iota(1, max_valid_psize)) {
            cmp_shift += cmp_shift_step;
            if (!isSafeShift(cmp_shift, layout.getRadius(), upsampled_ksize)) {
                break;
            }
            const double sad = calcSADWithTwoPoints(gray_src, anchor, cmp_center + cmp_shift, upsampled_ksize);
            if (sad < sad_threshold && sad < min_sad) {
                min_sad = sad;
                min_sad_psize = psize;
            }
        }

        if (min_sad_psize != INVALID_PSIZE) {
            sads[sad_num] = min_sad_psize;
            sad_num++;
        }
    }

    if (neighbors.hasRight()) {
        const cv::Point2d cmp_center = layout.getMICenter(neighbors.right);
        cv::Point2d cmp_shift = curr_shift;
        const cv::Point2d cmp_shift_step{1.0, 0.0};

        int min_sad_psize = INVALID_PSIZE;
        double min_sad = std::numeric_limits<double>::max();
        for (const int psize : rgs::views::iota(1, max_valid_psize)) {
            cmp_shift += cmp_shift_step;
            if (!isSafeShift(cmp_shift, layout.getRadius(), upsampled_ksize)) {
                break;
            }
            const double sad = calcSADWithTwoPoints(gray_src, anchor, cmp_center + cmp_shift, upsampled_ksize);
            if (sad < sad_threshold && sad < min_sad) {
                min_sad = sad;
                min_sad_psize = psize;
            }
        }

        if (min_sad_psize != INVALID_PSIZE) {
            sads[sad_num] = min_sad_psize;
            sad_num++;
        }
    }

    if (neighbors.hasUpLeft()) {
        const cv::Point2d cmp_center = layout.getMICenter(neighbors.upleft);
        cv::Point2d cmp_shift = curr_shift;
        const cv::Point2d cmp_shift_step{-lean_xstep, -lean_ystep};

        int min_sad_psize = INVALID_PSIZE;
        double min_sad = std::numeric_limits<double>::max();
        for (const int psize : rgs::views::iota(1, max_valid_psize)) {
            cmp_shift += cmp_shift_step;
            if (!isSafeShift(cmp_shift, layout.getRadius(), upsampled_ksize)) {
                break;
            }
            const double sad = calcSADWithTwoPoints(gray_src, anchor, cmp_center + cmp_shift, upsampled_ksize);
            if (sad < sad_threshold && sad < min_sad) {
                min_sad = sad;
                min_sad_psize = psize;
            }
        }

        if (min_sad_psize != INVALID_PSIZE) {
            sads[sad_num] = min_sad_psize;
            sad_num++;
        }
    }

    if (neighbors.hasUpRight()) {
        const cv::Point2d cmp_center = layout.getMICenter(neighbors.upright);
        cv::Point2d cmp_shift = curr_shift;
        const cv::Point2d cmp_shift_step{lean_xstep, -lean_ystep};

        int min_sad_psize = INVALID_PSIZE;
        double min_sad = std::numeric_limits<double>::max();
        for (const int psize : rgs::views::iota(1, max_valid_psize)) {
            cmp_shift += cmp_shift_step;
            if (!isSafeShift(cmp_shift, layout.getRadius(), upsampled_ksize)) {
                break;
            }
            const double sad = calcSADWithTwoPoints(gray_src, anchor, cmp_center + cmp_shift, upsampled_ksize);
            if (sad < sad_threshold && sad < min_sad) {
                min_sad = sad;
                min_sad_psize = psize;
            }
        }

        if (min_sad_psize != INVALID_PSIZE) {
            sads[sad_num] = min_sad_psize;
            sad_num++;
        }
    }

    if (neighbors.hasDownLeft()) {
        const cv::Point2d cmp_center = layout.getMICenter(neighbors.downleft);
        cv::Point2d cmp_shift = curr_shift;
        const cv::Point2d cmp_shift_step{-lean_xstep, lean_ystep};

        int min_sad_psize = INVALID_PSIZE;
        double min_sad = std::numeric_limits<double>::max();
        for (const int psize : rgs::views::iota(1, max_valid_psize)) {
            cmp_shift += cmp_shift_step;
            if (!isSafeShift(cmp_shift, layout.getRadius(), upsampled_ksize)) {
                break;
            }
            const double sad = calcSADWithTwoPoints(gray_src, anchor, cmp_center + cmp_shift, upsampled_ksize);
            if (sad < sad_threshold && sad < min_sad) {
                min_sad = sad;
                min_sad_psize = psize;
            }
        }

        if (min_sad_psize != INVALID_PSIZE) {
            sads[sad_num] = min_sad_psize;
            sad_num++;
        }
    }

    if (neighbors.hasDownRight()) {
        const cv::Point2d cmp_center = layout.getMICenter(neighbors.downright);
        cv::Point2d cmp_shift = curr_shift;
        const cv::Point2d cmp_shift_step{lean_xstep, lean_ystep};

        int min_sad_psize = INVALID_PSIZE;
        double min_sad = std::numeric_limits<double>::max();
        for (const int psize : rgs::views::iota(1, max_valid_psize)) {
            cmp_shift += cmp_shift_step;
            if (!isSafeShift(cmp_shift, layout.getRadius(), upsampled_ksize)) {
                break;
            }
            const double sad = calcSADWithTwoPoints(gray_src, anchor, cmp_center + cmp_shift, upsampled_ksize);
            if (sad < sad_threshold && sad < min_sad) {
                min_sad = sad;
                min_sad_psize = psize;
            }
        }

        if (min_sad_psize != INVALID_PSIZE) {
            sads[sad_num] = min_sad_psize;
            sad_num++;
        }
    }

    std::sort(sads.begin(), sads.begin() + sad_num);
    const double mid_sad = sads[sad_num / 2];
    return mid_sad;
}

static inline int estimatePatchsize(const cfg::tspc::Layout& layout, const cv::Mat& gray_src, const cv::Mat& psizes,
                                    const cv::Mat& prev_psizes, const PixHeaps& features, const cv::Point index)
{
    constexpr int ksize = 13;
    const int upsampled_ksize = ksize * layout.getUpsample();
    constexpr double ref_sad_threshold = 7.5;

    const cv::Point2d curr_center = layout.getMICenter(index);
    const auto neighbors = _hp::getNeibMIIndices(layout, index);

    const Pixel feature_point = *features[index.y][index.x].begin();
    const cv::Point2d curr_shift = (cv::Point2d)feature_point.pt - cv::Point2d(layout.getRadius(), layout.getRadius());

    const int prev_psize = prev_psizes.at<int>(index);
    const double prev_sad =
        calcSADWithPsize(layout, gray_src, curr_center, curr_shift, neighbors, prev_psize, upsampled_ksize);
    if (prev_sad < ref_sad_threshold) {
        return prev_psize;
    }

    const std::vector<int>& ref_psizes = getRefPsizes(psizes, neighbors);
    double min_ref_sad = std::numeric_limits<double>::max();
    int min_ref_psize = 0;
    for (const int ref_psize : ref_psizes) {
        const double ref_sad =
            calcSADWithPsize(layout, gray_src, curr_center, curr_shift, neighbors, prev_psize, upsampled_ksize);
        if (ref_sad < min_ref_sad) {
            min_ref_sad = ref_sad;
            min_ref_psize = ref_psize;
        }
    }
    if (min_ref_sad < ref_sad_threshold) {
        return min_ref_psize;
    }

    const std::vector<int>& prev_ref_psizes = getRefPsizes<false>(prev_psizes, neighbors);
    double min_prev_ref_sad = std::numeric_limits<double>::max();
    int min_prev_ref_psize = 0;
    for (const int prev_ref_psize : prev_ref_psizes) {
        const double prev_ref_sad =
            calcSADWithPsize(layout, gray_src, curr_center, curr_shift, neighbors, prev_psize, upsampled_ksize);
        if (prev_ref_sad < min_prev_ref_sad) {
            min_prev_ref_sad = prev_ref_sad;
            min_prev_ref_psize = prev_ref_psize;
        }
    }
    if (min_prev_ref_sad < ref_sad_threshold) {
        return min_prev_ref_psize;
    }

    const int psize =
        estimatePatchsizeOverFullMatch(layout, gray_src, curr_center, curr_shift, neighbors, upsampled_ksize, 7.5);

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

    cv::Mat psizes = cv::Mat::ones(layout.getMIRows(), layout.getMIMaxCols(), CV_32SC1) * 13;
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