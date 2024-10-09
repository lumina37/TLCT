#pragma once

namespace tlct::_cfg {

enum class PipelineType {
    RLC,
    TLCT,
    COUNT,
};

static constexpr size_t PIPELINE_COUNT = (size_t)PipelineType::COUNT;

} // namespace tlct::_cfg
