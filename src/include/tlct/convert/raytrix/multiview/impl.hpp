#pragma once

#include <array>
#include <cmath>
#include <numbers>
#include <numeric>
#include <ranges>

#include <opencv2/imgproc.hpp>

#include "tlct/config/raytrix.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/convert/raytrix/patchsize/neighbors.hpp"
#include "tlct/convert/raytrix/patchsize/params.hpp"

namespace tlct::_cvt::raytrix {

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg::raytrix;

static inline void computeWeights(const MIs_<tcfg::Layout>& mis, const tcfg::Layout& layout, MvCache& cache)
{
    cache.texture_I.create(layout.getMIRows(), layout.getMIMaxCols(), CV_32F);
    cache.rank.create(layout.getMIRows(), layout.getMIMaxCols(), CV_8U);
    cache.weights.create(layout.getMIRows(), layout.getMIMaxCols(), CV_32F);

    const cv::Point mi_center{_hp::iround(layout.getRadius()), _hp::iround(layout.getRadius())};
    const int mi_width = _hp::iround(layout.getRadius() / std::numbers::sqrt2);

    // 1-pass: obtain texture intensity
    int row_offset = 0;
    for (const int row : rgs::views::iota(0, layout.getMIRows())) {
        for (const int col : rgs::views::iota(0, layout.getMICols(row))) {
            const int offset = row_offset + col;

            const auto& mi = getRoiImageByCenter(mis.getMI(offset).I, mi_center, mi_width);
            const auto ti = (float)textureIntensity(mi);

            cache.texture_I.at<float>(row, col) = ti;
        }
        row_offset += layout.getMIMaxCols();
    }

    // 2-pass: draft weight and rank
    for (const int row : rgs::views::iota(0, layout.getMIRows())) {
        for (const int col : rgs::views::iota(0, layout.getMICols(row))) {
            const auto neighbors = NearNeighbors::fromLayoutAndIndex(layout, {col, row});

            float avg_I = 0.0;
            int neib_count = 0;
            const float curr_I = cache.texture_I.at<float>(neighbors.getSelfIdx());
            for (const auto direction : NearNeighbors::DIRECTIONS) {
                if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
                    continue;
                }

                const auto neib_I = cache.texture_I.at<float>(neighbors.getNeighborIdx(direction));

                neib_count++;
                avg_I += (neib_I - avg_I) / (float)neib_count;
            }

            int rank = NearNeighbors::DIRECTION_NUM;
            for (const auto direction : NearNeighbors::DIRECTIONS) {
                if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
                    continue;
                }
                const auto neib_I = cache.texture_I.at<float>(neighbors.getNeighborIdx(direction));
                if (curr_I > neib_I) {
                    rank--;
                }
            }

            constexpr auto HALF_DIRECTION_NUM = (float)(NearNeighbors::DIRECTION_NUM >> 1);
            float expo = _hp::clip(curr_I - avg_I, HALF_DIRECTION_NUM, HALF_DIRECTION_NUM);

            cache.weights.at<float>(row, col) = std::expf(expo);
            cache.rank.at<uint8_t>(row, col) = rank;
        }
    }

    // 3-pass: adjust weight with neighbor ranks
    for (const int row : rgs::views::iota(0, layout.getMIRows())) {
        for (const int col : rgs::views::iota(0, layout.getMICols(row))) {
            const auto neighbors = NearNeighbors::fromLayoutAndIndex(layout, {col, row});

            int hiwgt_neib_count = 0;
            for (const auto direction : {
                     NearNeighbors::Direction::UPLEFT,
                     NearNeighbors::Direction::RIGHT,
                     NearNeighbors::Direction::DOWNLEFT,
                 }) {
                if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
                    continue;
                }
                const auto neib_rank = cache.rank.at<uint8_t>(neighbors.getNeighborIdx(direction));
                if (neib_rank == 0) {
                    hiwgt_neib_count++;
                }
            }
            if (hiwgt_neib_count == (NearNeighbors::DIRECTION_NUM / 2)) {
                cache.weights.at<float>(neighbors.getSelfIdx()) = std::numeric_limits<float>::epsilon();
                continue;
            }

            hiwgt_neib_count = 0;
            for (const auto direction : {
                     NearNeighbors::Direction::UPRIGHT,
                     NearNeighbors::Direction::LEFT,
                     NearNeighbors::Direction::DOWNRIGHT,
                 }) {
                if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
                    continue;
                }
                const auto neib_rank = cache.rank.at<uint8_t>(neighbors.getNeighborIdx(direction));
                if (neib_rank == 0) {
                    hiwgt_neib_count++;
                }
            }
            if (hiwgt_neib_count == (NearNeighbors::DIRECTION_NUM / 2)) {
                cache.weights.at<float>(neighbors.getSelfIdx()) = std::numeric_limits<float>::epsilon();
            }
        }
    }
}

