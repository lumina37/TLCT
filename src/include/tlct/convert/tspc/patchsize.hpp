#pragma once

#include <algorithm>
#include <cmath>
#include <numeric>
#include <ranges>
#include <set>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/quality.hpp>

#include "direction.hpp"
#include "neighbors.hpp"
#include "tlct/common/defines.h"
#include "tlct/config/tspc/layout.hpp"
#include "tlct/convert/helper/roi.hpp"
#include "tlct/convert/helper/variance.hpp"
#include "tlct/convert/helper/wrapper.hpp"
#include "tlct/convert/tspc/state.hpp"

namespace tlct::cvt::tspc {

namespace rgs = std::ranges;

using namespace tlct::cvt::_hp;
namespace tcfg = tlct::cfg::tspc;

constexpr int INVALID_PSIZE = -1;

template <bool left_and_up_only>
static inline std::vector<int> getRefPsizes(const cv::Mat& psizes, const _hp::Neighbors& neighbors)
{
    std::set<int> ref_psizes_set;

    for (const int idx : rgs::views::iota(0, DIRECTION_NUM / 2)) {
        const auto direction = DIRECTIONS[idx];
        if (neighbors.hasNeighbor(direction)) {
            const int psize = psizes.at<int>(neighbors.getNeighborIdx(direction));
            ref_psizes_set.insert(psize);
        }
    }

    if constexpr (!left_and_up_only) {
        for (const int idx : rgs::views::iota(DIRECTION_NUM / 2, DIRECTION_NUM)) {
            const auto direction = DIRECTIONS[idx];
            if (neighbors.hasNeighbor(direction)) {
                const int psize = psizes.at<int>(neighbors.getNeighborIdx(direction));
                ref_psizes_set.insert(psize);
            }
        }
    }

    std::vector<int> ref_psizes(ref_psizes_set.begin(), ref_psizes_set.end());
    return std::move(ref_psizes);
}

double State::_calcMetricWithPsize(const _hp::Neighbors& neighbors, const int psize) const
{
    const cv::Point2d curr_center = neighbors.getSelfPt();

    double weighted_metric = 0.0;
    double total_weight = 0.0;

    for (const auto direction : DIRECTIONS) {
        if (neighbors.hasNeighbor(direction)) {
            const cv::Point2d anchor_shift = -neighbors.getUnitShift(direction) * pattern_shift_;
            const cv::Point2d match_step = neighbors.getUnitShift(direction);
            const cv::Point2d cmp_shift = anchor_shift + match_step * psize;

            const cv::Point2d anchor_center = curr_center + anchor_shift;
            const cv::Point2d neib_center = neighbors.getNeighborPt(direction);
            const cv::Point2d cmp_center = neib_center + cmp_shift;

            const auto& anchor = AnchorWrapper::fromRoi(getRoiImageByCenter(gray_src_, anchor_center, pattern_size_));
            const auto& rhs = getRoiImageByCenter(gray_src_, cmp_center, pattern_size_);

            const double metric = anchor.compare(rhs);
            weighted_metric += metric * anchor.getWeight();
            total_weight += anchor.getWeight();
        }
    };

    const double final_metric = weighted_metric / total_weight;
    return final_metric;
}

int State::_estimatePatchsizeOverFullMatch(const _hp::Neighbors& neighbors)
{
    const cv::Point2d curr_center = neighbors.getSelfPt();

    if (Inspector::PATTERN_ENABLED) {
        inspector_.saveMI(getRoiImageByCenter(gray_src_, curr_center, layout_.getDiameter()), neighbors.getSelfIdx());
    }

    const int max_shift = (int)(pattern_shift_ * 2);

    double weighted_psize = 0.0;
    double total_weight = 0.0;
    std::vector<double> psizes, weights;

    for (const auto direction : DIRECTIONS) {
        if (neighbors.hasNeighbor(direction)) {
            const cv::Point2d anchor_shift = -neighbors.getUnitShift(direction) * pattern_shift_;
            const cv::Point2d anchor_center = curr_center + anchor_shift;
            const cv::Point2d match_step = neighbors.getUnitShift(direction);
            const auto& anchor = AnchorWrapper::fromRoi(getRoiImageByCenter(gray_src_, anchor_center, pattern_size_));

            if (Inspector::PATTERN_ENABLED) {
                inspector_.saveAnchor(getRoiImageByCenter(gray_src_, anchor_center, pattern_size_),
                                      neighbors.getSelfIdx(), (int)direction);
            }

            const cv::Point2d neib_center = neighbors.getNeighborPt(direction);
            cv::Point2d cmp_shift = anchor_shift + match_step * min_psize_;

            int min_metric_psize = INVALID_PSIZE;
            double min_metric = std::numeric_limits<double>::max();
            std::vector<double> metrics;
            metrics.reserve(max_shift - min_psize_);
            for (const int psize : rgs::views::iota(min_psize_, max_shift)) {
                cmp_shift += match_step;
                const auto& rhs = getRoiImageByCenter(gray_src_, neib_center + cmp_shift, pattern_size_);
                const double metric = anchor.compare(rhs);
                metrics.push_back(metric);

                if (Inspector::PATTERN_ENABLED) {
                    inspector_.saveCmpPattern(getRoiImageByCenter(gray_src_, neib_center + cmp_shift, pattern_size_),
                                              neighbors.getSelfIdx(), (int)direction, psize, metric);
                }

                if (metric < min_metric) {
                    min_metric = metric;
                    min_metric_psize = psize;
                }
            }

            const double weight = anchor.getWeight() * var_d(metrics);
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
    const auto neighbors = _hp::Neighbors::fromLayoutAndIndex(layout_, index);

    const int prev_psize = prev_patchsizes_.at<int>(index);
    const double prev_metric = _calcMetricWithPsize(neighbors, prev_psize);
    if (prev_metric < spec_cfg_.getPsizeShortcutThreshold()) {
        return prev_psize;
    }

    const std::vector<int>& ref_psizes = getRefPsizes<true>(psizes, neighbors);
    double min_ref_metric = std::numeric_limits<double>::max();
    int min_ref_psize = 0;
    for (const int ref_psize : ref_psizes) {
        const double ref_metric = _calcMetricWithPsize(neighbors, prev_psize);
        if (ref_metric < min_ref_metric) {
            min_ref_metric = ref_metric;
            min_ref_psize = ref_psize;
        }
    }
    if (min_ref_metric < spec_cfg_.getPsizeShortcutThreshold()) {
        return min_ref_psize;
    }

    const std::vector<int>& prev_ref_psizes = getRefPsizes<false>(prev_patchsizes_, neighbors);
    double min_prev_ref_metric = std::numeric_limits<double>::max();
    int min_prev_ref_psize = 0;
    for (const int prev_ref_psize : prev_ref_psizes) {
        const double prev_ref_metric = _calcMetricWithPsize(neighbors, prev_psize);
        if (prev_ref_metric < min_prev_ref_metric) {
            min_prev_ref_metric = prev_ref_metric;
            min_prev_ref_psize = prev_ref_psize;
        }
    }
    if (min_prev_ref_metric < spec_cfg_.getPsizeShortcutThreshold()) {
        return min_prev_ref_psize;
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

} // namespace tlct::cvt::tspc