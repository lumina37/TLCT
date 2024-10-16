#pragma once

#include <numeric>
#include <ranges>
#include <vector>

#include <opencv2/core.hpp>

#include "neighbors.hpp"
#include "params.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/convert/helper/record.hpp"
#include "tlct/convert/helper/ssim.hpp"
#include "tlct/helper/constexpr/math.hpp"

namespace tlct::_cvt::raytrix {

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg::raytrix;

[[nodiscard]] static inline PsizeRecord
estimatePatchsize(const tcfg::Layout& layout, const typename tcfg::SpecificConfig& spec_cfg, const PsizeParams& params,
                  const MIs_<tcfg::Layout>& mis, const std::vector<PsizeRecord>& prev_patchsizes,
                  const NearNeighbors& near_neighbors, const FarNeighbors& far_neighbors, int offset)
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

    const cv::Point2d mi_center{layout.getRadius(), layout.getRadius()};
    WrapSSIM wrap_anchor{anchor_mi};

    const int max_shift = (int)(params.pattern_shift * 2);

    double near_weighted_psize = std::numeric_limits<float>::epsilon();
    double near_weighted_ssim_2 = std::numeric_limits<float>::epsilon();
    double near_total_weight = std::numeric_limits<float>::epsilon();

    for (const auto direction : NearNeighbors::DIRECTIONS) {
        if (near_neighbors.hasNeighbor(direction)) [[likely]] {
            const cv::Point2d anchor_shift =
                -_hp::sgn(tcfg::Layout::IS_KEPLER) * NearNeighbors::getUnitShift(direction) * params.pattern_shift;
            const cv::Rect anchor_roi = getRoiByCenter(mi_center + anchor_shift, params.pattern_size);
            wrap_anchor.updateRoi(anchor_roi);

            const auto& neib_mi = mis.getMI(near_neighbors.getNeighborIdx(direction));
            WrapSSIM wrap_neib{neib_mi};

            const cv::Point2d match_step = _hp::sgn(tcfg::Layout::IS_KEPLER) * NearNeighbors::getUnitShift(direction);
            cv::Point2d cmp_shift = anchor_shift + match_step * params.min_psize;

            int best_psize = 0;
            double max_ssim = 0.0;
            for (const int psize : rgs::views::iota(params.min_psize, max_shift)) {
                cmp_shift += match_step;

                const cv::Rect cmp_roi = getRoiByCenter(mi_center + cmp_shift, params.pattern_size);
                wrap_neib.updateRoi(cmp_roi);

                const double metric = wrap_anchor.compare(wrap_neib);
                if (metric > max_ssim) {
                    max_ssim = metric;
                    best_psize = psize;
                }
            }

            const double weight = textureIntensity(wrap_anchor.I_);
            near_weighted_psize += weight * best_psize;
            near_weighted_ssim_2 += weight * (max_ssim * max_ssim);
            near_total_weight += weight;
        }
    }

    double far_weighted_psize = std::numeric_limits<float>::epsilon();
    double far_weighted_ssim_2 = std::numeric_limits<float>::epsilon();
    double far_total_weight = std::numeric_limits<float>::epsilon();

    for (const auto direction : FarNeighbors::DIRECTIONS) {
        if (far_neighbors.hasNeighbor(direction)) [[likely]] {
            const cv::Point2d anchor_shift =
                -_hp::sgn(tcfg::Layout::IS_KEPLER) * FarNeighbors::getUnitShift(direction) * params.pattern_shift;
            const cv::Rect anchor_roi = getRoiByCenter(mi_center + anchor_shift, params.pattern_size);
            wrap_anchor.updateRoi(anchor_roi);

            const auto& neib_mi = mis.getMI(far_neighbors.getNeighborIdx(direction));
            WrapSSIM wrap_neib{neib_mi};

            const cv::Point2d match_step = _hp::sgn(tcfg::Layout::IS_KEPLER) * FarNeighbors::getUnitShift(direction);
            cv::Point2d cmp_shift = anchor_shift + match_step * params.min_psize;

            int best_psize = 0;
            double max_ssim = 0.0;
            for (const int psize : rgs::views::iota(params.min_psize, max_shift)) {
                cmp_shift += match_step;

                const cv::Rect cmp_roi = getRoiByCenter(mi_center + cmp_shift, params.pattern_size);
                wrap_neib.updateRoi(cmp_roi);

                const double metric = wrap_anchor.compare(wrap_neib);
                if (metric > max_ssim) {
                    max_ssim = metric;
                    best_psize = psize;
                }
            }

            const double weight = textureIntensity(wrap_anchor.I_);
            far_weighted_psize += weight * best_psize / FarNeighbors::INFLATE;
            far_weighted_ssim_2 += weight * (max_ssim * max_ssim);
            far_total_weight += weight;
        }
    }

    int final_psize;
    if (near_weighted_ssim_2 > far_weighted_ssim_2) {
        final_psize = _hp::iround(near_weighted_psize / near_total_weight);
    } else {
        final_psize = _hp::iround(far_weighted_psize / far_total_weight);
    }

    return {final_psize, hash};
}

static inline void estimatePatchsizes(const tcfg::Layout& layout, const typename tcfg::SpecificConfig& spec_cfg,
                                      const PsizeParams& params, const MIs_<tcfg::Layout>& mis,
                                      const std::vector<PsizeRecord>& prev_patchsizes,
                                      std::vector<PsizeRecord>& patchsizes)
{
    int row_offset = 0;
    for (const int row : rgs::views::iota(0, layout.getMIRows())) {
        for (const int col : rgs::views::iota(0, layout.getMICols(row))) {
            const auto near_neighbors = NearNeighbors::fromLayoutAndIndex(layout, {col, row});
            const auto far_neighbors = FarNeighbors::fromLayoutAndIndex(layout, {col, row});
            const int offset = row_offset + col;
            const auto& psize = estimatePatchsize(layout, spec_cfg, params, mis, prev_patchsizes, near_neighbors,
                                                  far_neighbors, offset);
            patchsizes[offset] = psize;
        }
        row_offset += layout.getMIMaxCols();
    }
}

} // namespace tlct::_cvt::raytrix
