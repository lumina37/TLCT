#pragma once

#include <ranges>
#include <vector>

#include <opencv2/core.hpp>

#include "roi.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/helper/static_math.hpp"

namespace tlct::_cvt {

namespace rgs = std::ranges;

class WrapMI
{
public:
    static constexpr int CACHED_MAT_NUM = 2;

    inline WrapMI() = default;
    inline WrapMI(cv::Mat&& I, cv::Mat&& I_2) noexcept : I_(std::move(I)), I_2_(std::move(I_2)) {};
    inline WrapMI& operator=(const WrapMI& rhs) = default;
    inline WrapMI(const WrapMI& rhs) = default;
    inline WrapMI& operator=(WrapMI&& rhs) noexcept = default;
    inline WrapMI(WrapMI&& rhs) noexcept = default;

    cv::Mat I_, I_2_;
};

template <typename TLayout>
    requires tlct::cfg::concepts::CLayout<TLayout>
class MIs
{
public:
    class Params
    {
    public:
        static constexpr int SIMD_FETCH_SIZE = 256 / 8;

        inline Params() noexcept = default;
        inline explicit Params(const TLayout& layout) noexcept
        {
            idiameter_ = _hp::iround(layout.getDiameter());
            const int row_step = _hp::align_to<SIMD_FETCH_SIZE>(idiameter_ * sizeof(float));
            aligned_mat_size_ = row_step * idiameter_;
            aligned_mi_size_ = WrapMI::CACHED_MAT_NUM * aligned_mat_size_;
            mi_max_cols_ = layout.getMIMaxCols();
            mi_num_ = mi_max_cols_ * layout.getMIRows();
            buffer_size_ = mi_num_ * aligned_mi_size_;
        };
        inline Params& operator=(Params&& rhs) noexcept = default;
        inline Params(Params&& rhs) noexcept = default;

        int idiameter_;
        int aligned_mat_size_;
        int aligned_mi_size_;
        int mi_max_cols_;
        int mi_num_;
        int buffer_size_;
    };

    // Constructor
    inline MIs() noexcept : layout_(), params_(), items_(), buffer_(nullptr) {};
    inline explicit MIs(const TLayout& layout);
    MIs& operator=(const MIs& rhs) = delete;
    MIs(const MIs& rhs) = delete;
    inline MIs& operator=(MIs&& rhs) noexcept
    {
        layout_ = std::move(rhs.layout_);
        params_ = std::move(rhs.params_);
        items_ = std::move(rhs.items_);
        buffer_ = std::exchange(rhs.buffer_, nullptr);
        return *this;
    };
    inline MIs(MIs&& rhs) noexcept
        : layout_(std::move(rhs.layout_)), params_(std::move(rhs.params_)), items_(std::move(rhs.items_)),
          buffer_(std::exchange(rhs.buffer_, nullptr)) {};
    inline ~MIs() { std::free(buffer_); }

    // Initialize from
    [[nodiscard]] static inline MIs fromLayout(const TLayout& layout);

    // Const methods
    [[nodiscard]] inline const WrapMI& getMI(int row, int col) const noexcept
    {
        const int offset = row * params_.mi_max_cols_ + col;
        return items_.at(offset);
    };
    [[nodiscard]] inline const WrapMI& getMI(cv::Point index) const noexcept { return getMI(index.y, index.x); };

    // Non-const methods
    inline MIs& update(const cv::Mat& src);

private:
    TLayout layout_;
    Params params_;
    std::vector<WrapMI> items_;
    void* buffer_;
};

template <typename TLayout>
    requires tlct::cfg::concepts::CLayout<TLayout>
MIs<TLayout>::MIs(const TLayout& layout) : layout_(layout), params_(layout)
{
    items_.resize(params_.mi_num_);
    buffer_ = std::malloc(params_.buffer_size_ + Params::SIMD_FETCH_SIZE);
}

template <typename TLayout>
    requires tlct::cfg::concepts::CLayout<TLayout>
MIs<TLayout> MIs<TLayout>::fromLayout(const TLayout& layout)
{
    return MIs(layout);
}

template <typename TLayout>
    requires tlct::cfg::concepts::CLayout<TLayout>
MIs<TLayout>& MIs<TLayout>::update(const cv::Mat& src)
{
    cv::Mat Iu8;
    cv::cvtColor(src, Iu8, cv::COLOR_BGR2GRAY);

    auto item_it = items_.begin();
    auto* row_cursor = (uint8_t*)_hp::align_to<Params::SIMD_FETCH_SIZE>((size_t)buffer_);
    size_t row_step = params_.mi_max_cols_ * params_.aligned_mi_size_;
    for (const int irow : rgs::views::iota(0, layout_.getMIRows())) {

        uint8_t* col_cursor = row_cursor;
        const int mi_cols = layout_.getMICols(irow);
        for (const int icol : rgs::views::iota(0, mi_cols)) {
            const auto mi_center = layout_.getMICenter(irow, icol);

            uint8_t* mat_cursor = col_cursor;

            const cv::Mat& I_src = getRoiImageByCenter(Iu8, mi_center, layout_.getDiameter());
            cv::Mat I_dst = cv::Mat(params_.idiameter_, params_.idiameter_, CV_32FC1, mat_cursor);
            I_src.convertTo(I_dst, CV_32F);
            mat_cursor += params_.aligned_mat_size_;

            cv::Mat I_2_src;
            I_src.convertTo(I_2_src, CV_16U);
            cv::multiply(I_2_src, I_2_src, I_2_src);
            cv::Mat I_2_dst = cv::Mat(params_.idiameter_, params_.idiameter_, CV_32FC1, mat_cursor);
            I_2_src.convertTo(I_2_dst, CV_32F);
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