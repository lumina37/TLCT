#pragma once

#include <numeric>
#include <ranges>
#include <vector>

#include <opencv2/core.hpp>

#include "neighbors.hpp"
#include "params.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/helper/constexpr/math.hpp"

namespace tlct::_cvt::tspc {

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg::tspc;

template <concepts::CNeighbors TNeighbors>
[[nodiscard]] static inline PsizeMetric estimateWithNeighbor(const tcfg::Layout& layout, const PsizeParams& params,
                                                             const MIs_<tcfg::Layout>& mis, const TNeighbors& neighbors,
                                                             WrapSSIM& wrap_anchor)
{
    const cv::Point2d mi_center{layout.getRadius(), layout.getRadius()};

    double weighted_psize = std::numeric_limits<float>::epsilon();
    double weighted_metric = std::numeric_limits<float>::epsilon();
    double total_weight = std::numeric_limits<float>::epsilon();

    for (const auto direction : TNeighbors::DIRECTIONS) {
        if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
            continue;
        }

        const cv::Point2d anchor_shift =
            -_hp::sgn(tcfg::Layout::IS_KEPLER) * TNeighbors::getUnitShift(direction) * params.pattern_shift;
        const cv::Rect anchor_roi = getRoiByCenter(mi_center + anchor_shift, params.pattern_size);
        wrap_anchor.updateRoi(anchor_roi);

        const auto& neib_mi = mis.getMI(neighbors.getNeighborIdx(direction));
        WrapSSIM wrap_neib{neib_mi};

        const cv::Point2d match_step = _hp::sgn(tcfg::Layout::IS_KEPLER) * TNeighbors::getUnitShift(direction);
        cv::Point2d cmp_shift = anchor_shift + match_step * params.min_psize;

        const int max_shift = (int)(params.pattern_shift * 2);
        int best_psize = 0;
        double max_ssim = 0.0;
        for (const int psize : rgs::views::iota(params.min_psize, max_shift)) {
            cmp_shift += match_step;

            const cv::Rect cmp_roi = getRoiByCenter(mi_center + cmp_shift, params.pattern_size);
            wrap_neib.updateRoi(cmp_roi);

            const double ssim = wrap_anchor.compare(wrap_neib);
            if (ssim > max_ssim) {
                max_ssim = ssim;
                best_psize = psize;
            }
        }

        const double weight = textureIntensity(wrap_anchor.I_);
        weighted_psize += weight * best_psize;
        weighted_metric += weight * (max_ssim * max_ssim);
        total_weight += weight;
    }

    const int psize = _hp::iround(weighted_psize / total_weight);
    const double metric = weighted_metric / total_weight;

    return {psize, metric};
}

[[nodiscard]] static inline PsizeRecord estimatePatchsize(const tcfg::Layout& layout,
                                                          const typename tcfg::SpecificConfig& spec_cfg,
                                                          const PsizeParams& params, const MIs_<tcfg::Layout>& mis,
                                                          const std::vector<PsizeRecord>& prev_patchsizes,
                                                          cv::Point index, int offset)
{
    const auto& anchor_mi = mis.getMI(offset);
    const uint64_t hash = dhash(anchor_mi.I);
    const auto& prev_psize = prev_patchsizes[offset];

    if (prev_psize.psize != PsizeParams::INVALID_PSIZE) [[likely]] {
        const int hash_dist = L1Dist(prev_psize.hash, hash);
        if (hash_dist <= spec_cfg.getPsizeShortcutThreshold()) {
            return {prev_psize.psize, hash};
        }
    }

    WrapSSIM wrap_anchor{anchor_mi};
    const auto neighbors = Neighbors::fromLayoutAndIndex(layout, index);
    const auto psize_metric = estimateWithNeighbor(layout, params, mis, neighbors, wrap_anchor);

    return {psize_metric.psize, hash};
}

static inline void estimatePatchsizes(const tcfg::Layout& layout, const typename tcfg::SpecificConfig& spec_cfg,
                                      const PsizeParams& params, const MIs_<tcfg::Layout>& mis,
                                      const std::vector<PsizeRecord>& prev_patchsizes,
                                      std::vector<PsizeRecord>& patchsizes)
{
    int row_offset = 0;
    for (const int row : rgs::views::iota(0, layout.getMIRows())) {
        for (const int col : rgs::views::iota(0, layout.getMICols(row))) {
            const int offset = row_offset + col;
            const cv::Point index{col, row};
            const auto& psize = estimatePatchsize(layout, spec_cfg, params, mis, prev_patchsizes, index, offset);
            patchsizes[offset] = psize;
        }
        row_offset += layout.getMIMaxCols();
    }
}

} // namespace tlct::_cvt::tspc
