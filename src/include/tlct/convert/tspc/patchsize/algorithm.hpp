#pragma once

#include <algorithm>
#include <cmath>
#include <ranges>
#include <vector>

#include <opencv2/imgproc.hpp>
#include <opencv2/quality.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/tspc.hpp"
#include "tlct/convert/tspc/state.hpp"
#include "tlct/helper/static_math.hpp"

namespace tlct::cvt::tspc {

namespace rgs = std::ranges;

namespace _hp {

static inline int estimatePatchsize(const cfg::tspc::Layout& layout, const cv::Mat& gray_src,
                                    const cv::Mat& psize_indices, const cv::Mat& prev_psize_indices,
                                    const cv::Point index, const cv::Range match_range)
{
    const cv::Point2d curr_center = layout.getMICenter(index);
    const cv::Point2d neib_center = layout.getMICenter(index.y, index.x + 1);

    // Controls the area size for SSIM comparison. Selected by Guotai J.
    constexpr double cmp_row_range_shift_factor = 0.17;
    constexpr double cmp_col_range_start_factor = -0.2;
    constexpr double cmp_col_range_end_factor = -0.057;
    const int cmp_row_range_shift = (int)(cmp_row_range_shift_factor * layout.getDiameter());
    const int cmp_col_range_start = (int)(cmp_col_range_start_factor * layout.getDiameter());
    const int cmp_col_range_end = (int)(cmp_col_range_end_factor * layout.getDiameter());

    const int prev_psize_idx = prev_psize_indices.at<int>(index.y, index.x);

    const cv::Range curr_cmp_row_range{(int)std::round(curr_center.y - cmp_row_range_shift),
                                       (int)std::round(curr_center.y + cmp_row_range_shift)};
    const cv::Range curr_cmp_col_range{(int)std::round(curr_center.x + cmp_col_range_start),
                                       (int)std::round(curr_center.x + cmp_col_range_end)};
    if (curr_cmp_row_range.end > gray_src.rows || curr_cmp_col_range.end > gray_src.cols) {
        return prev_psize_idx;
    }
    const cv::Mat curr_cmp = gray_src({curr_cmp_row_range, curr_cmp_col_range});
    const auto pssim_calc = cv::quality::QualitySSIM::create(curr_cmp);

    constexpr double default_ssim = -1.0;
    std::vector<double> ssims_over_mdist(match_range.size(), default_ssim);

    auto calc_ssim_with_mdist = [&](const int mdist) mutable {
        const int mdist_idx = mdist - match_range.start;

        if (ssims_over_mdist[mdist_idx] != default_ssim) {
            return ssims_over_mdist[mdist_idx];
        }

        const cv::Range neib_cmp_row_range{(int)std::round(neib_center.y - cmp_row_range_shift),
                                           (int)std::round(neib_center.y + cmp_row_range_shift)};
        const cv::Range neib_cmp_col_range{(int)std::round(neib_center.x + cmp_col_range_start) + mdist,
                                           (int)std::round(neib_center.x + cmp_col_range_end) + mdist};
        if (neib_cmp_row_range.end > gray_src.rows || neib_cmp_col_range.end > gray_src.cols) {
            return default_ssim;
        }

        const cv::Mat neib_cmp = gray_src({neib_cmp_row_range, neib_cmp_col_range});
        const double ssim = pssim_calc->compute(neib_cmp)[0];

        ssims_over_mdist[mdist_idx] = ssim;
        return ssim;
    };

    // Artifacts if the threshold is too small. Glitches if the threshold is too large.
    constexpr double ssim_threshold = 0.875;
    if (calc_ssim_with_mdist(prev_psize_idx + match_range.start) > ssim_threshold) {
        return prev_psize_idx;
    }

    std::vector<int> ref_psize_indices;
    ref_psize_indices.reserve(9);
    std::vector<int> prev_ref_psize_indices;
    prev_ref_psize_indices.reserve(9);

    if (index.x != 0) {
        const int left_psize_idx = psize_indices.at<int>(index.y, index.x - 1);
        ref_psize_indices.push_back(left_psize_idx);
        const int prev_left_psize_idx = prev_psize_indices.at<int>(index.y, index.x - 1);
        prev_ref_psize_indices.push_back(prev_left_psize_idx);
    }
    if (index.y != 0) {
        if (layout.isOutShift()) {
            const int upright_psize_idx = psize_indices.at<int>(index.y - 1, index.x);
            ref_psize_indices.push_back(upright_psize_idx);
            const int prev_upright_psize_idx = prev_psize_indices.at<int>(index.y - 1, index.x);
            prev_ref_psize_indices.push_back(prev_upright_psize_idx);
            if (index.x != 0) {
                const int upleft_psize_idx = psize_indices.at<int>(index.y - 1, index.x - 1);
                ref_psize_indices.push_back(upleft_psize_idx);
                const int prev_upleft_psize_idx = prev_psize_indices.at<int>(index.y - 1, index.x - 1);
                prev_ref_psize_indices.push_back(prev_upleft_psize_idx);
            }
        } else {
            const int upleft_psize_idx = psize_indices.at<int>(index.y - 1, index.x);
            ref_psize_indices.push_back(upleft_psize_idx);
            const int prev_upleft_psize_idx = prev_psize_indices.at<int>(index.y - 1, index.x);
            prev_ref_psize_indices.push_back(prev_upleft_psize_idx);
            if (index.x != (layout.getMICols(index.y - 1) - 1)) {
                const int upright_psize_idx = psize_indices.at<int>(index.y - 1, index.x + 1);
                ref_psize_indices.push_back(upright_psize_idx);
                const int prev_upright_psize_idx = prev_psize_indices.at<int>(index.y - 1, index.x + 1);
                prev_ref_psize_indices.push_back(prev_upright_psize_idx);
            }
        }
    }
    if (index.y != layout.getMIRows() - 1) {
        if (layout.isOutShift()) {
            const int prev_downright_psize_idx = prev_psize_indices.at<int>(index.y + 1, index.x);
            prev_ref_psize_indices.push_back(prev_downright_psize_idx);
            if (index.x != 0) {
                const int prev_downleft_psize_idx = prev_psize_indices.at<int>(index.y + 1, index.x - 1);
                prev_ref_psize_indices.push_back(prev_downleft_psize_idx);
            }
        } else {
            const int prev_downleft_psize_idx = prev_psize_indices.at<int>(index.y + 1, index.x);
            prev_ref_psize_indices.push_back(prev_downleft_psize_idx);
            if (index.x != (layout.getMICols(index.y + 1) - 1)) {
                const int prev_downright_psize_idx = prev_psize_indices.at<int>(index.y + 1, index.x + 1);
                prev_ref_psize_indices.push_back(prev_downright_psize_idx);
            }
        }
    }

    double max_ref_ssim = 0.0;
    int max_ref_psize_idx = 0;
    for (const int ref_psize_idx : ref_psize_indices) {
        const double ref_ssim = calc_ssim_with_mdist(ref_psize_idx + match_range.start);
        if (ref_ssim > max_ref_ssim) {
            max_ref_ssim = ref_ssim;
            max_ref_psize_idx = ref_psize_idx;
        }
    }
    if (max_ref_ssim > ssim_threshold) {
        return max_ref_psize_idx;
    }

    double max_prev_ref_ssim = 0.0;
    int max_prev_ref_psize_idx = 0;
    for (const int prev_ref_psize_idx : prev_ref_psize_indices) {
        const double prev_ref_ssim = calc_ssim_with_mdist(prev_ref_psize_idx + match_range.start);
        if (prev_ref_ssim > max_prev_ref_ssim) {
            max_prev_ref_ssim = prev_ref_ssim;
            max_prev_ref_psize_idx = prev_ref_psize_idx;
        }
    }
    if (max_prev_ref_ssim > ssim_threshold) {
        return max_prev_ref_psize_idx;
    }

    for (const int mdist : rgs::views::iota(match_range.start, match_range.end)) {
        calc_ssim_with_mdist(mdist);
    }
    const auto pmax_ssim = std::max_element(ssims_over_mdist.begin(), ssims_over_mdist.end());
    const int max_ssim_idx = (int)std::distance(ssims_over_mdist.begin(), pmax_ssim);
    return max_ssim_idx;
}

} // namespace _hp

cv::Mat estimatePatchsizes(const State& state)
{
    const auto& layout = state.layout_;

    // Indirectly controls the depth range.
    constexpr double match_start_factor = 0.214; // set this smaller to adapt images with large DoF.
    constexpr double match_end_factor = 0.414;   // set this larger to adapt images with small DoF.
    const int match_start = (int)(match_start_factor * layout.getDiameter());
    const int match_end = (int)(match_end_factor * layout.getDiameter());

    cv::Mat psize_indices = cv::Mat::zeros(layout.getMIRows(), layout.getMIMaxCols(), CV_32SC1);
    cv::Mat prev_psize_indices;
    if (!state.prev_patchsizes_.empty()) {
        prev_psize_indices = state.prev_patchsizes_ - match_start;
    } else {
        prev_psize_indices = psize_indices.clone();
    }

    for (const int row : rgs::views::iota(0, layout.getMIRows())) {
        for (const int col : rgs::views::iota(0, layout.getMICols(row) - 1)) {
            const cv::Point2d neib_center = layout.getMICenter(row, col + 1);
            if (neib_center.x == 0.0 or neib_center.y == 0.0)
                continue;

            const cv::Point index{col, row};
            const int patchsize_idx = _hp::estimatePatchsize(layout, state.gray_src_, psize_indices, prev_psize_indices,
                                                             index, {match_start, match_end});
            psize_indices.at<int>(row, col) = patchsize_idx;
        }
    }
    psize_indices.col(layout.getMIMinCols() - 2).copyTo(psize_indices.col(layout.getMIMinCols() - 1));

    cv::Mat patchsizes = psize_indices + match_start;
    return std::move(patchsizes);
}

} // namespace tlct::cvt::tspc