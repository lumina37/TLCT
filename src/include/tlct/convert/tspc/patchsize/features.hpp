#pragma once

#include <ranges>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/tspc/layout.hpp"
#include "tlct/convert/common/pixel_heap.hpp"
#include "tlct/convert/helper.hpp"

namespace tlct::cvt::tspc::_hp {

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg::tspc;
using namespace tlct::cvt::_hp;

using PixHeaps = std::vector<std::vector<PixHeap>>;

TLCT_API inline PixHeap calcFeaturesInOneMI(const cv::Mat& src, const int border_width,
                                            const float accept_threshold = 64.0) noexcept
{
    PixHeap heap{};
    for (const int irow : rgs::views::iota(border_width, src.rows - border_width)) {
        const auto prow = src.ptr<float>(irow);
        for (const int icol : rgs::views::iota(border_width, src.cols - border_width)) {
            const float response = prow[icol];
            if (response > accept_threshold) {
                heap.push({{icol, irow}, response});
            }
        }
    }

    return heap;
}

static inline PixHeaps calcFeatures(const tcfg::Layout& layout, const cv::Mat& gray_src)
{
    cv::Mat blurred_src;
    cv::medianBlur(gray_src, blurred_src, 3);

    cv::Mat xedges, yedges, edges;
    cv::Sobel(blurred_src, xedges, CV_32F, 1, 0, cv::FILTER_SCHARR);
    cv::Sobel(blurred_src, yedges, CV_32F, 0, 1, cv::FILTER_SCHARR);
    edges = (xedges + yedges) / 2.0;

    const PixHeap default_heap{};
    const int feature_bbox_size = (int)(layout.getDiameter() / std::numbers::sqrt2);
    PixHeaps features;
    features.reserve(layout.getMIRows());
    for (const int irow : rgs::views::iota(0, layout.getMIRows())) {

        std::vector<PixHeap> row_features(layout.getMIMaxCols(), default_heap);
        for (const int icol : rgs::views::iota(0, layout.getMICols(irow))) {
            const cv::Point2d center = layout.getMICenter(irow, icol);
            const cv::Mat& roi = getRoiImageByCenter(edges, (cv::Point)center, feature_bbox_size);
            PixHeap feature = calcFeaturesInOneMI(roi, (int)(0.25 * layout.getRadius()));
            row_features[icol] = feature;
        }
        features.push_back(std::move(row_features));
    }

    return std::move(features);
}

} // namespace tlct::cvt::tspc::_hp
