#pragma once

#include <ranges>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/tspc/layout.hpp"
#include "tlct/convert/helper.hpp"
#include "topk.hpp"

namespace tlct::cvt::tspc {

using PixHeaps = std::vector<std::vector<PixHeap>>;

namespace _hp {

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg::tspc;

static inline PixHeaps calcFeatures(const tcfg::Layout& layout, const cv::Mat& gray_src)
{
    cv::Mat blurred_src;
    cv::medianBlur(gray_src, blurred_src, 3);

    cv::Mat xedges, yedges, edges;
    cv::Sobel(blurred_src, xedges, CV_32F, 1, 0, cv::FILTER_SCHARR);
    cv::Sobel(blurred_src, yedges, CV_32F, 0, 1, cv::FILTER_SCHARR);
    edges = (xedges + yedges) / 2.0;

    const PixHeap default_heap{};
    const int roi_size = (int)(layout.getDiameter() / std::numbers::sqrt2);
    PixHeaps features;
    features.reserve(layout.getMIRows());
    for (const int irow : rgs::views::iota(0, layout.getMIRows())) {

        std::vector<PixHeap> row_features(layout.getMIMaxCols(), default_heap);
        for (const int icol : rgs::views::iota(0, layout.getMICols(irow))) {
            const auto& center = layout.getMICenter(irow, icol);
            const cv::Mat& roi = getRoiImageByCenter(edges, (cv::Point)center, roi_size);
            PixHeap feature = statTopKFeaturePoints(roi);
            row_features[icol] = feature;
        }
        features.push_back(std::move(row_features));
    }

    return std::move(features);
}

} // namespace _hp

} // namespace tlct::cvt::tspc
