#include <cmath>

#include <opencv2/core.hpp>

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/helper/roi.hpp"
#endif

namespace tlct::_cvt {

[[nodiscard]] cv::Rect getRoiByCenter(const cv::Point2f& center, const float width) noexcept {
    const int startX = (int)std::roundf(center.x - width / 2.f);
    const int startY = (int)std::roundf(center.y - width / 2.f);
    const int iWidth = (int)std::roundf(width);
    return {startX, startY, iWidth, iWidth};
}

[[nodiscard]] cv::Mat getRoiImageByCenter(const cv::Mat& src, const cv::Point2f& center,
                                                        const float width) noexcept {
    cv::Mat roi = src(getRoiByCenter(center, width));
    return roi;
}

[[nodiscard]] cv::Rect getRoiByCenter(const cv::Point2f& center, const cv::Size2f size) noexcept {
    const int startX = (int)std::roundf(center.x - size.width / 2.f);
    const int startY = (int)std::roundf(center.y - size.height / 2.f);
    const int iWidth = (int)std::roundf(size.width);
    const int iHeight = (int)std::roundf(size.width);
    return {startX, startY, iWidth, iHeight};
}

[[nodiscard]] cv::Mat getRoiImageByCenter(const cv::Mat& src, const cv::Point2f& center,
                                                        const cv::Size2f size) noexcept {
    cv::Mat roi = src(getRoiByCenter(center, size));
    return roi;
}

[[nodiscard]] cv::Rect getRoiByCenter(const cv::Point& center, const int width) noexcept {
    const int startX = center.x - (width + 1) / 2;
    const int startY = center.y - (width + 1) / 2;
    return {startX, startY, width, width};
}

[[nodiscard]] cv::Mat getRoiImageByCenter(const cv::Mat& src, const cv::Point& center,
                                                        const int width) noexcept {
    cv::Mat roi = src(getRoiByCenter(center, width));
    return roi;
}

[[nodiscard]] cv::Rect getRoiByCenter(const cv::Point& center, const cv::Size size) noexcept {
    const int startX = center.x - (size.width + 1) / 2;
    const int startY = center.y - (size.height + 1) / 2;
    return {startX, startY, size.width, size.height};
}

[[nodiscard]] cv::Mat getRoiImageByCenter(const cv::Mat& src, const cv::Point& center,
                                                        const cv::Size size) noexcept {
    cv::Mat roi = src(getRoiByCenter(center, size));
    return roi;
}

[[nodiscard]] cv::Mat getRoiImageByLeftupCorner(const cv::Mat& src, const cv::Point& corner,
                                                              const float width) noexcept {
    const int iWidth = (int)std::roundf(width);
    cv::Mat roi = src({corner.x, corner.y, iWidth, iWidth});
    return roi;
}

[[nodiscard]] cv::Mat getRoiImageByLeftupCorner(const cv::Mat& src, const cv::Point& corner,
                                                              const int width) noexcept {
    cv::Mat roi = src({corner.x, corner.y, width, width});
    return roi;
}

}  // namespace tlct::_cvt
