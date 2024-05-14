#pragma once

#include <cstdio>
#include <filesystem>
#include <string>

#include <opencv2/core.hpp>

#include "calib.hpp"
#include "tlct/common/defines.h"
#include "tlct/config/common.hpp"

namespace tlct::cfg::inline tspc {

namespace fs = std::filesystem;

class ParamConfig
{
public:
    TLCT_API ParamConfig() : calib_cfg_(), views_(0), imgsize_(), range_(), src_pattern_(), dst_pattern_() {};
    TLCT_API ParamConfig(const ParamConfig& config) = default;
    TLCT_API ParamConfig(ParamConfig&& config) = default;
    TLCT_API ParamConfig(CalibConfig&& calib_cfg, int views, cv::Size imgsize, cv::Range range, std::string src_pattern,
                         std::string dst_pattern)
        : calib_cfg_(calib_cfg), views_(views), imgsize_(imgsize), range_(range), src_pattern_(std::move(src_pattern)),
          dst_pattern_(std::move(dst_pattern)) {};

    [[nodiscard]] TLCT_API static ParamConfig fromCommonCfg(const CommonParamConfig& config);

    [[nodiscard]] TLCT_API const CalibConfig& getCalibCfg() const noexcept;
    [[nodiscard]] TLCT_API int getViews() const noexcept;
    [[nodiscard]] TLCT_API cv::Size getImgSize() const noexcept;
    [[nodiscard]] TLCT_API cv::Range getRange() const noexcept;
    [[nodiscard]] TLCT_API const std::string& getSrcPattern() const noexcept;
    [[nodiscard]] TLCT_API fs::path getSrcPath(int i) const noexcept;
    [[nodiscard]] TLCT_API const std::string& getDstPattern() const noexcept;
    [[nodiscard]] TLCT_API fs::path getDstPath(int i) const noexcept;

private:
    CalibConfig calib_cfg_;
    int views_;
    cv::Size imgsize_;
    cv::Range range_;
    std::string src_pattern_;
    std::string dst_pattern_;
};

inline ParamConfig ParamConfig::fromCommonCfg(const CommonParamConfig& config)
{
    const auto& cfg_map = config.getConfigMap();
    auto calib_cfg = CalibConfig::fromXMLPath(cfg_map.at("Calibration_xml").c_str());
    const int views = std::stoi(cfg_map.at("viewNum"));
    const int width = std::stoi(cfg_map.at("width"));
    const int height = std::stoi(cfg_map.at("height"));
    const int start = std::stoi(cfg_map.at("start_frame"));
    const int end = std::stoi(cfg_map.at("end_frame"));
    const std::string& src_pattern = cfg_map.at("RawImage_Path");
    const std::string& dst_pattern = cfg_map.at("Output_Path");
    return {std::move(calib_cfg), views, {width, height}, {start, end}, src_pattern, dst_pattern};
}

inline const CalibConfig& ParamConfig::getCalibCfg() const noexcept { return calib_cfg_; }

inline int ParamConfig::getViews() const noexcept { return views_; }

inline cv::Size ParamConfig::getImgSize() const noexcept { return imgsize_; }

inline cv::Range ParamConfig::getRange() const noexcept { return range_; }

inline const std::string& ParamConfig::getSrcPattern() const noexcept { return src_pattern_; }

inline fs::path ParamConfig::getSrcPath(int i) const noexcept
{
    char buffer[256];
    sprintf(buffer, src_pattern_.c_str(), i);
    return {buffer};
}

inline const std::string& ParamConfig::getDstPattern() const noexcept { return dst_pattern_; }

inline fs::path ParamConfig::getDstPath(int i) const noexcept
{
    char buffer[256];
    sprintf(buffer, dst_pattern_.c_str(), i);
    return {buffer};
}

} // namespace tlct::cfg::inline tspc
