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
    const auto& anchor_mi = mis_.getMI(neighbors.getSelfIdx());
    WrapSSIM wrap_anchor{anchor_mi};

    double weighted_metric = 0.0;
    double total_weight = 0.0;

    for (const auto direction : DIRECTIONS) {
        if (neighbors.hasNeighbor(direction)) {
            const cv::Point2d anchor_shift = -neighbors.getUnitShift(direction) * pattern_shift_;
            const cv::Point2d match_step = neighbors.getUnitShift(direction);
            const cv::Point2d cmp_shift = anchor_shift + match_step * psize;

            const cv::Rect anchor_roi = getRoiByCenter(mi_center + anchor_shift, pattern_size_);
            wrap_anchor.updateRoi(anchor_roi);
            const cv::Point neib_idx = neighbors.getNeighborIdx(direction);
            const auto& neib_mi = mis_.getMI(neib_idx);
            const cv::Rect neib_roi = getRoiByCenter(mi_center + cmp_shift, pattern_size_);
            WrapSSIM wrap_neib{neib_mi};
            wrap_neib.updateRoi(neib_roi);

            const double metric = wrap_anchor.compare(wrap_neib);
            const double weight = computeGrad(wrap_anchor.I_);
            weighted_metric += metric * weight;
            total_weight += weight;
        }
    };

    const double final_metric = weighted_metric / total_weight;
    return final_metric;
}

int State::_estimatePatchsizeOverFullMatch(const Neighbors& neighbors)
{
    const cv::Point2d mi_center{layout_.getRadius(), layout_.getRadius()};
    const auto& anchor_mi = mis_.getMI(neighbors.getSelfIdx());
    WrapSSIM wrap_anchor{anchor_mi};

    if (Inspector::PATTERN_ENABLED) {
        inspector_.saveMI(anchor_mi.I_, neighbors.getSelfIdx());
    }

    const int max_shift = (int)(pattern_shift_ * 2);

    double weighted_psize = 0.0;
    double total_weight = 0.0;
    std::vector<double> psizes, weights;

    for (const auto direction : DIRECTIONS) {
        if (neighbors.hasNeighbor(direction)) {
            const cv::Point2d anchor_shift = neighbors.getUnitShift(direction) * pattern_shift_;
            const cv::Rect anchor_roi = getRoiByCenter(mi_center + anchor_shift, pattern_size_);
            wrap_anchor.updateRoi(anchor_roi);

            if (Inspector::PATTERN_ENABLED) {
                inspector_.saveAnchor(wrap_anchor.I_, neighbors.getSelfIdx(), (int)direction);
            }

            const auto& neib_mi = mis_.getMI(neighbors.getNeighborIdx(direction));
            WrapSSIM wrap_neib{neib_mi};

            const cv::Point2d match_step = -neighbors.getUnitShift(direction);
            cv::Point2d cmp_shift = anchor_shift + match_step * min_psize_;

            int min_metric_psize = INVALID_PSIZE;
            double min_metric = std::numeric_limits<double>::max();
            std::vector<double> metrics;
            metrics.reserve(max_shift - min_psize_);
            for (const int psize : rgs::views::iota(min_psize_, max_shift)) {
                cmp_shift += match_step;

                const cv::Rect cmp_roi = getRoiByCenter(mi_center + cmp_shift, pattern_size_);
                wrap_neib.updateRoi(cmp_roi);

                const double metric = wrap_anchor.compare(wrap_neib);
                metrics.push_back(metric);

                if (Inspector::PATTERN_ENABLED) {
                    inspector_.saveCmpPattern(wrap_neib.I_, neighbors.getSelfIdx(), (int)direction, psize, metric);
                }

                if (metric < min_metric) {
                    min_metric = metric;
                    min_metric_psize = psize;
                }
            }

            const double weight = computeGrad(wrap_anchor.I_) * stdvar(metrics);
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

cv::Mat State::estimatePatchsizes()
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