static inline void renderView(const cv::Mat& src, cv::Mat& dst, const tcfg::Layout& layout,
                              const std::vector<PsizeRecord>& patchsizes, const MvParams& params, MvCache& cache,
                              int view_row, int view_col)
{
    const int view_shift_x = (view_col - params.views / 2) * params.view_interval;
    const int view_shift_y = (view_row - params.views / 2) * params.view_interval;

    cache.render_canvas.setTo(std::numeric_limits<float>::epsilon());
    cache.weight_canvas.setTo(std::numeric_limits<float>::epsilon());

    cv::Mat resized_patch;
    cv::Mat resized_patch_channels[MvCache::CHANNELS];
    cv::Mat weighted_patch;

    int row_offset = 0;
    for (const int row : rgs::views::iota(0, layout.getMIRows())) {
        for (const int col : rgs::views::iota(0, layout.getMICols(row))) {
            const int offset = row_offset + col;
            const float weight = cache.weights.at<float>(row, col);

            // Extract patch
            const cv::Point2d center = layout.getMICenter(row, col);
            const double psize = params.psize_inflate * patchsizes[offset].psize;
            const cv::Point2d patch_center{center.x + view_shift_x, center.y + view_shift_y};
            const cv::Mat& patch = getRoiImageByCenter(src, patch_center, psize);

            // Paste patch
            cv::resize(patch, resized_patch, {params.resized_patch_width, params.resized_patch_width}, 0, 0,
                       cv::INTER_LINEAR);

            cv::split(resized_patch, resized_patch_channels);
            for (cv::Mat& resized_patch_channel : resized_patch_channels) {
                cv::multiply(resized_patch_channel, cache.grad_blending_weight, resized_patch_channel);
            }

            cv::merge(resized_patch_channels, MvCache::CHANNELS, weighted_patch);

            // if the second bar is not out shift, then we need to shift the 1 col
            // else if the second bar is out shift, then we need to shift the 0 col
            const int right_shift = ((row % 2) ^ (int)layout.isOutShift()) * (params.patch_xshift / 2);
            const cv::Rect roi{col * params.patch_xshift + right_shift, row * params.patch_yshift,
                               params.resized_patch_width, params.resized_patch_width};
            cache.render_canvas(roi) += weighted_patch * weight;
            cache.weight_canvas(roi) += cache.grad_blending_weight * weight;
        }
        row_offset += layout.getMIMaxCols();
    }

    cv::Mat cropped_rendered_image = cache.render_canvas(params.canvas_crop_roi);
    cv::Mat cropped_weight_matrix = cache.weight_canvas(params.canvas_crop_roi);

    cv::split(cropped_rendered_image, cache.cropped_rendered_image_channels);
    for (cv::Mat& cropped_new_image_channel : cache.cropped_rendered_image_channels) {
        cropped_new_image_channel /= cropped_weight_matrix;
    }
    cv::Mat& normed_image = cropped_rendered_image;
    cv::merge(cache.cropped_rendered_image_channels, MvCache::CHANNELS, normed_image);

    normed_image.convertTo(cache.normed_image_u8, CV_8UC3);
    cv::resize(cache.normed_image_u8, cache.resized_normed_image_u8, {params.output_width, params.output_height}, 0.0,
               0.0, cv::INTER_AREA);

    if (layout.getRotation() > std::numbers::pi / 4.0) {
        cv::transpose(cache.resized_normed_image_u8, dst);
    } else {
        dst = std::move(cache.resized_normed_image_u8);
    }
}

} // namespace tlct::_cvt::raytrix
