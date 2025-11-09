#pragma once

#include <vector>

#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/helper/std.hpp"

namespace tlct::_cvt {

class PatchMergeDebugInfo {
public:
    int dhashDiff = 0;
    bool isBlurredNear = false;
    bool isBlurredFar = false;
    std::vector<float> nearMetrics{};
    std::vector<float> farMetrics{};
};

template <bool ENABLE_DEBUG>
class PatchMergeInfo_;

template <>
class PatchMergeInfo_<true> {
public:
    void setPatchsize(const float v) noexcept { patchsize_ = v; }
    void setDhash(const uint16_t v) noexcept { dhash_ = v; }

    [[nodiscard]] float getPatchsize() const noexcept { return patchsize_; }
    [[nodiscard]] uint16_t getDhash() const noexcept { return dhash_; }
    [[nodiscard]] PatchMergeDebugInfo* getPDebugInfo() noexcept { return &debugInfo_; }

private:
    float patchsize_;
    float weight_;
    uint16_t dhash_;
    PatchMergeDebugInfo debugInfo_;
};

template <>
class PatchMergeInfo_<false> {
public:
    void setPatchsize(const float v) noexcept { psize_ = v; }
    void setDhash(const uint16_t v) noexcept { dhash_ = v; }

    [[nodiscard]] float getPatchsize() const noexcept { return psize_; }
    [[nodiscard]] uint16_t getDhash() const noexcept { return dhash_; }
    [[nodiscard]] PatchMergeDebugInfo* getPDebugInfo() noexcept { return nullptr; }

private:
    float psize_;
    uint16_t dhash_;
};

template <cfg::concepts::CArrange TArrange_, bool ENABLE_DEBUG>
class PatchMergeBridge_ {
public:
    using TArrange = TArrange_;
    using TInfo = PatchMergeInfo_<ENABLE_DEBUG>;
    using TInfos = std::vector<TInfo>;

private:
    PatchMergeBridge_(const TArrange& arrange, std::vector<TInfo>&& infos, std::vector<float>&& weights) noexcept;

public:
    // Initialize from
    [[nodiscard]] TLCT_API static std::expected<PatchMergeBridge_, Error> create(const TArrange& arrange) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API float getPatchsize(int offset) const noexcept { return infos_[offset].getPatchsize(); }
    [[nodiscard]] TLCT_API float getPatchsize(int row, int col) const noexcept {
        const int offset = row * arrange_.getMIMaxCols() + col;
        return getPatchsize(offset);
    }
    [[nodiscard]] TLCT_API float getWeight(int offset) const noexcept { return weights_[offset]; }
    [[nodiscard]] TLCT_API float getWeight(int row, int col) const noexcept {
        const int offset = row * arrange_.getMIMaxCols() + col;
        return getWeight(offset);
    }

    [[nodiscard]] TLCT_API const TInfos& getInfos() const noexcept { return infos_; }

    // Non-const methods
    TLCT_API void swapInfos(TInfos& rhs) noexcept { return std::swap(infos_, rhs); }

    [[nodiscard]] TLCT_API TInfo& getInfo(int offset) noexcept { return infos_[offset]; }
    [[nodiscard]] TLCT_API TInfo& getInfo(int row, int col) noexcept {
        const int offset = row * arrange_.getMIMaxCols() + col;
        return getInfo(offset);
    }

    [[nodiscard]] TLCT_API TInfos& getInfos() noexcept { return infos_; }
    TLCT_API void setInfos(TInfos& infos) noexcept { infos_ = infos; }

    TLCT_API void setWeight(int offset, float v) noexcept { weights_[offset] = v; }
    TLCT_API void setWeight(int row, int col, float v) noexcept {
        const int offset = row * arrange_.getMIMaxCols() + col;
        setWeight(offset, v);
    }

private:
    TArrange arrange_;
    TInfos infos_;
    std::vector<float> weights_;
};

}  // namespace tlct::_cvt

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/common/bridge/patch_merge.cpp"
#endif
