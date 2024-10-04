#pragma once

#include <numeric>
#include <ranges>
#include <vector>

#include <opencv2/core.hpp>

#include "neighbors.hpp"
#include "params.hpp"
#include "record.hpp"
#include "ssim.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/helper/static_math.hpp"

namespace tlct::_cvt {

namespace rgs = std::ranges;

template <typename TLayout>
    requires tlct::cfg::concepts::CLayout<TLayout>
[[nodiscard]] static inline PsizeRecord
estimatePatchsize(const TLayout& layout, const typename TLayout::TSpecificConfig& spec_cfg,
                  const PsizeParams_<TLayout>& params, const MIs_<TLayout>& mis,
                  const std::vector<PsizeRecord>& prev_patchsizes, const Neighbors_<TLayout>& neighbors, int offset)
{
    const auto& anchor_mi = mis.getMI(offset);
    const uint64_t hash = dhash(anchor_mi.I);
    const auto& prev_psize = prev_patchsizes[offset];

    if (prev_psize.psize != 0) [[likely]] {
        const int hash_dist = L1_dist(prev_psize.hash, hash);
        if (hash_dist < spec_cfg.getPsizeShortcutThreshold()) {
            return {prev_psize.psize, hash};
        }
    }

    const cv::Point2d mi_center{layout.getRadius(), layout.getRadius()};
    WrapSSIM wrap_anchor{anchor_mi};

    const int max_shift = (int)(params.pattern_shift * 2);

    double weighted_psize = 0.0;
    double total_weight = 0.0;

    for (const auto direction : DIRECTIONS) {
        if (neighbors.hasNeighbor(direction)) [[likely]] {
            const cv::Point2d anchor_shift =
                -_hp::sgn(TLayout::IS_KEPLER) * neighbors.getUnitShift(direction) * params.pattern_shift;
            const cv::Rect anchor_roi = getRoiByCenter(mi_center + anchor_shift, params.pattern_size);
            wrap_anchor.updateRoi(anchor_roi);

            const auto& neib_mi = mis.getMI(neighbors.getNeighborIdx(direction));
            WrapSSIM wrap_neib{neib_mi};

            const cv::Point2d match_step = _hp::sgn(TLayout::IS_KEPLER) * neighbors.getUnitShift(direction);
            cv::Point2d cmp_shift = anchor_shift + match_step * params.min_psize;

            int min_metric_psize = 0;
            double min_metric = std::numeric_limits<double>::max();
            std::vector<double> metrics;
            metrics.reserve(max_shift - params.min_psize);
            for (const int psize : rgs::views::iota(params.min_psize, max_shift)) {
                cmp_shift += match_step;

                const cv::Rect cmp_roi = getRoiByCenter(mi_center + cmp_shift, params.pattern_size);
                wrap_neib.updateRoi(cmp_roi);

                const double metric = wrap_anchor.compare(wrap_neib);
                metrics.push_back(metric);

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
    return {final_psize, hash};
}

template <typename TLayout>
    requires tlct::cfg::concepts::CLayout<TLayout>
static inline void estimatePatchsizes(const TLayout& layout, const typename TLayout::TSpecificConfig& spec_cfg,
                                      const PsizeParams_<TLayout>& params, const MIs_<TLayout>& mis,
                                      const std::vector<PsizeRecord>& prev_patchsizes,
                                      std::vector<PsizeRecord>& patchsizes)
{
    int row_offset = 0;
    for (const int row : rgs::views::iota(0, layout.getMIRows())) {
        for (const int col : rgs::views::iota(0, layout.getMICols(row))) {
            const auto neighbors = Neighbors_<TLayout>::fromLayoutAndIndex(layout, {col, row});
            const int offset = row_offset + col;
            const auto& psize =
                estimatePatchsize<TLayout>(layout, spec_cfg, params, mis, prev_patchsizes, neighbors, offset);
            patchsizes[offset] = psize;
        }
        row_offset += layout.getMIMaxCols();
    }
}

} // namespace tlct::_cvt
