#pragma once

#include <algorithm>
#include <array>
#include <expected>

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/common/map.hpp"
#include "tlct/helper/error.hpp"

namespace tlct::_cfg {

class OffsetArrange {
public:
    // Typename alias
    using TMiCols = std::array<int, 2>;

    // Constructor
    TLCT_API OffsetArrange() noexcept = default;
    TLCT_API OffsetArrange(const OffsetArrange& rhs) noexcept = default;
    TLCT_API OffsetArrange& operator=(const OffsetArrange& rhs) noexcept = default;
    TLCT_API OffsetArrange(OffsetArrange&& rhs) noexcept = default;
    TLCT_API OffsetArrange& operator=(OffsetArrange&& rhs) noexcept = default;
    TLCT_API OffsetArrange(cv::Size imgSize, float diameter, cv::Point2f leftTop, float xUnitShift, float yUnitShift,
                           int miRows, TMiCols miCols, int upsample, bool direction, bool isKepler, bool isMultiFocus,
                           bool isOutShift) noexcept;

    // Initialize from
    [[nodiscard]] TLCT_API static std::expected<OffsetArrange, Error> create(cv::Size imgSize, float diameter,
                                                                             bool direction, bool isKepler,
                                                                             bool isMultiFocus,
                                                                             cv::Point2f offset) noexcept;
    [[nodiscard]] TLCT_API static std::expected<OffsetArrange, Error> createWithCfgMap(const ConfigMap& map) noexcept;

    // Non-const methods
    TLCT_API OffsetArrange& upsample(int factor) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API int getImgWidth() const noexcept { return imgSize_.width; }
    [[nodiscard]] TLCT_API int getImgHeight() const noexcept { return imgSize_.height; }
    [[nodiscard]] TLCT_API cv::Size getImgSize() const noexcept { return imgSize_; }
    [[nodiscard]] TLCT_API float getDiameter() const noexcept { return diameter_; }
    [[nodiscard]] TLCT_API float getRadius() const noexcept { return diameter_ / 2.0f; }
    [[nodiscard]] TLCT_API bool getDirection() const noexcept { return direction_; }
    [[nodiscard]] TLCT_API bool isKepler() const noexcept { return isKepler_; }
    [[nodiscard]] TLCT_API bool isMultiFocus() const noexcept { return isMultiFocus_; }
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
    cv::Point2f leftTop_;
    float xUnitShift_;
    float yUnitShift_;
    int miRows_;
    TMiCols miCols_;
    int upsample_;
    bool direction_;
    bool isKepler_;
    bool isMultiFocus_;
    bool isOutShift_;
};

}  // namespace tlct::_cfg

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/config/arrange/offset.cpp"
#endif
