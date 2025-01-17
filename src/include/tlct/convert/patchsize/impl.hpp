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

template <concepts::CNeighbors TNeighbors, bool IS_KEPLER, typename TLayout = TNeighbors::TLayout>
    requires std::is_same_v<TLayout, typename TNeighbors::TLayout>
[[nodiscard]] static inline PsizeMetric estimateWithNeighbor(const TLayout& layout, const PsizeParams_<TLayout>& params,
                                                             const MIs_<TLayout>& mis, const TNeighbors& neighbors,
                                                             WrapSSIM& wrap_anchor)
{
    const cv::Point2d mi_center{layout.getRadius(), layout.getRadius()};
    const int max_shift = (int)(params.pattern_shift * 2);

    double sum_psize = 0.0;
    double sum_metric = 0.0;
    double sum_psize_weight = std::numeric_limits<float>::epsilon();
    double sum_metric_weight = std::numeric_limits<float>::epsilon();

    for (const auto direction : TNeighbors::DIRECTIONS) {
        if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
            continue;
        }

        const cv::Point2d anchor_shift =
            -_hp::sgn(IS_KEPLER) * TNeighbors::getUnitShift(direction) * params.pattern_shift;
        const cv::Rect anchor_roi = getRoiByCenter(mi_center + anchor_shift, params.pattern_size);
        wrap_anchor.updateRoi(anchor_roi);

        const WrapMI& neib_mi = mis.getMI(neighbors.getNeighborIdx(direction));
        WrapSSIM wrap_neib{neib_mi};

        const cv::Point2d match_step = _hp::sgn(IS_KEPLER) * TNeighbors::getUnitShift(direction);
        cv::Point2d cmp_shift = anchor_shift + match_step * params.min_psize;

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
        const double metric = max_ssim * max_ssim;
        const double weighted_metric = weight * metric;
        sum_psize += best_psize * weighted_metric;
        sum_psize_weight += weighted_metric;
        sum_metric += weighted_metric;
        sum_metric_weight += weight;
    }

    const double cliped_sum_psize =
        _hp::clip(sum_psize / sum_psize_weight, (double)params.min_psize, (double)max_shift);
    const int psize = _hp::iround(cliped_sum_psize / TNeighbors::INFLATE);
    const double metric = sum_metric / sum_metric_weight;

    return {psize, metric};
}

template <tcfg::concepts::CLayout TLayout, bool IS_KEPLER, bool USE_FAR_NEIGHBOR>
[[nodiscard]] static inline PsizeRecord
estimatePatchsize(const TLayout& layout, const tcfg::CliConfig::Convert& cvt_cfg, const PsizeParams_<TLayout>& params,
                  const MIs_<TLayout>& mis, const std::vector<PsizeRecord>& prev_patchsizes, const cv::Point index)
{
    using NearNeighbors = NearNeighbors_<TLayout>;
    using FarNeighbors = FarNeighbors_<TLayout>;
    using PsizeParams = PsizeParams_<TLayout>;

    const int offset = index.y * layout.getMIMaxCols() + index.x;
    const WrapMI& anchor_mi = mis.getMI(offset);
    const uint64_t hash = dhash(anchor_mi.I);
    const PsizeRecord& prev_psize = prev_patchsizes[offset];

    if (prev_psize.psize != PsizeParams::INVALID_PSIZE) [[likely]] {
        const int hash_dist = L1Dist(prev_psize.hash, hash);
        if (hash_dist <= cvt_cfg.psize_shortcut_threshold) {
            return {prev_psize.psize, hash};
        }
    }

    WrapSSIM wrap_anchor{anchor_mi};
    const NearNeighbors& near_neighbors = NearNeighbors::fromLayoutAndIndex(layout, index);
    const PsizeMetric& near_psize_metric =
        estimateWithNeighbor<NearNeighbors, IS_KEPLER>(layout, params, mis, near_neighbors, wrap_anchor);
    double max_matric = near_psize_metric.metric;
    int best_psize = near_psize_metric.psize;

    if constexpr (USE_FAR_NEIGHBOR) {
        const FarNeighbors& far_neighbors = FarNeighbors::fromLayoutAndIndex(layout, index);
        const PsizeMetric& far_psize_metric =
            estimateWithNeighbor<FarNeighbors, IS_KEPLER>(layout, params, mis, far_neighbors, wrap_anchor);
        if (far_psize_metric.metric > max_matric) {
            best_psize = far_psize_metric.psize;
        }
    }

    return {best_psize, hash};
}

template <tcfg::concepts::CLayout TLayout, bool IS_KEPLER, bool USE_FAR_NEIGHBOR>
static inline void estimatePatchsizes(const TLayout& layout, const tcfg::CliConfig::Convert& cvt_cfg,
                                      const PsizeParams_<TLayout>& params, const MIs_<TLayout>& mis,
                                      const std::vector<PsizeRecord>& prev_patchsizes,
                                      std::vector<PsizeRecord>& patchsizes)
{
    for (const int row : rgs::views::iota(0, layout.getMIRows())) {
        for (const int col : rgs::views::iota(0, layout.getMICols(row))) {
            const cv::Point index{col, row};
            const PsizeRecord& psize = estimatePatchsize<TLayout, IS_KEPLER, USE_FAR_NEIGHBOR>(
                layout, cvt_cfg, params, mis, prev_patchsizes, index);
            const int offset = index.y * layout.getMIMaxCols() + index.x;
            patchsizes[offset] = psize;
        }
    }
}

} // namespace tlct::_cvt
