#include <iostream>

#include "tlct.hpp"

namespace tcfg = tlct::cfg::tspc;
namespace tcvt = tlct::cvt::tspc;

int main(int argc, char* argv[])
{
    using ParamConfig = tcfg::ParamConfig<tcfg::CalibConfig>;

    const auto cfg_map = tlct::cfg::ConfigMap::fromPath(argv[1]);
    const auto param_cfg = ParamConfig::fromConfigMap(cfg_map);

    constexpr int upsample = 1;
    const auto layout =
        tcfg::Layout::fromCfgAndImgsize(param_cfg.getCalibCfg(), param_cfg.getImgSize()).upsample(upsample);

    const int row = std::stoi(argv[2]);
    const int col = std::stoi(argv[3]);

    const auto center = layout.getMICenter(row, col);
    std::cout << center << std::endl;

    const auto neighbors = tcvt::_hp::NeibMIIndices::fromLayoutAndIndex(layout, {col, row});
    if (neighbors.hasLeft()) {
        std::cout << "Left:" << layout.getMICenter(neighbors.getLeft()) << std::endl;
    }
    if (neighbors.hasRight()) {
        std::cout << "Right:" << layout.getMICenter(neighbors.getRight()) << std::endl;
    }
    if (neighbors.hasUpLeft()) {
        std::cout << "UpLeft:" << layout.getMICenter(neighbors.getUpLeft()) << std::endl;
    }
    if (neighbors.hasUpRight()) {
        std::cout << "UpRight:" << layout.getMICenter(neighbors.getUpRight()) << std::endl;
    }
    if (neighbors.hasDownLeft()) {
        std::cout << "DownLeft:" << layout.getMICenter(neighbors.getDownLeft()) << std::endl;
    }
    if (neighbors.hasDownRight()) {
        std::cout << "DownRight:" << layout.getMICenter(neighbors.getDownRight()) << std::endl;
    }
}