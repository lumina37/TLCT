#include <algorithm>
#include <array>

#include "tlct/common/defines.h"

namespace tlct::cvt::tspc {

template <typename TValue_>
class Pixel_
{
public:
    using TValue = TValue_;

    TLCT_API inline constexpr std::strong_ordering operator<=>(const Pixel_& rhs) const
    {
        if (val < rhs.val)
            return std::strong_ordering::less;
        if (val > rhs.val)
            return std::strong_ordering::greater;
        return std::strong_ordering::equal;
    }

    int x;
    int y;
    TValue_ val;
};

using Pixel = Pixel_<double>;

template <typename TValue, int size>
class PixHeap_
{
public:
    using TPixel = Pixel_<TValue>;
    using TData = std::array<TPixel, size>;
    using iterator = TData::iterator;
    using const_iterator = TData::const_iterator;

    TLCT_API inline void push(TPixel pixel)
    {
        if (size_ < size) {
            data_[size_] = pixel;
            size_++;
            if (size_ == size) {
                std::make_heap(data_.begin(), data_.end(), std::greater<TPixel>());
            }
            return;
        }

        if (pixel.val > data_[0].val) {
            // if larger than all elems in `data_`, then insert
            std::pop_heap(data_.begin(), data_.end(), std::greater<TPixel>());
            data_[size - 1] = pixel;
            std::make_heap(data_.begin(), data_.end(), std::greater<TPixel>());
        }
    }

    TLCT_API inline iterator begin() { return data_.begin(); }
    TLCT_API inline iterator end() { return data_.end(); }
    TLCT_API inline const_iterator begin() const { return data_.begin(); }
    TLCT_API inline const_iterator end() const { return data_.end(); }

private:
    TData data_;
    int size_;
};

template <int size>
using PixHeap = PixHeap_<Pixel::TValue, size>;

} // namespace tlct::cvt::tspc