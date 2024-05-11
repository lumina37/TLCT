#pragma once

#include <opencv2/core.hpp>

#include "calib.hpp"

namespace tlct::cfg::inline tspc {

struct BorderCheckList {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
};

class TLCT_API Layout
{
public:
    Layout(const cv::Mat& micenters, cv::Size imgsize, double diameter, double rotation)
        : micenters_(micenters.clone()), imgsize_(imgsize), diameter_(diameter), radius_(diameter / 2.0),
          rotation_(rotation){};

    static Layout fromConfigAndImgsize(const CalibConfig& config, cv::Size imgsize);

    [[nodiscard]] Layout upsample(int factor) noexcept;

    [[nodiscard]] int getImgWidth() const noexcept;
    [[nodiscard]] int getImgHeight() const noexcept;
    [[nodiscard]] double getDiameter() const noexcept;
    [[nodiscard]] double getRadius() const noexcept;
    [[nodiscard]] double getRotation() const noexcept;
    [[nodiscard]] cv::Point2d getMICenter(int row, int col) const noexcept;
    [[nodiscard]] cv::Point2d getMICenter(cv::Point index) const noexcept;
    [[nodiscard]] cv::Size getMISize() const noexcept;
    [[nodiscard]] int getMIRows() const noexcept;
    [[nodiscard]] int getMICols() const noexcept;

    template <BorderCheckList checklist = {true, true, true, true}>
    [[nodiscard]] bool isMIBroken(const cv::Point2d micenter) const noexcept;

    template <BorderCheckList checklist = {true, true, true, true}>
    [[nodiscard]] cv::Rect restrictToImgBorder(const cv::Rect area) const noexcept;

    template <BorderCheckList checklist = {true, true, true, true}>
    [[nodiscard]] std::vector<cv::Range> restrictToImgBorder(const std::vector<cv::Range>& ranges) const noexcept;

private:
    cv::Mat micenters_; // CV_64FC2
    cv::Size imgsize_;
    double diameter_;
    double radius_;
    double rotation_;
};

inline Layout Layout::fromConfigAndImgsize(const CalibConfig& config, cv::Size imgsize)
{
    return {config.micenters_, imgsize, config.getDiameter(), config.getRotation()};
}

inline Layout Layout::upsample(int factor) noexcept
{
    micenters_ *= factor;
    imgsize_ *= factor;
    diameter_ *= factor;
    radius_ *= factor;
    return *this;
}

inline int Layout::getImgWidth() const noexcept { return imgsize_.width; }

inline int Layout::getImgHeight() const noexcept { return imgsize_.height; }

inline double Layout::getDiameter() const noexcept { return diameter_; }

inline double Layout::getRadius() const noexcept { return radius_; }

inline double Layout::getRotation() const noexcept { return rotation_; }

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
inline cv::Rect Layout::restrictToImgBorder(const cv::Rect area) const noexcept
{
    cv::Rect modarea{area};

    if (checklist.up && area.y < 0) {
        modarea.y = 0;
    }
    if (checklist.down && area.y + area.height > imgsize_.height) {
        modarea.height = imgsize_.height - modarea.y;
    }
    if (checklist.left && area.x < 0) {
        modarea.x = 0;
    }
    if (checklist.right && area.x + area.width > imgsize_.width) {
        modarea.width = imgsize_.width - modarea.x;
    }

    return modarea;
}

template <BorderCheckList checklist>
inline std::vector<cv::Range> Layout::restrictToImgBorder(const std::vector<cv::Range>& ranges) const noexcept
{
    std::vector<cv::Range> modranges{ranges};

    if (checklist.up && ranges[0].start < 0) {
        modranges[0].start = 0;
    }
    if (checklist.down && ranges[0].end > imgsize_.height) {
        modranges[0].end = imgsize_.height;
    }
    if (checklist.left && ranges[1].start < 0) {
        modranges[1].start = 0;
    }
    if (checklist.right && ranges[1].end > imgsize_.width) {
        modranges[1].end = imgsize_.width;
    }

    return modranges;
}

} // namespace tlct::cfg::inline tspc