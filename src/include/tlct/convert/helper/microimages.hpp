#pragma once

#include <ranges>
#include <vector>

#include <opencv2/core.hpp>

#include "roi.hpp"
#include "tlct/common/defines.h"
#include "tlct/config/concepts.hpp"
#include "tlct/helper/static_math.hpp"

namespace tlct::_cvt {

namespace rgs = std::ranges;

template <typename TLayout>
    requires tlct::cfg::concepts::CLayout<TLayout>
class MIs
{
public:
    // Constructor
    TLCT_API inline MIs(std::vector<cv::Mat>&& mis, uint8_t* buffer, int mirows)
        : mis_(mis), buffer_(buffer), mirows_(mirows){};
    TLCT_API inline ~MIs() { delete[] buffer_; }

    // Initialize from
    [[nodiscard]] TLCT_API static inline MIs fromLayoutAndImg(const TLayout& layout, const cv::Mat& img);

    // Const methods
    [[nodiscard]] TLCT_API inline cv::Mat& getMI(int row, int col) noexcept
    {
        const int offset = row * mirows_ + col;
        return mis_[offset];
    };
    [[nodiscard]] TLCT_API inline cv::Mat& getMI(cv::Point index) noexcept { return getMI(index.y, index.x); };

private:
    std::vector<cv::Mat> mis_;
    uint8_t* buffer_;
    int mirows_;
};

template <typename TLayout>
    requires tlct::cfg::concepts::CLayout<TLayout>
MIs<TLayout> MIs<TLayout>::fromLayoutAndImg(const TLayout& layout, const cv::Mat& img)
{
    const int diameter = _hp::iround(layout.getDiameter());
    const int mi_size = diameter * diameter;
    constexpr size_t cacheline_size = 64;
    const size_t aligned_mi_size = _hp::align_to<cacheline_size>(mi_size);
    const int mi_num = layout.getMIMaxCols() * layout.getMIRows();
    const size_t buffer_size = mi_num * aligned_mi_size;
    auto* buffer = new ((std::align_val_t)cacheline_size) uint8_t[buffer_size];

    std::vector<cv::Mat> mis;
    mis.reserve(mi_num);

    uint8_t* row_cursor = buffer;
    size_t row_step = layout.getMIMaxCols() * aligned_mi_size;
    for (const int irow : rgs::views::iota(0, layout.getMIRows())) {
        uint8_t* col_cursor = row_cursor;
        for (const int icol : rgs::views::iota(0, layout.getMICols(irow))) {
            const auto mi_center = layout.getMICenter(irow, icol);
            const cv::Mat& mi_src = getRoiImageByCenter(img, mi_center, layout.getDiameter());
            auto& mi_dst = mis.emplace_back(diameter, diameter, CV_8U, col_cursor);
            mi_src.copyTo(mi_dst);
            mi_dst.addref(); // prevent double-free
            col_cursor += aligned_mi_size;
        }
        row_cursor += row_step;
    }

    return {std::move(mis), buffer};
}

} // namespace tlct::_cvt