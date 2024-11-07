#pragma once

#include "tlct/config.hpp"
#include "tlct/convert.hpp"
#include "tlct/helper.hpp"
#include "tlct/io.hpp"
#include "tlct/namespace.hpp"

namespace tlct {

using cfg::ConfigMap;
using cfg::GenericParamConfig;
using cfg::PIPELINE_COUNT;
using cfg::PipelineType;

} // namespace tlct

#ifdef PUGIXML_HEADER_ONLY
#    include "pugixml.cpp"
#endif
