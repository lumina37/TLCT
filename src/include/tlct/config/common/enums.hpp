#pragma once

namespace tlct::_cfg {

enum PipelineType {
    RLC,
    TLCT,
    COUNT,
};

static constexpr int PIPELINE_COUNT = (int)PipelineType::COUNT;

} // namespace tlct::_cfg
