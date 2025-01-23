#pragma once

#include <cstddef>
#include <ranges>
#include <vector>

#include <opencv2/core.hpp>

#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper/roi.hpp"
#include "tlct/helper/constexpr/math.hpp"

namespace tlct::_cvt {

namespace rgs = std::ranges;

struct MIBuffer {
    cv::Mat I, I_2;
};

template <tlct::cfg::concepts::CArrange TArrange_>
class MIBuffers_
{
public:
    // Typename alias
    using TArrange = TArrange_;

    struct Params {
        static constexpr size_t SIMD_FETCH_SIZE = 128 / 8;

        inline Params() = default;
        inline explicit Params(const TArrange& arrange) noexcept
        {
            idiameter_ = _hp::iround(arrange.getDiameter());
            aligned_mat_size_ = _hp::alignUp<SIMD_FETCH_SIZE>(idiameter_ * idiameter_ * sizeof(float));
            aligned_mi_size_ = (sizeof(MIBuffer) / sizeof(cv::Mat)) * aligned_mat_size_;
            mi_max_cols_ = arrange.getMIMaxCols();
            mi_num_ = mi_max_cols_ * arrange.getMIRows();
            buffer_size_ = mi_num_ * aligned_mi_size_;
        };
        inline Params& operator=(Params&& rhs) noexcept = default;
        inline Params(Params&& rhs) noexcept = default;

        size_t aligned_mat_size_;
        size_t aligned_mi_size_;
        size_t buffer_size_;
        int idiameter_;
        int mi_max_cols_;
        int mi_num_;
    };

    // Constructor
    inline MIBuffers_() noexcept : arrange_(), params_(), items_(), buffer_(nullptr) {};
    inline explicit MIBuffers_(const TArrange& arrange);
    MIBuffers_& operator=(const MIBuffers_& rhs) = delete;
    MIBuffers_(const MIBuffers_& rhs) = delete;
    inline MIBuffers_& operator=(MIBuffers_&& rhs) noexcept
    {
        arrange_ = std::move(rhs.arrange_);
        params_ = std::move(rhs.params_);
        items_ = std::move(rhs.items_);
        buffer_ = std::exchange(rhs.buffer_, nullptr);
        return *this;
    };
    inline MIBuffers_(MIBuffers_&& rhs) noexcept
        : arrange_(std::move(rhs.arrange_)), params_(std::move(rhs.params_)), items_(std::move(rhs.items_)),
          buffer_(std::exchange(rhs.buffer_, nullptr)) {};
    inline ~MIBuffers_() { std::free(buffer_); }

    // Initialize from
    [[nodiscard]] static inline MIBuffers_ fromArrange(const TArrange& arrange);

    // Const methods
    [[nodiscard]] inline const MIBuffer& getMI(int row, int col) const noexcept
    {
        const int offset = row * params_.mi_max_cols_ + col;
        return items_.at(offset);
    };
    [[nodiscard]] inline const MIBuffer& getMI(cv::Point index) const noexcept { return getMI(index.y, index.x); };
    [[nodiscard]] inline const MIBuffer& getMI(int offset) const noexcept { return items_.at(offset); };

    // Non-const methods
    inline MIBuffers_& update(const cv::Mat& src);

private:
    TArrange arrange_;
    Params params_;
    std::vector<MIBuffer> items_;
    void* buffer_;
};

template <tlct::cfg::concepts::CArrange TArrange>
MIBuffers_<TArrange>::MIBuffers_(const TArrange& arrange) : arrange_(arrange), params_(arrange)
{
    items_.resize(params_.mi_num_);
    buffer_ = std::malloc(params_.buffer_size_ + Params::SIMD_FETCH_SIZE);
}

template <tlct::cfg::concepts::CArrange TArrange>
MIBuffers_<TArrange> MIBuffers_<TArrange>::fromArrange(const TArrange& arrange)
{
    return MIBuffers_(arrange);
}

template <tlct::cfg::concepts::CArrange TArrange>
MIBuffers_<TArrange>& MIBuffers_<TArrange>::update(const cv::Mat& src)
{
    cv::Mat I_2_32f;
    const cv::Mat& I_32f = src;
    cv::multiply(I_32f, I_32f, I_2_32f);

    auto item_it = items_.begin();
    uint8_t* row_cursor = (uint8_t*)_hp::alignUp<Params::SIMD_FETCH_SIZE>((size_t)buffer_);
    size_t row_step = params_.mi_max_cols_ * params_.aligned_mi_size_;
    for (const int irow : rgs::views::iota(0, arrange_.getMIRows())) {

        uint8_t* col_cursor = row_cursor;
        const int mi_cols = arrange_.getMICols(irow);
        for (const int icol : rgs::views::iota(0, mi_cols)) {
            const cv::Point2f& mi_center = arrange_.getMICenter(irow, icol);
            const cv::Rect roi = getRoiByCenter(mi_center, arrange_.getDiameter());

            uint8_t* mat_cursor = col_cursor;

            const cv::Mat& I_src = I_32f(roi);
            cv::Mat I_dst = cv::Mat(params_.idiameter_, params_.idiameter_, CV_32FC1, mat_cursor);
            I_src.copyTo(I_dst);
            mat_cursor += params_.aligned_mat_size_;

            const cv::Mat& I_2_src = I_2_32f(roi);
            cv::Mat I_2_dst = cv::Mat(params_.idiameter_, params_.idiameter_, CV_32FC1, mat_cursor);
            I_2_src.copyTo(I_2_dst);
            mat_cursor += params_.aligned_mat_size_;

            *item_it = {std::move(I_dst), std::move(I_2_dst)};
            item_it++;
            col_cursor += params_.aligned_mi_size_;
        }

        if (mi_cols < params_.mi_max_cols_) {
            item_it++;
        }

        row_cursor += row_step;
    }

    return *this;
}

} // namespace tlct::_cvt
