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

int State::_estimatePatchsizeOverFullMatch(const Neighbors& neighbors, int offset)
{
    const cv::Point2d mi_center{layout_.getRadius(), layout_.getRadius()};
    const auto& anchor_mi = mis_.getMI(offset);
    WrapSSIM wrap_anchor{anchor_mi};

#ifdef TLCT_ENABLE_INSPECT
    inspector_.saveMI(anchor_mi.I_, neighbors.getSelfIdx());
#endif

    const int max_shift = (int)(pattern_shift_ * 2);

    double weighted_psize = 0.0;
    double total_weight = 0.0;

    for (const auto direction : DIRECTIONS) {
        if (neighbors.hasNeighbor(direction)) [[likely]] {
            const cv::Point2d anchor_shift = neighbors.getUnitShift(direction) * pattern_shift_;
            const cv::Rect anchor_roi = getRoiByCenter(mi_center + anchor_shift, pattern_size_);
            wrap_anchor.updateRoi(anchor_roi);

#ifdef TLCT_ENABLE_INSPECT
            inspector_.saveAnchor(wrap_anchor.I_, neighbors.getSelfIdx(), (int)direction);
#endif

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

#ifdef TLCT_ENABLE_INSPECT
                inspector_.saveCmpPattern(wrap_neib.I_, neighbors.getSelfIdx(), (int)direction, psize, metric);
#endif

                if (metric < min_metric) {
                    min_metric = metric;
                    min_metric_psize = psize;
                }
            }

            const double weight = grad(wrap_anchor.I_) * stdvar(metrics) + std::numeric_limits<float>::epsilon();
            weighted_psize += weight * min_metric_psize;
            total_weight += weight;
        }
    }

    const int final_psize = (int)std::round(weighted_psize / total_weight);
    return final_psize;
}

PsizeCache State::_estimatePatchsize(const Neighbors& neighbors, int offset)
{
    const auto& anchor_mi = mis_.getMI(offset);
    const uint64_t hash = dhash(anchor_mi.I_);
    const auto& prev_psize = prev_patchsizes_[offset];

    if (prev_psize.psize != INVALID_PSIZE) [[likely]] {
        const int hash_dist = L1_dist(prev_psize.hash, hash);
        if (hash_dist < spec_cfg_.getPsizeShortcutThreshold()) {
            return {prev_psize.psize, hash};
        }
    }

    const int psize = _estimatePatchsizeOverFullMatch(neighbors, offset);
    return {psize, hash};
}

void State::estimatePatchsizes()
{
    int row_offset = 0;
    for (const int row : rgs::views::iota(0, layout_.getMIRows())) {
        for (const int col : rgs::views::iota(0, layout_.getMICols(row))) {
            const auto neighbors = Neighbors::fromLayoutAndIndex(layout_, {col, row});
            const int offset = row_offset + col;
            const auto& psize = _estimatePatchsize(neighbors, offset);
            patchsizes_[offset] = psize;
        }
        row_offset += layout_.getMIMaxCols();
    }
}

} // namespace tlct::_cvt::raytrix
