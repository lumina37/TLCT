#pragma once

#include <limits>
#include <ranges>
#include <vector>

#include <opencv2/core.hpp>

#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/convert/raytrix/patchsize/neighbors.hpp"
#include "tlct/convert/raytrix/patchsize/params.hpp"
#include "tlct/helper/constexpr/math.hpp"

namespace tlct::_cvt::raytrix {

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg::raytrix;

[[nodiscard]] static inline PsizeMetric evaluatePsize(const tcfg::Layout& layout, const PsizeParams& params,
                                                      const MIs_<tcfg::Layout>& mis, const NearNeighbors& neighbors,
                                                      WrapSSIM& wrap_anchor, const int psize)
{
    const cv::Point2d mi_center{layout.getRadius(), layout.getRadius()};

    double sum_metric = 0.0;
    double sum_metric_weight = std::numeric_limits<float>::epsilon();

    for (const auto direction : NearNeighbors::DIRECTIONS) {
        if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
            continue;
        }

        const cv::Point2d anchor_shift =
            -_hp::sgn(tcfg::Layout::IS_KEPLER) * NearNeighbors::getUnitShift(direction) * params.pattern_shift;
        const cv::Rect anchor_roi = getRoiByCenter(mi_center + anchor_shift, params.pattern_size);
        wrap_anchor.updateRoi(anchor_roi);

        const auto& neib_mi = mis.getMI(neighbors.getNeighborIdx(direction));
        WrapSSIM wrap_neib{neib_mi};

        const cv::Point2d match_step = _hp::sgn(tcfg::Layout::IS_KEPLER) * NearNeighbors::getUnitShift(direction);
        cv::Point2d cmp_shift = anchor_shift + match_step * psize;
        const cv::Rect neib_roi = getRoiByCenter(mi_center + cmp_shift, params.pattern_size);
        wrap_neib.updateRoi(neib_roi);

        const double ssim = wrap_anchor.compare(wrap_neib);
        const double weight = textureIntensity(wrap_anchor.I_);
        sum_metric += weight * (ssim * ssim);
        sum_metric_weight += weight;
    };

    const double metric = sum_metric / sum_metric_weight;
    return {psize, metric};
}

template <concepts::CNeighbors TNeighbors>
[[nodiscard]] static inline PsizeMetric estimateWithNeighbor(const tcfg::Layout& layout, const PsizeParams& params,
                                                             const MIs_<tcfg::Layout>& mis, const TNeighbors& neighbors,
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
            -_hp::sgn(tcfg::Layout::IS_KEPLER) * TNeighbors::getUnitShift(direction) * params.pattern_shift;
        const cv::Rect anchor_roi = getRoiByCenter(mi_center + anchor_shift, params.pattern_size);
        wrap_anchor.updateRoi(anchor_roi);

        const auto& neib_mi = mis.getMI(neighbors.getNeighborIdx(direction));
        WrapSSIM wrap_neib{neib_mi};

        const cv::Point2d match_step = _hp::sgn(tcfg::Layout::IS_KEPLER) * TNeighbors::getUnitShift(direction);
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

[[nodiscard]] static inline PsizeRecord
estimatePatchsize(const tcfg::Layout& layout, const typename tcfg::SpecificConfig& spec_cfg, const PsizeParams& params,
                  const MIs_<tcfg::Layout>& mis, const std::vector<PsizeRecord>& patchsizes,
                  const std::vector<PsizeRecord>& prev_patchsizes, const cv::Point index, const int offset)
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
    const auto near_neighbors = NearNeighbors::fromLayoutAndIndex(layout, index);
    const auto near_psize_metric = estimateWithNeighbor(layout, params, mis, near_neighbors, wrap_anchor);
    double max_matric = near_psize_metric.metric;
    int best_psize = near_psize_metric.psize;

    const auto far_neighbors = FarNeighbors::fromLayoutAndIndex(layout, index);
    const auto far_psize_metric = estimateWithNeighbor(layout, params, mis, far_neighbors, wrap_anchor);
    if (far_psize_metric.metric > max_matric) {
        max_matric = far_psize_metric.metric;
        best_psize = far_psize_metric.psize;
    }

    for (const auto direction : {
             NearNeighbors::Direction::UPLEFT,
             NearNeighbors::Direction::UPRIGHT,
             NearNeighbors::Direction::LEFT,
         }) {
        if (!near_neighbors.hasNeighbor(direction)) [[unlikely]] {
            continue;
        }

        const auto ref_idx = near_neighbors.getNeighborIdx(direction);
        const int ref_offset = ref_idx.y * layout.getMIMaxCols() + ref_idx.x;
        const int ref_psize = patchsizes[ref_offset].psize;
        const auto ref_psize_metric = evaluatePsize(layout, params, mis, near_neighbors, wrap_anchor, ref_psize);

        if (ref_psize_metric.metric > max_matric) {
            max_matric = ref_psize_metric.metric;
            best_psize = ref_psize_metric.psize;
        }
    }

    return {best_psize, hash};
}

static inline void estimatePatchsizes(const tcfg::Layout& layout, const typename tcfg::SpecificConfig& spec_cfg,
                                      const PsizeParams& params, const MIs_<tcfg::Layout>& mis,
                                      const std::vector<PsizeRecord>& prev_patchsizes,
                                      std::vector<PsizeRecord>& patchsizes)
{
    int row_offset = 0;
    for (const int row : rgs::views::iota(0, layout.getMIRows())) {
        int offset = row_offset;
        for (const int col : rgs::views::iota(0, layout.getMICols(row))) {
            const cv::Point index{col, row};
            const auto& psize =
                estimatePatchsize(layout, spec_cfg, params, mis, patchsizes, prev_patchsizes, index, offset);
            patchsizes[offset] = psize;
            offset++;
        }
        row_offset += layout.getMIMaxCols();
    }
}

} // namespace tlct::_cvt::raytrix
