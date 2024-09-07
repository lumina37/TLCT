#pragma once

#include <algorithm>
#include <cmath>
#include <numeric>
#include <ranges>
#include <set>
#include <vector>

#include <opencv2/core.hpp>

#include "neighbors.hpp"
#include "state.hpp"
#include "tlct/common/defines.h"
#include "tlct/config/raytrix.hpp"
#include "tlct/convert/helper.hpp"

namespace tlct::_cvt::raytrix {

namespace rgs = std::ranges;

constexpr int INVALID_PSIZE = -1;

double State::_calcMetricWithPsize(const Neighbors& neighbors, const int psize) const
{
    const cv::Point2d mi_center{layout_.getRadius(), layout_.getRadius()};
    const cv::Mat& anchor_mi = mis_.getMI(neighbors.getSelfIdx());

    double weighted_metric = 0.0;
    double total_weight = 0.0;

    for (const auto direction : DIRECTIONS) {
        if (neighbors.hasNeighbor(direction)) {
            const cv::Point2d anchor_shift = -neighbors.getUnitShift(direction) * pattern_shift_;
            const cv::Point2d match_step = neighbors.getUnitShift(direction);
            const cv::Point2d cmp_shift = anchor_shift + match_step * psize;

            const cv::Mat& anchor = getRoiImageByCenter(anchor_mi, mi_center + anchor_shift, pattern_size_);
            const auto& anchor_wrapper = AnchorWrapper::fromRoi(anchor);
            const cv::Point neib_idx = neighbors.getNeighborIdx(direction);
            const cv::Mat& neib_mi = mis_.getMI(neib_idx);
            const cv::Mat& neib = getRoiImageByCenter(neib_mi, mi_center + cmp_shift, pattern_size_);

            const double metric = anchor_wrapper.compare(neib);
            weighted_metric += metric * anchor_wrapper.getWeight();
            total_weight += anchor_wrapper.getWeight();
        }
    };

    const double final_metric = weighted_metric / total_weight;
    return final_metric;
}

int State::_estimatePatchsizeOverFullMatch(const Neighbors& neighbors)
{
    const cv::Point2d mi_center{layout_.getRadius(), layout_.getRadius()};
    const cv::Mat& anchor_mi = mis_.getMI(neighbors.getSelfIdx());

    if (Inspector::PATTERN_ENABLED) {
        inspector_.saveMI(getRoiImageByCenter(gray_src_, neighbors.getSelfPt(), layout_.getDiameter()),
                          neighbors.getSelfIdx());
    }

    const int max_shift = (int)(pattern_shift_ * 2);

    double weighted_psize = 0.0;
    double total_weight = 0.0;
    std::vector<double> psizes, weights;

    for (const auto direction : DIRECTIONS) {
        if (neighbors.hasNeighbor(direction)) {
            const cv::Point2d anchor_shift = neighbors.getUnitShift(direction) * pattern_shift_;

            const cv::Mat& anchor = getRoiImageByCenter(anchor_mi, mi_center + anchor_shift, pattern_size_);
            const auto& anchor_wrapper = AnchorWrapper::fromRoi(anchor);

            if (Inspector::PATTERN_ENABLED) {
                inspector_.saveAnchor(anchor, neighbors.getSelfIdx(), (int)direction);
            }

            const cv::Mat& neib_mi = mis_.getMI(neighbors.getNeighborIdx(direction));

            const cv::Point2d match_step = -neighbors.getUnitShift(direction);
            cv::Point2d cmp_shift = anchor_shift + match_step * min_psize_;

            int min_metric_psize = INVALID_PSIZE;
            double min_metric = std::numeric_limits<double>::max();
            std::vector<double> metrics;
            metrics.reserve(max_shift - min_psize_);
            for (const int psize : rgs::views::iota(min_psize_, max_shift)) {
                cmp_shift += match_step;
                const cv::Mat& cmp = getRoiImageByCenter(neib_mi, mi_center + cmp_shift, pattern_size_);
                const double metric = anchor_wrapper.compare(cmp);
                metrics.push_back(metric);

                if (Inspector::PATTERN_ENABLED) {
                    inspector_.saveCmpPattern(cmp, neighbors.getSelfIdx(), (int)direction, psize, metric);
                }

                if (metric < min_metric) {
                    min_metric = metric;
                    min_metric_psize = psize;
                }
            }

            const double weight = anchor_wrapper.getWeight() * stdvar(metrics);
            weighted_psize += weight * min_metric_psize;
            total_weight += weight;
            psizes.push_back(min_metric_psize);
            weights.push_back(weight);
        }
    };

    if (Inspector::METRIC_REPORT_ENABLED) {
        inspector_.appendMetricReport(neighbors.getSelfIdx(), psizes, weights);
    }

    if (total_weight == 0.0)
        return INVALID_PSIZE;

    const int final_psize = (int)std::round(weighted_psize / total_weight);
    return final_psize;
}

int State::_estimatePatchsize(cv::Mat& psizes, const cv::Point index)
{
    const auto neighbors = Neighbors::fromLayoutAndIndex(layout_, index);

    const int prev_psize = prev_patchsizes_.at<int>(index);
    const double prev_metric = _calcMetricWithPsize(neighbors, prev_psize);
    if (prev_metric < spec_cfg_.getPsizeShortcutThreshold()) {
        return prev_psize;
    }

    const int psize = _estimatePatchsizeOverFullMatch(neighbors);

    if (psize == INVALID_PSIZE) {
        return prev_psize;
    } else {
        return psize;
    }
}

inline cv::Mat State::estimatePatchsizes()
{
    for (const int row : rgs::views::iota(0, layout_.getMIRows())) {
        for (const int col : rgs::views::iota(0, layout_.getMICols(row))) {
            const cv::Point index{col, row};
            const int psize = _estimatePatchsize(patchsizes_, index);
            patchsizes_.at<int>(index) = psize;
        }
    }

    return std::move(patchsizes_);
}

} // namespace tlct::_cvt::raytrix
