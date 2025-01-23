#pragma once

#include <limits>
#include <ranges>
#include <vector>

#include <opencv2/core.hpp>

#include "tlct/config/concepts.hpp"
#include "tlct/convert/concepts.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/convert/patchsize/params.hpp"
#include "tlct/helper/constexpr/math.hpp"

namespace tlct::_cvt {

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg;

template <concepts::CNeighbors TNeighbors, bool IS_KEPLER, typename TArrange = TNeighbors::TArrange>
    requires std::is_same_v<TArrange, typename TNeighbors::TArrange>
[[nodiscard]] static inline PsizeMetric
estimateWithNeighbor(const TArrange& arrange, const PsizeParams_<TArrange>& params, const MIBuffers_<TArrange>& mis,
                     const TNeighbors& neighbors, WrapSSIM& wrap_anchor)
{
    const cv::Point2f mi_center{arrange.getRadius(), arrange.getRadius()};
    const int max_shift = (int)(params.pattern_shift * 2);

    float sum_psize = 0.0;
    float sum_metric = 0.0;
    float sum_psize_weight = std::numeric_limits<float>::epsilon();
    float sum_metric_weight = std::numeric_limits<float>::epsilon();

    for (const auto direction : TNeighbors::DIRECTIONS) {
        if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
            continue;
        }

        const cv::Point2f anchor_shift =
            -_hp::sgn(IS_KEPLER) * TNeighbors::getUnitShift(direction) * params.pattern_shift;
        const cv::Rect anchor_roi = getRoiByCenter(mi_center + anchor_shift, params.pattern_size);
        wrap_anchor.updateRoi(anchor_roi);

        const MIBuffer& neib_mi = mis.getMI(neighbors.getNeighborIdx(direction));
        WrapSSIM wrap_neib{neib_mi};

        const cv::Point2f match_step = _hp::sgn(IS_KEPLER) * TNeighbors::getUnitShift(direction);
        cv::Point2f cmp_shift = anchor_shift + match_step * params.min_psize;

        int best_psize = 0;
        float max_ssim = 0.0;
        for (const int psize : rgs::views::iota(params.min_psize, max_shift)) {
            cmp_shift += match_step;

            const cv::Rect cmp_roi = getRoiByCenter(mi_center + cmp_shift, params.pattern_size);
            wrap_neib.updateRoi(cmp_roi);

            const float ssim = wrap_anchor.compare(wrap_neib);
            if (ssim > max_ssim) {
                max_ssim = ssim;
                best_psize = psize;
            }
        }

        const float weight = textureIntensity(wrap_anchor.I_);
        const float metric = max_ssim * max_ssim;
        const float weighted_metric = weight * metric;
        sum_psize += best_psize * weighted_metric;
        sum_psize_weight += weighted_metric;
        sum_metric += weighted_metric;
        sum_metric_weight += weight;
    }

    const float cliped_sum_psize = _hp::clip(sum_psize / sum_psize_weight, (float)params.min_psize, (float)max_shift);
    const int psize = _hp::iround(cliped_sum_psize / TNeighbors::INFLATE);
    const float metric = sum_metric / sum_metric_weight;

    return {psize, metric};
}

template <tcfg::concepts::CArrange TArrange, bool IS_KEPLER, bool USE_FAR_NEIGHBOR>
[[nodiscard]] static inline PsizeRecord
estimatePatchsize(const TArrange& arrange, const tcfg::CliConfig::Convert& cvt_cfg,
                  const PsizeParams_<TArrange>& params, const MIBuffers_<TArrange>& mis,
                  const std::vector<PsizeRecord>& prev_patchsizes, const cv::Point index)
{
    using NearNeighbors = NearNeighbors_<TArrange>;
    using FarNeighbors = FarNeighbors_<TArrange>;
    using PsizeParams = PsizeParams_<TArrange>;

    const int offset = index.y * arrange.getMIMaxCols() + index.x;
    const MIBuffer& anchor_mi = mis.getMI(offset);
    const uint64_t hash = dhash(anchor_mi.I);
    const PsizeRecord& prev_psize = prev_patchsizes[offset];

    if (prev_psize.psize != PsizeParams::INVALID_PSIZE) [[likely]] {
        const int hash_dist = L1Dist(prev_psize.hash, hash);
        if (hash_dist <= cvt_cfg.psize_shortcut_threshold) {
            return {prev_psize.psize, hash};
        }
    }

    WrapSSIM wrap_anchor{anchor_mi};
    const NearNeighbors& near_neighbors = NearNeighbors::fromArrangeAndIndex(arrange, index);
    const PsizeMetric& near_psize_metric =
        estimateWithNeighbor<NearNeighbors, IS_KEPLER>(arrange, params, mis, near_neighbors, wrap_anchor);
    float max_matric = near_psize_metric.metric;
    int best_psize = near_psize_metric.psize;

    if constexpr (USE_FAR_NEIGHBOR) {
        const FarNeighbors& far_neighbors = FarNeighbors::fromArrangeAndIndex(arrange, index);
        const PsizeMetric& far_psize_metric =
            estimateWithNeighbor<FarNeighbors, IS_KEPLER>(arrange, params, mis, far_neighbors, wrap_anchor);
        if (far_psize_metric.metric > max_matric) {
            best_psize = far_psize_metric.psize;
        }
    }

    return {best_psize, hash};
}

template <tcfg::concepts::CArrange TArrange, bool IS_KEPLER, bool USE_FAR_NEIGHBOR>
static inline void estimatePatchsizes(const TArrange& arrange, const tcfg::CliConfig::Convert& cvt_cfg,
                                      const PsizeParams_<TArrange>& params, const MIBuffers_<TArrange>& mis,
                                      const std::vector<PsizeRecord>& prev_patchsizes,
                                      std::vector<PsizeRecord>& patchsizes)
{
    for (const int row : rgs::views::iota(0, arrange.getMIRows())) {
        for (const int col : rgs::views::iota(0, arrange.getMICols(row))) {
            const cv::Point index{col, row};
            const PsizeRecord& psize = estimatePatchsize<TArrange, IS_KEPLER, USE_FAR_NEIGHBOR>(
                arrange, cvt_cfg, params, mis, prev_patchsizes, index);
            const int offset = index.y * arrange.getMIMaxCols() + index.x;
            patchsizes[offset] = psize;
        }
    }
}

} // namespace tlct::_cvt
