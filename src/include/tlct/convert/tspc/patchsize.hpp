#pragma once

#include <algorithm>
#include <ranges>
#include <vector>

#include <opencv2/imgproc.hpp>
#include <opencv2/quality.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/tspc.hpp"
#include "tlct/helper/static_math.hpp"

namespace tlct::cvt::inline tspc {

namespace rgs = std::ranges;

namespace _hp {

static inline int estimatePatchsize(const cfg::tspc::Layout& layout, const cv::Mat& gray_src,
                                    const cv::Mat& psize_indices, const cv::Point index, const cv::Range match_range)
{
    const cv::Point2d curr_center = layout.getMICenter(index);
    const cv::Point2d neib_center = layout.getMICenter(index.y, index.x + 1);

    const int row_shift = 12 * layout.getUpsample();
    const int col_start_shift = -14 * layout.getUpsample();
    const int col_end_shift = -4 * layout.getUpsample();

    const cv::Range curr_cmp_row_range{tlct::_hp::iround(curr_center.y - row_shift),
                                       tlct::_hp::iround(curr_center.y + row_shift)};
    const cv::Range curr_cmp_col_range{tlct::_hp::iround(curr_center.x + col_start_shift),
                                       tlct::_hp::iround(curr_center.x + col_end_shift)};
    if (curr_cmp_row_range.end > gray_src.rows || curr_cmp_col_range.end > gray_src.cols) {
        return 0;
    }
    const cv::Mat curr_cmp = gray_src({curr_cmp_row_range, curr_cmp_col_range});
    const auto pssim_calc = cv::quality::QualitySSIM::create(curr_cmp);

    std::vector<double> ssims_over_mdist;
    ssims_over_mdist.reserve(match_range.size());
    for (const int mdist : rgs::views::iota(match_range.start, match_range.end)) {
        const cv::Range neib_cmp_row_range{tlct::_hp::iround(neib_center.y - row_shift),
                                           tlct::_hp::iround(neib_center.y + row_shift)};
        const cv::Range neib_cmp_col_range{tlct::_hp::iround(neib_center.x + col_start_shift) + mdist,
                                           tlct::_hp::iround(neib_center.x + col_end_shift) + mdist};
        if (neib_cmp_row_range.end > gray_src.rows || neib_cmp_col_range.end > gray_src.cols) {
            break;
        }

        const cv::Mat neib_cmp = gray_src({neib_cmp_row_range, neib_cmp_col_range});
        const double ssim = pssim_calc->compute(neib_cmp)[0];
        ssims_over_mdist.push_back(ssim);
    }

    const auto pmax_ssim = std::max_element(ssims_over_mdist.begin(), ssims_over_mdist.end());
    const int max_ssim_idx = (int)std::distance(ssims_over_mdist.begin(), pmax_ssim);

    std::vector<int> ref_psize_indices;
    if (index.x != 0) {
        const int left_psize_idx = psize_indices.at<int>(index.y, index.x - 1);
        ref_psize_indices.push_back(left_psize_idx);
    }
    if (index.y != 0) {
        if (layout.isOutShift()) {
            const int upright_psize_idx = psize_indices.at<int>(index.y - 1, index.x);
            ref_psize_indices.push_back(upright_psize_idx);
            if (index.x != 0) {
                const int upleft_psize_idx = psize_indices.at<int>(index.y - 1, index.x - 1);
                ref_psize_indices.push_back(upleft_psize_idx);
            }
        } else {
            const int upleft_psize_idx = psize_indices.at<int>(index.y - 1, index.x);
            ref_psize_indices.push_back(upleft_psize_idx);
            if (index.x != (layout.getMICols() - 1)) {
                const int upright_psize_idx = psize_indices.at<int>(index.y - 1, index.x + 1);
                ref_psize_indices.push_back(upright_psize_idx);
            }
        }
    }

    if (ref_psize_indices.empty()) {
        return max_ssim_idx;
    }

    double max_ref_ssim = 0.0;
    int max_ref_psize_idx = 0;
    for (const int ref_psize_idx : ref_psize_indices) {
        const double ref_ssim = ssims_over_mdist[ref_psize_idx];
        if (ref_ssim > max_ref_ssim) {
            max_ref_ssim = ref_ssim;
            max_ref_psize_idx = ref_psize_idx;
        }
    }

    constexpr double ssim_threshold = 0.875;
    if (max_ref_ssim > ssim_threshold) {
        return max_ref_psize_idx;
    } else {
        return max_ssim_idx;
    }
}

} // namespace _hp

TLCT_API inline void estimatePatchsizes_(const cfg::tspc::Layout& layout, const cv::Mat& src, cv::Mat& patchsizes)
{
    cv::Mat psize_indices = cv::Mat::zeros(layout.getMIRows(), layout.getMICols(), CV_32SC1);

    cv::Mat gray_src;
    cv::cvtColor(src, gray_src, cv::COLOR_BGR2GRAY);

    const int match_start = 15 * layout.getUpsample();
    const int match_end = 29 * layout.getUpsample();

    for (const int row : rgs::views::iota(0, layout.getMIRows())) {
        for (const int col : rgs::views::iota(0, layout.getMICols() - 1)) {
            const cv::Point2d neib_center = layout.getMICenter(row, col + 1);
            if (neib_center.x == 0.0 or neib_center.y == 0.0)
                continue;

            const cv::Point index{col, row};
            const int patchsize_idx =
                _hp::estimatePatchsize(layout, gray_src, psize_indices, index, {match_start, match_end});
            psize_indices.at<int>(row, col) = patchsize_idx;
        }
    }
    psize_indices.col(layout.getMICols() - 2).copyTo(psize_indices.col(layout.getMICols() - 1));
    patchsizes = psize_indices + match_start;
}

TLCT_API inline cv::Mat estimatePatchsizes(const cfg::tspc::Layout& layout, const cv::Mat& src)
{
    cv::Mat patchsizes;
    estimatePatchsizes_(layout, src, patchsizes);
    return patchsizes;
}

} // namespace tlct::cvt::inline tspc