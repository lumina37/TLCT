#pragma once

#include <cstdint>
#include <vector>

namespace tlct::_cvt {

template <bool ENABLE_DEBUG>
class PatchRecord_;

template <>
class PatchRecord_<true> {
public:
    void setPsize(const float v) noexcept { psize_ = v; }
    void setDhash(const uint16_t v) noexcept { dhash_ = v; }
    void setIsBlurredNear(const bool v) noexcept { isBlurredNear = v; }
    void setIsBlurredFar(const bool v) noexcept { isBlurredFar = v; }
    void setDhashDiff(const uint16_t v) noexcept { dhashDiff = v; }
    void setNearMetrics(std::vector<float>&& v) noexcept { nearMetrics = std::move(v); }
    void setFarMetrics(std::vector<float>&& v) noexcept { farMetrics = std::move(v); }

    [[nodiscard]] float getPsize() const noexcept { return psize_; }
    [[nodiscard]] uint16_t getDhash() const noexcept { return dhash_; }
    [[nodiscard]] bool getIsBlurredNear() const noexcept { return isBlurredNear; }
    [[nodiscard]] bool getIsBlurredFar() const noexcept { return isBlurredFar; }
    [[nodiscard]] int getDhashDiff() const noexcept { return dhashDiff; }
    [[nodiscard]] std::vector<float>& getNearMetrics() noexcept { return nearMetrics; }
    [[nodiscard]] const std::vector<float>& getNearMetrics() const noexcept { return nearMetrics; }
    [[nodiscard]] std::vector<float>& getFarMetrics() noexcept { return farMetrics; }
    [[nodiscard]] const std::vector<float>& getFarMetrics() const noexcept { return farMetrics; }

private:
    float psize_;
    uint16_t dhash_;
    int dhashDiff;
    bool isBlurredNear;
    bool isBlurredFar;
    std::vector<float> nearMetrics;
    std::vector<float> farMetrics;
};

template <>
class PatchRecord_<false> {
public:
    void setPsize(const float v) noexcept { psize_ = v; }
    void setDhash(const uint16_t v) noexcept { dhash_ = v; }
    void setIsBlurredNear(const bool) noexcept {}
    void setIsBlurredFar(const bool) noexcept {}
    void setDhashDiff(const uint16_t) noexcept {}
    void setNearMetrics(std::vector<float>&&) noexcept {}
    void setFarMetrics(std::vector<float>&&) noexcept {}

    [[nodiscard]] float getPsize() const noexcept { return psize_; }
    [[nodiscard]] uint16_t getDhash() const noexcept { return dhash_; }
    [[nodiscard]] bool getIsBlurredNear() const noexcept { return false; }
    [[nodiscard]] bool getIsBlurredFar() const noexcept { return false; }
    [[nodiscard]] int getDhashDiff() const noexcept { return 0; }
    [[nodiscard]] std::vector<float>& getNearMetrics() noexcept { return nullVec; }
    [[nodiscard]] const std::vector<float>& getNearMetrics() const noexcept { return nullVec; }
    [[nodiscard]] std::vector<float>& getFarMetrics() noexcept { return nullVec; }
    [[nodiscard]] const std::vector<float>& getFarMetrics() const noexcept { return nullVec; }

private:
    static std::vector<float> nullVec;
    float psize_;
    uint16_t dhash_;
};

}  // namespace tlct::_cvt
