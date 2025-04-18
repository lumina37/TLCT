#pragma once

#include <algorithm>
#include <array>
#include <expected>

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"
#include "tlct/common/error.hpp"
#include "tlct/config/common/map.hpp"

namespace tlct::_cfg {

class CornersArrange {
public:
    // Typename alias
    using ErrTp = Error;
    using TMiCols = std::array<int, 2>;

    // Constructor
    TLCT_API CornersArrange() noexcept
        : imgSize_(),
          diameter_(),
          radius_(),
          direction_(),
          leftTop_(),
          rightTop_(),
          leftYUnitShift_(),
          rightYUnitShift_(),
          miRows_(),
          miCols_(),
          upsample_(1),
          isOutShift_() {}
    TLCT_API CornersArrange(const CornersArrange& rhs) noexcept = default;
    TLCT_API CornersArrange& operator=(const CornersArrange& rhs) noexcept = default;
    TLCT_API CornersArrange(CornersArrange&& rhs) noexcept = default;
    TLCT_API CornersArrange& operator=(CornersArrange&& rhs) noexcept = default;
    TLCT_API CornersArrange(cv::Size imgSize, float diameter, bool direction, cv::Point2f leftTop, cv::Point2f rightTop,
                            cv::Point2f leftBottom, cv::Point2f rightBottom) noexcept;

    // Initialize from
    [[nodiscard]] TLCT_API static std::expected<CornersArrange, Error> create(cv::Size imgSize, float diameter,
                                                                              bool direction, cv::Point2f leftTop,
                                                                              cv::Point2f rightTop,
                                                                              cv::Point2f leftBottom,
                                                                              cv::Point2f rightBottom) noexcept;
    [[nodiscard]] TLCT_API static std::expected<CornersArrange, Error> createWithCfgMap(const ConfigMap& map) noexcept;

    // Non-const methods
    TLCT_API CornersArrange& upsample(int factor) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API int getImgWidth() const noexcept { return imgSize_.width; }
    [[nodiscard]] TLCT_API int getImgHeight() const noexcept { return imgSize_.height; }
    [[nodiscard]] TLCT_API cv::Size getImgSize() const noexcept { return imgSize_; }
    [[nodiscard]] TLCT_API float getDiameter() const noexcept { return diameter_; }
    [[nodiscard]] TLCT_API float getRadius() const noexcept { return radius_; }
    [[nodiscard]] TLCT_API bool getDirection() const noexcept { return direction_; }
    [[nodiscard]] TLCT_API int getUpsample() const noexcept { return upsample_; }
    [[nodiscard]] TLCT_API int getMIRows() const noexcept { return miRows_; }
    [[nodiscard]] TLCT_API int getMICols(const int row) const noexcept { return miCols_[row % miCols_.size()]; }
    [[nodiscard]] TLCT_API int getMIMaxCols() const noexcept { return std::max(miCols_[0], miCols_[1]); }
    [[nodiscard]] TLCT_API int getMIMinCols() const noexcept { return std::min(miCols_[0], miCols_[1]); }
    [[nodiscard]] TLCT_API cv::Point2f getMICenter(int row, int col) const noexcept;
    [[nodiscard]] TLCT_API cv::Point2f getMICenter(cv::Point index) const noexcept;
    [[nodiscard]] TLCT_API bool isOutShift() const noexcept { return isOutShift_; }

private:
    cv::Size imgSize_;
    float diameter_;
    float radius_;
    bool direction_;
    cv::Point2f leftTop_;
    cv::Point2f rightTop_;
    cv::Point2f leftYUnitShift_;
    cv::Point2f rightYUnitShift_;
    int miRows_;
    TMiCols miCols_;
    int upsample_;
    bool isOutShift_;
};

}  // namespace tlct::_cfg

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/config/arrange/corners.cpp"
#endif
