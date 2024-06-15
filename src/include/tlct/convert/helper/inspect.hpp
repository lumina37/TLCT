#pragma once

#include <filesystem>
#include <ranges>
#include <sstream>
#include <string>

#include <opencv2/imgcodecs.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/common.hpp"
#include "tlct/config/tspc/layout.hpp"
#include "tlct/convert/helper/direction.hpp"

namespace tlct::cvt::_hp {

namespace fs = std::filesystem;
namespace rgs = std::ranges;
namespace tcfg = tlct::cfg;

class Inspector
{
public:
#ifdef TLCT_ENABLE_INSPECT
    static constexpr bool ENABLED = true;
#else
    static constexpr bool ENABLED = false;
#endif

    // Constructor
    inline Inspector() noexcept : dst_dir_(){};
    inline Inspector& operator=(const Inspector& rhs) noexcept = default;
    inline Inspector(const Inspector& rhs) noexcept = default;
    inline Inspector& operator=(Inspector&& rhs) noexcept = default;
    inline Inspector(Inspector&& rhs) noexcept = default;
    inline Inspector(const fs::path dst_dir) : dst_dir_(std::move(dst_dir)){};

    // Init from
    [[nodiscard]] static inline Inspector fromCommonCfgAndLayout(const tcfg::CommonParamConfig& common_cfg,
                                                                 const tcfg::tspc::Layout& layout);

    // Const methods
    [[nodiscard]] inline fs::path getDstDir() const noexcept { return dst_dir_; }
    [[nodiscard]] inline fs::path getMIDir(const int row, const int col) const noexcept;
    [[nodiscard]] inline fs::path getMIDir(const cv::Point index) const noexcept;
    [[nodiscard]] inline fs::path getDirectionDir(const cv::Point index, const Direction direction) const noexcept;

    // Utils
    static inline void saveMI(const Inspector& inspector, const cv::Mat& img, const cv::Point index);
    static inline void saveAnchor(const Inspector& inspector, const cv::Mat& img, const cv::Point index,
                                  const Direction direction);
    static inline void saveCmpPattern(const Inspector& inspector, const cv::Mat& img, const cv::Point index,
                                      const Direction direction, const int psize, const double metric);

private:
    fs::path dst_dir_;
};

Inspector Inspector::fromCommonCfgAndLayout(const tcfg::CommonParamConfig& common_cfg, const tcfg::tspc::Layout& layout)
{
    if constexpr (!ENABLED)
        return {};

    const fs::path dst_pattern{common_cfg.getDstPattern()};
    const fs::path dst_dir = dst_pattern.parent_path() / "inspect";
    Inspector inspector{dst_dir};

    return {dst_dir};
}

fs::path Inspector::getMIDir(const int row, const int col) const noexcept
{
    std::stringstream ss;
    ss << row << '-' << col;
    const fs::path mi_dir = dst_dir_ / ss.str();
    fs::create_directories(mi_dir);
    return mi_dir;
}

fs::path Inspector::getMIDir(const cv::Point index) const noexcept { return getMIDir(index.y, index.x); }

fs::path Inspector::getDirectionDir(const cv::Point index, const Direction direction) const noexcept
{
    const fs::path mi_dir = getMIDir(index.y, index.x);
    std::stringstream ss;
    ss << "d" << (int)direction;
    const fs::path direction_dir = mi_dir / ss.str();
    fs::create_directories(direction_dir);
    return direction_dir;
}

void Inspector::saveMI(const Inspector& inspector, const cv::Mat& img, const cv::Point index)
{
    const fs::path mi_dir = inspector.getMIDir(index);
    const fs::path save_path = mi_dir / "mi.png";
    cv::imwrite(save_path.string(), img);
}

void Inspector::saveAnchor(const Inspector& inspector, const cv::Mat& img, const cv::Point index,
                           const Direction direction)
{
    const fs::path dir = inspector.getDirectionDir(index, direction);
    const fs::path save_path = dir / "anchor.png";
    cv::imwrite(save_path.string(), img);
}

void Inspector::saveCmpPattern(const Inspector& inspector, const cv::Mat& img, const cv::Point index,
                               const Direction direction, const int psize, const double metric)
{
    const fs::path dir = inspector.getDirectionDir(index, direction);
    std::stringstream ss;
    ss << "cmp" << "-p" << psize << "-m" << std::fixed << std::setprecision(3) << metric << ".png";
    const fs::path save_path = dir / ss.str();
    cv::imwrite(save_path.string(), img);
}

} // namespace tlct::cvt::_hp
