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

struct WrapMI {
    cv::Mat I, I_2;
};

template <tlct::cfg::concepts::CLayout TLayout_>
class MIs_
{
public:
    // Typename alias
    using TLayout = TLayout_;

    struct Params {
        static constexpr size_t SIMD_FETCH_SIZE = 128 / 8;

        inline Params() = default;
        inline explicit Params(const TLayout& layout) noexcept
        {
            idiameter_ = _hp::iround(layout.getDiameter());
            aligned_mat_size_ = _hp::alignUp<SIMD_FETCH_SIZE>(idiameter_ * idiameter_ * sizeof(float));
            aligned_mi_size_ = (sizeof(WrapMI) / sizeof(cv::Mat)) * aligned_mat_size_;
            mi_max_cols_ = layout.getMIMaxCols();
            mi_num_ = mi_max_cols_ * layout.getMIRows();
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
    inline MIs_() noexcept : layout_(), params_(), items_(), buffer_(nullptr) {};
    inline explicit MIs_(const TLayout& layout);
    MIs_& operator=(const MIs_& rhs) = delete;
    MIs_(const MIs_& rhs) = delete;
    inline MIs_& operator=(MIs_&& rhs) noexcept
    {
        layout_ = std::move(rhs.layout_);
        params_ = std::move(rhs.params_);
        items_ = std::move(rhs.items_);
        buffer_ = std::exchange(rhs.buffer_, nullptr);
        return *this;
    };
    inline MIs_(MIs_&& rhs) noexcept
        : layout_(std::move(rhs.layout_)), params_(std::move(rhs.params_)), items_(std::move(rhs.items_)),
          buffer_(std::exchange(rhs.buffer_, nullptr)) {};
    inline ~MIs_() { std::free(buffer_); }

    // Initialize from
    [[nodiscard]] static inline MIs_ fromLayout(const TLayout& layout);

    // Const methods
    [[nodiscard]] inline const WrapMI& getMI(int row, int col) const noexcept
    {
        const int offset = row * params_.mi_max_cols_ + col;
        return items_.at(offset);
    };
    [[nodiscard]] inline const WrapMI& getMI(cv::Point index) const noexcept { return getMI(index.y, index.x); };
    [[nodiscard]] inline const WrapMI& getMI(int offset) const noexcept { return items_.at(offset); };

    // Non-const methods
    inline MIs_& update(const cv::Mat& src);

private:
    TLayout layout_;
    Params params_;
    std::vector<WrapMI> items_;
    void* buffer_;
};

template <tlct::cfg::concepts::CLayout TLayout>
MIs_<TLayout>::MIs_(const TLayout& layout) : layout_(layout), params_(layout)
{
    items_.resize(params_.mi_num_);
    buffer_ = std::malloc(params_.buffer_size_ + Params::SIMD_FETCH_SIZE);
}

template <tlct::cfg::concepts::CLayout TLayout>
MIs_<TLayout> MIs_<TLayout>::fromLayout(const TLayout& layout)
{
    return MIs_(layout);
}

template <tlct::cfg::concepts::CLayout TLayout>
MIs_<TLayout>& MIs_<TLayout>::update(const cv::Mat& src)
{
    cv::Mat I_32f, I_2_32f;
    src.convertTo(I_32f, CV_32S);
    cv::multiply(I_32f, I_32f, I_2_32f);
    I_32f.convertTo(I_32f, CV_32F);
    I_2_32f.convertTo(I_2_32f, CV_32F);

    auto item_it = items_.begin();
    auto* row_cursor = (uint8_t*)_hp::alignUp<Params::SIMD_FETCH_SIZE>((size_t)buffer_);
    size_t row_step = params_.mi_max_cols_ * params_.aligned_mi_size_;
    for (const int irow : rgs::views::iota(0, layout_.getMIRows())) {

        uint8_t* col_cursor = row_cursor;
        const int mi_cols = layout_.getMICols(irow);
        for (const int icol : rgs::views::iota(0, mi_cols)) {
            const auto mi_center = layout_.getMICenter(irow, icol);
            const cv::Rect roi = getRoiByCenter(mi_center, layout_.getDiameter());

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
