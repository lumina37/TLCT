#pragma once

#include <cmath>
#include <filesystem>
#include <functional>
#include <numeric>
#include <sstream>
#include <string>

#include <opencv2/imgcodecs.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/common.hpp"

namespace tlct::_cvt {

namespace fs = std::filesystem;
namespace tcfg = tlct::cfg;

class Inspector
{
public:
#ifdef TLCT_ENABLE_INSPECT
    static constexpr bool ENABLED = true;
#else
    static constexpr bool ENABLED = false;
#endif

    using FnEnableIf = bool(cv::Point);
    static constexpr auto ALWAYS_ENABLE = [](cv::Point) { return true; };

    // Constructor
    inline Inspector() noexcept : dst_dir_(), enable_if_(ALWAYS_ENABLE) {};
    Inspector(const Inspector& rhs) noexcept = delete;
    Inspector& operator=(const Inspector& rhs) noexcept = delete;
    inline Inspector(Inspector&& rhs) noexcept = default;
    inline Inspector& operator=(Inspector&& rhs) noexcept = default;
    inline explicit Inspector(fs::path&& dst_dir) : dst_dir_(std::move(dst_dir)), enable_if_(ALWAYS_ENABLE) {};

    // Init from
    [[nodiscard]] static inline Inspector fromGenericCfg(const tcfg::GenericParamConfig& generic_cfg);

    // Const methods
    [[nodiscard]] inline fs::path getDstDir() const noexcept { return dst_dir_; }
    [[nodiscard]] inline fs::path getMIDir(int row, int col) const noexcept;
    [[nodiscard]] inline fs::path getMIDir(cv::Point index) const noexcept;
    [[nodiscard]] inline fs::path getDirectionDir(cv::Point index, int direction) const noexcept;

    inline void saveMI(const cv::Mat& img, cv::Point index) const;
    inline void saveAnchor(const cv::Mat& img, cv::Point index, int direction) const;
    inline void saveCmpPattern(const cv::Mat& img, cv::Point index, int direction, int psize, double metric) const;

    // Non-const methods
    inline void setEnableIf(const std::function<FnEnableIf>& enable_if) noexcept { enable_if_ = enable_if; };

private:
    fs::path dst_dir_;
    std::function<FnEnableIf> enable_if_;
};

Inspector Inspector::fromGenericCfg(const tcfg::GenericParamConfig& generic_cfg)
{
    if (!ENABLED)
        return {};

    const fs::path dst_pattern{generic_cfg.getDstPattern()};
    fs::path dst_dir = dst_pattern.parent_path() / "inspect";
    fs::create_directories(dst_dir);

    return Inspector{std::move(dst_dir)};
}

fs::path Inspector::getMIDir(const int row, const int col) const noexcept
{
    if (!enable_if_({col, row}))
        return {};

    std::stringstream ss;
    ss << row << '-' << col;
    const fs::path mi_dir = dst_dir_ / ss.str();
    fs::create_directories(mi_dir);
    return mi_dir;
}

fs::path Inspector::getMIDir(const cv::Point index) const noexcept { return getMIDir(index.y, index.x); }

fs::path Inspector::getDirectionDir(const cv::Point index, const int direction) const noexcept
{
    if (!enable_if_(index))
        return {};

    const fs::path mi_dir = getMIDir(index.y, index.x);
    std::stringstream ss;
    ss << "d" << (int)direction;
    const fs::path direction_dir = mi_dir / ss.str();
    fs::create_directories(direction_dir);
    return direction_dir;
}

void Inspector::saveMI(const cv::Mat& img, const cv::Point index) const
{
    if (!enable_if_(index))
        return;

    const fs::path mi_dir = getMIDir(index);
    const fs::path save_path = mi_dir / "mi.png";

    cv::Mat imgu8;
    img.convertTo(imgu8, CV_8U);
    cv::imwrite(save_path.string(), imgu8);
}

void Inspector::saveAnchor(const cv::Mat& img, const cv::Point index, const int direction) const
{
    if (!enable_if_(index))
        return;

    const fs::path dir = getDirectionDir(index, direction);
    const fs::path save_path = dir / "anchor.png";

    cv::Mat imgu8;
    img.convertTo(imgu8, CV_8U);
    cv::imwrite(save_path.string(), imgu8);
}

void Inspector::saveCmpPattern(const cv::Mat& img, const cv::Point index, const int direction, const int psize,
                               const double metric) const
{
    if (!enable_if_(index))
        return;

    const fs::path dir = getDirectionDir(index, direction);
    std::stringstream ss;
    ss << "cmp" << "-p" << psize << "-m" << std::fixed << std::setprecision(3) << metric << ".png";
    const fs::path save_path = dir / ss.str();

    cv::Mat imgu8;
    img.convertTo(imgu8, CV_8U);
    cv::imwrite(save_path.string(), imgu8);
}

} // namespace tlct::_cvt
