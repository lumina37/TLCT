#pragma once

#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <ranges>
#include <sstream>
#include <string>

#include <opencv2/imgcodecs.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/common.hpp"
#include "tlct/config/concepts/layout.hpp"
#include "tlct/config/tspc/layout.hpp"
#include "tlct/convert/helper/direction.hpp"

namespace tlct::cvt::_hp {

namespace fs = std::filesystem;
namespace rgs = std::ranges;
namespace tcfg = tlct::cfg;

template <typename TLayout_>
    requires tcfg::concepts::CLayout<TLayout_>
class Inspector_
{
public:
#ifdef TLCT_ENABLE_INSPECT
    static constexpr bool ENABLED = true;
#else
    static constexpr bool ENABLED = false;
#endif
    static constexpr bool PATTERN_ENABLED = ENABLED;
    static constexpr bool METRIC_REPORT_ENABLED = ENABLED;

    // Typename alias
    using TLayout = TLayout_;

    // Constructor
    inline Inspector_() noexcept : dst_dir_(), fstream_(){};
    inline Inspector_& operator=(const Inspector_& rhs) noexcept = default;
    inline Inspector_(const Inspector_& rhs) noexcept = default;
    inline Inspector_& operator=(Inspector_&& rhs) noexcept = default;
    inline Inspector_(Inspector_&& rhs) noexcept = default;
    inline Inspector_(const fs::path dst_dir, std::ofstream&& fstream)
        : dst_dir_(std::move(dst_dir)), fstream_(std::move(fstream)){};

    // Init from
    [[nodiscard]] static inline Inspector_ fromCommonCfgAndLayout(const tcfg::CommonParamConfig& common_cfg,
                                                                  const TLayout& layout);

    // Const methods
    [[nodiscard]] inline fs::path getDstDir() const noexcept { return dst_dir_; }
    [[nodiscard]] inline fs::path getMIDir(const int row, const int col) const noexcept;
    [[nodiscard]] inline fs::path getMIDir(const cv::Point index) const noexcept;
    [[nodiscard]] inline fs::path getDirectionDir(const cv::Point index, const Direction direction) const noexcept;

    // Utils
    static inline void saveMI(const Inspector_& inspector, const cv::Mat& img, const cv::Point index);
    static inline void saveAnchor(const Inspector_& inspector, const cv::Mat& img, const cv::Point index,
                                  const Direction direction);
    static inline void saveCmpPattern(const Inspector_& inspector, const cv::Mat& img, const cv::Point index,
                                      const Direction direction, const int psize, const double metric);
    inline void appendMetricReport(const cv::Point index, const std::vector<double>& psizes,
                                   const std::vector<double>& weights);

private:
    fs::path dst_dir_;
    std::ofstream fstream_;
};

template <typename TLayout>
    requires tcfg::concepts::CLayout<TLayout>
Inspector_<TLayout> Inspector_<TLayout>::fromCommonCfgAndLayout(const tcfg::CommonParamConfig& common_cfg,
                                                                const TLayout& layout)
{
    if (!ENABLED)
        return {};

    const fs::path dst_pattern{common_cfg.getDstPattern()};
    const fs::path dst_dir = dst_pattern.parent_path() / "inspect";
    fs::create_directories(dst_dir);
    std::ofstream fstream{dst_dir / "report.csv"};

    return {dst_dir, std::move(fstream)};
}

template <typename TLayout>
    requires tcfg::concepts::CLayout<TLayout>
fs::path Inspector_<TLayout>::getMIDir(const int row, const int col) const noexcept
{
    std::stringstream ss;
    ss << row << '-' << col;
    const fs::path mi_dir = dst_dir_ / ss.str();
    fs::create_directories(mi_dir);
    return mi_dir;
}

template <typename TLayout>
    requires tcfg::concepts::CLayout<TLayout>
fs::path Inspector_<TLayout>::getMIDir(const cv::Point index) const noexcept
{
    return getMIDir(index.y, index.x);
}

template <typename TLayout>
    requires tcfg::concepts::CLayout<TLayout>
fs::path Inspector_<TLayout>::getDirectionDir(const cv::Point index, const Direction direction) const noexcept
{
    const fs::path mi_dir = getMIDir(index.y, index.x);
    std::stringstream ss;
    ss << "d" << (int)direction;
    const fs::path direction_dir = mi_dir / ss.str();
    fs::create_directories(direction_dir);
    return direction_dir;
}

template <typename TLayout>
    requires tcfg::concepts::CLayout<TLayout>
void Inspector_<TLayout>::saveMI(const Inspector_& inspector, const cv::Mat& img, const cv::Point index)
{
    const fs::path mi_dir = inspector.getMIDir(index);
    const fs::path save_path = mi_dir / "mi.png";
    cv::imwrite(save_path.string(), img);
}

template <typename TLayout>
    requires tcfg::concepts::CLayout<TLayout>
void Inspector_<TLayout>::saveAnchor(const Inspector_& inspector, const cv::Mat& img, const cv::Point index,
                                     const Direction direction)
{
    const fs::path dir = inspector.getDirectionDir(index, direction);
    const fs::path save_path = dir / "anchor.png";
    cv::imwrite(save_path.string(), img);
}

template <typename TLayout>
    requires tcfg::concepts::CLayout<TLayout>
void Inspector_<TLayout>::saveCmpPattern(const Inspector_& inspector, const cv::Mat& img, const cv::Point index,
                                         const Direction direction, const int psize, const double metric)
{
    const fs::path dir = inspector.getDirectionDir(index, direction);
    std::stringstream ss;
    ss << "cmp" << "-p" << psize << "-m" << std::fixed << std::setprecision(3) << metric << ".png";
    const fs::path save_path = dir / ss.str();
    cv::imwrite(save_path.string(), img);
}

template <typename TLayout>
    requires tcfg::concepts::CLayout<TLayout>
void Inspector_<TLayout>::appendMetricReport(const cv::Point index, const std::vector<double>& psizes,
                                             const std::vector<double>& weights)
{
    double total_psize = 0.0;
    double total_weight = std::numeric_limits<double>::epsilon();
    for (int i = 0; i < psizes.size(); i++) {
        total_psize += psizes[i] * weights[i];
        total_weight += weights[i];
    }
    const double mean = total_psize / total_weight;

    double var = 0;
    for (int i = 0; i < psizes.size(); i++) {
        const double psize = psizes[i];
        var += (psize - mean) * (psize - mean) * weights[i];
    }
    var /= total_weight;
    var = std::sqrt(var);
    fstream_ << index.y << ',' << index.x << ',' << var;
    for (int i = 0; i < psizes.size(); i++) {
        fstream_ << ',' << psizes[i] << ',' << weights[i];
    }
    fstream_ << '\n';
}

} // namespace tlct::cvt::_hp
