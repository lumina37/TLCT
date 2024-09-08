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
    static constexpr size_t CACHELINE_SIZE = 64;

    // Constructor
    TLCT_API inline MIs() noexcept : mis_(), buffer_(), mi_max_cols_(){};
    TLCT_API inline MIs(std::vector<cv::Mat>&& mis, void* buffer, int mi_max_cols) noexcept
        : mis_(std::move(mis)), buffer_(buffer), mi_max_cols_(mi_max_cols){};
    TLCT_API inline MIs& operator=(const MIs& rhs) noexcept = delete;
    TLCT_API inline MIs(const MIs& rhs) noexcept = delete;
    TLCT_API inline MIs& operator=(MIs&& rhs) noexcept
    {
        mis_ = std::move(rhs.mis_);
        buffer_ = std::exchange(rhs.buffer_, nullptr);
        mi_max_cols_ = std::exchange(rhs.mi_max_cols_, 0);
        return *this;
    };
    TLCT_API inline MIs(MIs&& rhs) noexcept
        : mis_(std::move(rhs.mis_)), buffer_(std::exchange(rhs.buffer_, nullptr)),
          mi_max_cols_(std::exchange(rhs.mi_max_cols_, 0)){};
    TLCT_API inline ~MIs() { std::free(buffer_); }

    // Initialize from
    [[nodiscard]] TLCT_API static inline MIs fromLayoutAndImg(const TLayout& layout, const cv::Mat& src);

    // Const methods
    [[nodiscard]] TLCT_API inline const cv::Mat& getMI(int row, int col) const noexcept
    {
        const int offset = row * mi_max_cols_ + col;
        return mis_.at(offset);
    };
    [[nodiscard]] TLCT_API inline const cv::Mat& getMI(cv::Point index) const noexcept
    {
        return getMI(index.y, index.x);
    };

private:
    std::vector<cv::Mat> mis_;
    void* buffer_;
    int mi_max_cols_;
};

template <typename TLayout>
    requires tlct::cfg::concepts::CLayout<TLayout>
MIs<TLayout> MIs<TLayout>::fromLayoutAndImg(const TLayout& layout, const cv::Mat& src)
{
    cv::Mat gray_src;
    cv::cvtColor(src, gray_src, cv::COLOR_BGR2GRAY);

    const int diameter = _hp::iround(layout.getDiameter());
    const int mi_size = diameter * diameter;
    const size_t aligned_mi_size = _hp::align_to<CACHELINE_SIZE>(mi_size);
    const int mi_max_cols = layout.getMIMaxCols();
    const int mi_num = mi_max_cols * layout.getMIRows();
    const size_t buffer_size = mi_num * aligned_mi_size;

    std::vector<cv::Mat> mis;
    mis.reserve(mi_num);
    void* buffer = std::malloc(buffer_size + CACHELINE_SIZE);

    auto* row_cursor = (uint8_t*)_hp::align_to<CACHELINE_SIZE>((size_t)buffer);
    size_t row_step = mi_max_cols * aligned_mi_size;
    for (const int irow : rgs::views::iota(0, layout.getMIRows())) {

        uint8_t* col_cursor = row_cursor;
        const int mi_cols = layout.getMICols(irow);
        for (const int icol : rgs::views::iota(0, mi_cols)) {
            const auto mi_center = layout.getMICenter(irow, icol);
            const cv::Mat& mi_src = getRoiImageByCenter(gray_src, mi_center, layout.getDiameter());
            auto& mi_dst = mis.emplace_back(diameter, diameter, CV_8U, col_cursor);
            mi_src.copyTo(mi_dst);
            col_cursor += aligned_mi_size;
        }

        if (mi_cols < mi_max_cols) {
            mis.emplace_back();
        }

        row_cursor += row_step;
    }

    return {std::move(mis), buffer, layout.getMIMaxCols()};
}

} // namespace tlct::_cvt
