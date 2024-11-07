#pragma once

#include <cmath>
#include <numbers>
#include <ranges>

#include <opencv2/imgproc.hpp>

#include "tlct/config/tspc.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/convert/tspc/patchsize/params.hpp"

namespace tlct::_cvt::tspc {

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg::tspc;

static inline void renderView(const MvCache::TChannels& srcs, MvCache::TChannels& dsts, const tcfg::Layout& layout,
                              const std::vector<PsizeRecord>& patchsizes, const MvParams& params, MvCache& cache,
                              int view_row, int view_col)
{
    const int view_shift_x = (view_col - params.views / 2) * params.view_interval;
    const int view_shift_y = (view_row - params.views / 2) * params.view_interval;

    cv::Mat rotated_patch, resized_patch;
    cv::Mat weighted_patch;

    for (const int chan_id : rgs::views::iota(0, MvCache::CHANNELS)) {
        cache.render_canvas.setTo(std::numeric_limits<float>::epsilon());
        cache.weight_canvas.setTo(std::numeric_limits<float>::epsilon());

        int row_offset = 0;
        for (const int row : rgs::views::iota(0, layout.getMIRows())) {
            for (const int col : rgs::views::iota(0, layout.getMICols(row))) {
                const int offset = row_offset + col;

                // Extract patch
                const cv::Point2d center = layout.getMICenter(row, col);
                const double psize = params.psize_inflate * patchsizes[offset].psize;
                const cv::Point2d patch_center{center.x + view_shift_x, center.y + view_shift_y};
                const cv::Mat& patch = getRoiImageByCenter(srcs[chan_id], patch_center, psize);

                // Paste patch
                cv::rotate(patch, rotated_patch, cv::ROTATE_180);
                cv::resize(rotated_patch, resized_patch, {params.resized_patch_width, params.resized_patch_width}, 0, 0,
                           cv::INTER_LINEAR);

                cv::multiply(resized_patch, cache.grad_blending_weight, weighted_patch);

                // if the second bar is not out shift, then we need to shift the 1 col
                // else if the second bar is out shift, then we need to shift the 0 col
                const int right_shift = ((row % 2) ^ (int)layout.isOutShift()) * (params.patch_xshift / 2);
                const cv::Rect roi{col * params.patch_xshift + right_shift, row * params.patch_yshift,
                                   params.resized_patch_width, params.resized_patch_width};
                cache.render_canvas(roi) += weighted_patch;
                cache.weight_canvas(roi) += cache.grad_blending_weight;
            }
            row_offset += layout.getMIMaxCols();
        }

        cv::Mat cropped_rendered_image = cache.render_canvas(params.canvas_crop_roi);
        cv::Mat cropped_weight_matrix = cache.weight_canvas(params.canvas_crop_roi);

        cv::divide(cropped_rendered_image, cropped_weight_matrix, cache.normed_image);
        cache.normed_image.convertTo(cache.normed_image_u8, CV_8UC1);
        cv::resize(cache.normed_image_u8, cache.resized_normed_image_u8, {params.output_width, params.output_height},
                   0.0, 0.0, cv::INTER_AREA);

        if (layout.getRotation() > std::numbers::pi / 4.0) {
            cv::transpose(cache.resized_normed_image_u8, dsts[chan_id]);
        } else {
            cache.resized_normed_image_u8.copyTo(dsts[chan_id]);
        }
    }
}

} // namespace tlct::_cvt::tspc
