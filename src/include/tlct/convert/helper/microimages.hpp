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
    static constexpr size_t CACHELINE_SIZE = 0x40;

    // Constructor
    TLCT_API inline MIs() noexcept : layout_(), mi_max_cols_(), items_(), buffer_(){};
    TLCT_API inline explicit MIs(const TLayout& layout) noexcept;
    TLCT_API inline MIs& operator=(const MIs& rhs) noexcept = delete;
    TLCT_API inline MIs(const MIs& rhs) noexcept = delete;
    TLCT_API inline MIs& operator=(MIs&& rhs) noexcept
    {
        layout_ = std::move(rhs.layout_);
        mi_max_cols_ = std::exchange(rhs.mi_max_cols_, 0);
        items_ = std::move(rhs.items_);
        buffer_ = std::exchange(rhs.buffer_, nullptr);
        return *this;
    };
    TLCT_API inline MIs(MIs&& rhs) noexcept
        : layout_(std::move(rhs.layout_)), mi_max_cols_(std::exchange(rhs.mi_max_cols_, 0)),
          items_(std::move(rhs.items_)), buffer_(std::exchange(rhs.buffer_, nullptr)){};
    TLCT_API inline ~MIs() { std::free(buffer_); }

    // Initialize from
    [[nodiscard]] TLCT_API static inline MIs fromLayout(const TLayout& layout);

    // Const methods
    [[nodiscard]] TLCT_API inline const cv::Mat& getMI(int row, int col) const noexcept
    {
        const int offset = row * layout_.getMIMaxCols() + col;
        return items_.at(offset);
    };
    [[nodiscard]] TLCT_API inline const cv::Mat& getMI(cv::Point index) const noexcept
    {
        return getMI(index.y, index.x);
    };

    // Non-const methods
    TLCT_API inline MIs& update(const cv::Mat& src) noexcept;

private:
    TLayout layout_;
    int mi_max_cols_;
    std::vector<cv::Mat> items_;
    void* buffer_;
};

template <typename TLayout>
    requires tlct::cfg::concepts::CLayout<TLayout>
MIs<TLayout>::MIs(const TLayout& layout) noexcept : layout_(layout), mi_max_cols_(layout.getMIMaxCols())
{
    const int diameter = _hp::iround(layout.getDiameter());
    const int mi_size = diameter * diameter;
    const size_t aligned_mi_size = _hp::align_to<CACHELINE_SIZE>(mi_size);
    const int mi_max_cols = layout.getMIMaxCols();
    const int mi_num = mi_max_cols * layout.getMIRows();
    const size_t buffer_size = mi_num * aligned_mi_size;

    items_.resize(mi_num);
    buffer_ = std::malloc(buffer_size + CACHELINE_SIZE);
}

template <typename TLayout>
    requires tlct::cfg::concepts::CLayout<TLayout>
MIs<TLayout> MIs<TLayout>::fromLayout(const TLayout& layout)
{
    return MIs(layout);
}

template <typename TLayout>
    requires tlct::cfg::concepts::CLayout<TLayout>
MIs<TLayout>& MIs<TLayout>::update(const cv::Mat& src) noexcept
{
    cv::Mat gray_src;
    cv::cvtColor(src, gray_src, cv::COLOR_BGR2GRAY);

    const int diameter = _hp::iround(layout_.getDiameter());
    const int mi_size = diameter * diameter;
    const size_t aligned_mi_size = _hp::align_to<CACHELINE_SIZE>(mi_size);
    const int mi_num = mi_max_cols_ * layout_.getMIRows();
    const size_t buffer_size = mi_num * aligned_mi_size;

    auto item_it = items_.begin();
    auto* row_cursor = (uint8_t*)_hp::align_to<CACHELINE_SIZE>((size_t)buffer_);
    size_t row_step = mi_max_cols_ * aligned_mi_size;
    for (const int irow : rgs::views::iota(0, layout_.getMIRows())) {

        uint8_t* col_cursor = row_cursor;
        const int mi_cols = layout_.getMICols(irow);
        for (const int icol : rgs::views::iota(0, mi_cols)) {
            const auto mi_center = layout_.getMICenter(irow, icol);
            const cv::Mat& mi_src = getRoiImageByCenter(gray_src, mi_center, layout_.getDiameter());
            cv::Mat mi_dst = cv::Mat(diameter, diameter, CV_8U, col_cursor);
            mi_src.copyTo(mi_dst);
            *item_it = std::move(mi_dst);

            item_it++;
            col_cursor += aligned_mi_size;
        }

        if (mi_cols < mi_max_cols_) {
            item_it++;
        }

        row_cursor += row_step;
    }

    return *this;
}

} // namespace tlct::_cvt
