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
#elif
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

    // Utils
    static inline void saveMI(const Inspector& inspector, const cv::Mat& img, const cv::Point index);
    static inline void saveAnchor(const Inspector& inspector, const cv::Mat& img, const cv::Point index);
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

    for (const int irow : rgs::views::iota(0, layout.getMIRows())) {
        for (const int icol : rgs::views::iota(0, layout.getMICols(irow))) {
            const fs::path mi_dir = inspector.getMIDir(irow, icol);
            fs::create_directory(mi_dir);
        }
    }

    return inspector;
}

fs::path Inspector::getMIDir(const int row, const int col) const noexcept
{
    std::stringstream ss;
    ss << row << '-' << col;
    const fs::path mi_dir = dst_dir_ / ss.str();
    return mi_dir;
}

fs::path Inspector::getMIDir(const cv::Point index) const noexcept { return getMIDir(index.y, index.x); }

void Inspector::saveMI(const Inspector& inspector, const cv::Mat& img, const cv::Point index)
{
    if constexpr (!ENABLED)
        return;

    const fs::path mi_dir = inspector.getMIDir(index);
    const fs::path save_path = mi_dir / "mi.png";
    cv::imwrite(save_path.string(), img);
}

void Inspector::saveAnchor(const Inspector& inspector, const cv::Mat& img, const cv::Point index)
{
    if constexpr (!ENABLED)
        return;

    const fs::path mi_dir = inspector.getMIDir(index);
    const fs::path save_path = mi_dir / "anchor.png";
    cv::imwrite(save_path.string(), img);
}

void Inspector::saveCmpPattern(const Inspector& inspector, const cv::Mat& img, const cv::Point index,
                               const Direction direction, const int psize, const double metric)
{
    if constexpr (!ENABLED)
        return;

    const fs::path mi_dir = inspector.getMIDir(index);
    std::stringstream ss;
    ss << "cmp-d" << (int)direction << "-p" << psize << "-m" << std::fixed << std::setprecision(3) << metric << ".png";
    const fs::path save_path = mi_dir / ss.str();
    cv::imwrite(save_path.string(), img);
}

} // namespace tlct::cvt::_hp
