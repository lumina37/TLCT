#pragma once

#include <algorithm>
#include <array>
#include <ranges>

#include "tlct/common/defines.h"

namespace tlct::cvt::tspc {

namespace rgs = std::ranges;

template <typename TValue_>
class Pixel_
{
public:
    using TValue = TValue_;

    TLCT_API inline constexpr std::strong_ordering operator<=>(const Pixel_& rhs) const
    {
        if (response < rhs.response)
            return std::strong_ordering::less;
        if (response > rhs.response)
            return std::strong_ordering::greater;
        return std::strong_ordering::equal;
    }

    cv::Point pt;
    TValue_ response;
};

using Pixel = Pixel_<double>;

template <typename TValue, int dsize>
class PixHeap_
{
public:
    using TPixel = Pixel_<TValue>;
    using TData = std::array<TPixel, dsize>;
    using iterator = TData::iterator;
    using const_iterator = TData::const_iterator;

    TLCT_API inline void push(const TPixel pixel)
    {
        if (size_ < dsize) {
            // Direct insert if `data_` is not full yet.
            data_[size_] = pixel;
            size_++;
            if (size_ == dsize) {
                // Immediately heapify after `data_` is full.
                std::make_heap(data_.begin(), data_.end(), std::greater<TPixel>());
            }
            return;
        }

        if (pixel.response > data_[0].response) {
            // Insert if the input is larger than all elems in `data_`.
            std::pop_heap(data_.begin(), data_.end(), std::greater<TPixel>());
            data_[dsize - 1] = pixel;
            std::make_heap(data_.begin(), data_.end(), std::greater<TPixel>());
        }
    }

    [[nodiscard]] TLCT_API inline int size() const { return size_; }
    [[nodiscard]] TLCT_API inline bool empty() const { return size() == 0; }

    [[nodiscard]] TLCT_API inline iterator begin() { return data_.begin(); }
    [[nodiscard]] TLCT_API inline iterator end() { return data_.end(); }
    [[nodiscard]] TLCT_API inline const_iterator begin() const { return data_.begin(); }
    [[nodiscard]] TLCT_API inline const_iterator end() const { return data_.end(); }

private:
    TData data_;
    int size_;
};

using PixHeap = PixHeap_<Pixel::TValue, 2>;

TLCT_API inline PixHeap statTopKFeaturePoints(const cv::Mat src, const float accept_threshold = 64.0)
{
    assert(src.type() == CV_32F);

    PixHeap heap{};
    for (const int irow : rgs::views::iota(0, src.rows)) {
        const auto prow = src.ptr<float>(irow);
        for (const int icol : rgs::views::iota(0, src.cols)) {
            const float response = prow[icol];
            if (response > accept_threshold) {
                heap.push({{icol, irow}, response});
            }
        }
    }

    return heap;
}

} // namespace tlct::cvt::tspc