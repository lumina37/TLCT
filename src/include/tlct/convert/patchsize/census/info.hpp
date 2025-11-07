#pragma once

#include <vector>

#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/helper/std.hpp"

namespace tlct::_cvt::census {

class PatchDebugInfo {
public:
    int dhashDiff = 0;
    bool isBlurredNear = false;
    bool isBlurredFar = false;
    std::vector<float> nearMetrics{};
    std::vector<float> farMetrics{};
};

template <bool ENABLE_DEBUG>
class PatchInfo_;

template <>
class PatchInfo_<true> {
public:
    void setPatchsize(const float v) noexcept { patchsize_ = v; }
    void setDhash(const uint16_t v) noexcept { dhash_ = v; }

    [[nodiscard]] float getPatchsize() const noexcept { return patchsize_; }
    [[nodiscard]] uint16_t getDhash() const noexcept { return dhash_; }
    [[nodiscard]] PatchDebugInfo* getPDebugInfo() noexcept { return &debugInfo_; }

private:
    float patchsize_;
    float weight_;
    uint16_t dhash_;
    PatchDebugInfo debugInfo_;
};

template <>
class PatchInfo_<false> {
public:
    void setPatchsize(const float v) noexcept { psize_ = v; }
    void setDhash(const uint16_t v) noexcept { dhash_ = v; }

    [[nodiscard]] float getPatchsize() const noexcept { return psize_; }
    [[nodiscard]] uint16_t getDhash() const noexcept { return dhash_; }
    [[nodiscard]] PatchDebugInfo* getPDebugInfo() noexcept { return nullptr; }

private:
    float psize_;
    uint16_t dhash_;
};

template <cfg::concepts::CArrange TArrange_, bool ENABLE_DEBUG>
class PatchInfos_ {
public:
    using TArrange = TArrange_;
    using TPatchInfo = PatchInfo_<ENABLE_DEBUG>;
    using TPatchInfoVec = std::vector<TPatchInfo>;

private:
    PatchInfos_(const TArrange& arrange, std::vector<TPatchInfo>&& infos, std::vector<float>&& weights) noexcept;

public:
    // Initialize from
    [[nodiscard]] TLCT_API static std::expected<PatchInfos_, Error> create(const TArrange& arrange) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API float getPatchsize(int offset) const noexcept { return infoVec_[offset].getPatchsize(); }
    [[nodiscard]] TLCT_API float getPatchsize(int row, int col) const noexcept {
        const int offset = row * arrange_.getMIMaxCols() + col;
        return getPatchsize(offset);
    }
    [[nodiscard]] TLCT_API float getWeight(int offset) const noexcept { return weights_[offset]; }
    [[nodiscard]] TLCT_API float getWeight(int row, int col) const noexcept {
        const int offset = row * arrange_.getMIMaxCols() + col;
        return getWeight(offset);
    }

    [[nodiscard]] TLCT_API const TPatchInfoVec& getInfoVec() const noexcept { return infoVec_; }

    // Non-const methods
    TLCT_API void swapInfos(TPatchInfoVec& rhs) noexcept { return std::swap(infoVec_, rhs); }

    [[nodiscard]] TLCT_API TPatchInfo& getInfo(int offset) noexcept { return infoVec_[offset]; }
    [[nodiscard]] TLCT_API TPatchInfo& getInfo(int row, int col) noexcept {
        const int offset = row * arrange_.getMIMaxCols() + col;
        return getInfo(offset);
    }

    [[nodiscard]] TLCT_API TPatchInfoVec& getInfoVec() noexcept { return infoVec_; }
    TLCT_API void setInfoVec(TPatchInfoVec& infoVec) noexcept { infoVec_ = infoVec; }

    TLCT_API void setWeight(int offset, float v) noexcept { weights_[offset] = v; }
    TLCT_API void setWeight(int row, int col, float v) noexcept {
        const int offset = row * arrange_.getMIMaxCols() + col;
        setWeight(offset, v);
    }

private:
    TArrange arrange_;
    TPatchInfoVec infoVec_;
    std::vector<float> weights_;
};

}  // namespace tlct::_cvt::census
