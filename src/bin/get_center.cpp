#include <iostream>

#include "tlct.hpp"

namespace tcfg = tlct::cfg::tspc;
namespace tcvt = tlct::cvt::tspc;

int main(int argc, char* argv[])
{
    const auto cfg_map = tlct::cfg::ConfigMap::fromPath(argv[1]);
    const int row = std::stoi(argv[2]);
    const int col = std::stoi(argv[3]);

    if (cfg_map.getPipelineType() == tlct::cfg::PipelineType::RLC) {
        const auto param_cfg = tlct::cfg::raytrix::ParamConfig::fromConfigMap(cfg_map);
        const auto layout = tlct::cfg::raytrix::Layout::fromCfgAndImgsize(param_cfg.getCalibCfg(),
                                                                          param_cfg.getSpecificCfg().getImgSize());
        const auto center = layout.getMICenter(row, col);
        std::cout << center << std::endl;
    } else {
        const auto param_cfg = tlct::cfg::tspc::ParamConfig::fromConfigMap(cfg_map);
        const auto layout = tlct::cfg::tspc::Layout::fromCfgAndImgsize(param_cfg.getCalibCfg(),
                                                                       param_cfg.getSpecificCfg().getImgSize());
        const auto center = layout.getMICenter(row, col);
        std::cout << center << std::endl;
    }
}