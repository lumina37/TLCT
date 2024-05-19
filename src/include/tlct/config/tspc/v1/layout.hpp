#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include "calib.hpp"
#include "tlct/common/defines.h"
#include "tlct/helper/transpose.hpp"

namespace tlct::cfg::tspc::v1 {

struct TLCT_API BorderCheckList {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
};

class Layout
{
public:
    TLCT_API Layout() noexcept
        : micenters_(), imgsize_(), diameter_(), radius_(), rotation_(), upsample_(1) {};
    TLCT_API Layout& operator=(const Layout& layout);
    TLCT_API Layout(const Layout& layout)
        : micenters_(layout.micenters_.clone()), imgsize_(layout.imgsize_), diameter_(layout.diameter_),
          radius_(layout.radius_), rotation_(layout.rotation_), upsample_(layout.upsample_) {};
    TLCT_API Layout& operator=(Layout&& layout) noexcept = default;
    TLCT_API Layout(Layout&& layout) noexcept = default;
    TLCT_API Layout(cv::Mat&& micenters, cv::Size imgsize, double diameter, double rotation, int upsample)
        : micenters_(micenters), imgsize_(imgsize), diameter_(diameter), radius_(diameter / 2.0), rotation_(rotation),
          upsample_(upsample) {};

    [[nodiscard]] TLCT_API static Layout fromCfgAndImgsize(const CalibConfig& cfg, cv::Size imgsize);

    [[nodiscard]] TLCT_API Layout& upsample(int factor) noexcept;
    [[nodiscard]] TLCT_API Layout& transpose();

    [[nodiscard]] TLCT_API int getImgWidth() const noexcept;
    [[nodiscard]] TLCT_API int getImgHeight() const noexcept;
    [[nodiscard]] TLCT_API cv::Size getImgSize() const noexcept;
    [[nodiscard]] TLCT_API double getDiameter() const noexcept;
    [[nodiscard]] TLCT_API double getRadius() const noexcept;
    [[nodiscard]] TLCT_API double getRotation() const noexcept;
    [[nodiscard]] TLCT_API int getUpsample() const noexcept;
    [[nodiscard]] TLCT_API cv::Point2d getMICenter(int row, int col) const noexcept;
    [[nodiscard]] TLCT_API cv::Point2d getMICenter(cv::Point index) const noexcept;
    [[nodiscard]] TLCT_API cv::Size getMISize() const noexcept;
    [[nodiscard]] TLCT_API int getMIRows() const noexcept;
    [[nodiscard]] TLCT_API int getMICols() const noexcept;

    template <BorderCheckList checklist = {true, true, true, true}>
    [[nodiscard]] bool isMIBroken(const cv::Point2d micenter) const noexcept;

private:
    cv::Mat micenters_; // CV_64FC2
    cv::Size imgsize_;
    double diameter_{};
    double radius_{};
    double rotation_{};
    int upsample_{};
};

inline Layout& Layout::operator=(const Layout& layout)
{
    if (this != &layout) {
        micenters_ = layout.micenters_.clone();
        imgsize_ = layout.imgsize_;
        diameter_ = layout.diameter_;
        radius_ = layout.radius_;
        rotation_ = layout.rotation_;
        upsample_ = layout.upsample_;
    }
    return *this;
}

inline Layout Layout::fromCfgAndImgsize(const CalibConfig& cfg, cv::Size imgsize)
{
    Layout layout{cfg.micenters_.clone(), imgsize, cfg.diameter_, cfg.rotation_, 1};
    if (cfg.getRotation() != 0.0) {
        layout = layout.transpose();
    }
    return layout;
}

inline Layout& Layout::upsample(int factor) noexcept
{
    micenters_ *= factor;
    imgsize_ *= factor;
    diameter_ *= factor;
    radius_ *= factor;
    upsample_ = factor;
    return *this;
}

inline Layout& Layout::transpose()
{
    micenters_ = _hp::transposeCenters<double>(micenters_);
    std::swap(imgsize_.height, imgsize_.width);
    return *this;
}

inline int Layout::getImgWidth() const noexcept { return imgsize_.width; }

inline int Layout::getImgHeight() const noexcept { return imgsize_.height; }

inline cv::Size Layout::getImgSize() const noexcept { return imgsize_; }

inline double Layout::getDiameter() const noexcept { return diameter_; }

inline double Layout::getRadius() const noexcept { return radius_; }

inline double Layout::getRotation() const noexcept { return rotation_; }

inline int Layout::getUpsample() const noexcept { return upsample_; }

inline cv::Point2d Layout::getMICenter(const int row, const int col) const noexcept
{
    return micenters_.at<cv::Point2d>(row, col);
}

inline cv::Point2d Layout::getMICenter(const cv::Point index) const noexcept
{
    return micenters_.at<cv::Point2d>(index);
}

inline cv::Size Layout::getMISize() const noexcept { return micenters_.size(); }

inline int Layout::getMIRows() const noexcept { return micenters_.rows; }

inline int Layout::getMICols() const noexcept { return micenters_.cols; }

template <BorderCheckList checklist>
inline bool Layout::isMIBroken(const cv::Point2d micenter) const noexcept
{
    if (checklist.up && micenter.y < radius_) {
        return true;
    }
    if (checklist.down && micenter.y > imgsize_.height - radius_) {
        return true;
    }
    if (checklist.left && micenter.x < radius_) {
        return true;
    }
    if (checklist.right && micenter.x > imgsize_.width - radius_) {
        return true;
    }
    return false;
}

template <BorderCheckList checklist>
inline cv::Rect restrictToImgBorder(const Layout& layout, const cv::Rect area) noexcept
{
    cv::Rect modarea{area};

    if (checklist.up && area.y < 0) {
        modarea.y = 0;
    }
    if (checklist.down && area.y + area.height > layout.getImgHeight()) {
        modarea.height = layout.getImgHeight() - modarea.y;
    }
    if (checklist.left && area.x < 0) {
        modarea.x = 0;
    }
    if (checklist.right && area.x + area.width > layout.getImgWidth()) {
        modarea.width = layout.getImgWidth() - modarea.x;
    }

    return modarea;
}

template <BorderCheckList checklist>
inline std::vector<cv::Range> restrictToImgBorder(const Layout& layout, const std::vector<cv::Range>& ranges) noexcept
{
    std::vector<cv::Range> modranges{ranges};

    if (checklist.up && ranges[0].start < 0) {
        modranges[0].start = 0;
    }
    if (checklist.down && ranges[0].end > layout.getImgHeight()) {
        modranges[0].end = layout.getImgHeight();
    }
    if (checklist.left && ranges[1].start < 0) {
        modranges[1].start = 0;
    }
    if (checklist.right && ranges[1].end > layout.getImgWidth()) {
        modranges[1].end = layout.getImgWidth();
    }

    return modranges;
}

TLCT_API inline void procImg_(const Layout& layout, const cv::Mat& src, cv::Mat& dst)
{
    const double rotation = layout.getRotation();
    if (rotation != 0.0) {
        cv::Mat transposed_src;
        cv::transpose(src, transposed_src);
        dst = std::move(transposed_src);
    }

    const int upsample = layout.getUpsample();
    if (upsample != 1) {
        cv::Mat upsampled_src;
        cv::resize(dst, upsampled_src, {}, upsample, upsample, cv::INTER_CUBIC);
        dst = std::move(upsampled_src);
    }
}

TLCT_API inline cv::Mat procImg(const Layout& layout, const cv::Mat& src)
{
    cv::Mat dst;
    procImg_(layout, src, dst);
    return dst;
}

} // namespace tlct::cfg::inline tspc