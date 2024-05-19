#include <filesystem>
#include <iostream>

#include <gtest/gtest.h>

#include "tlct/config.hpp"

using namespace tlct;
namespace fs = std::filesystem;

TEST(Experiment, TSPC)
{
    const auto calib_cfg = cfg::_experiment::CalibConfig::fromXMLPath("exp_calib.xml");
    auto layout = cfg::_experiment::Layout::fromCfgAndImgsize(calib_cfg, {4080, 3068});

    std::cout << layout.getMICenter(0, layout.getMICols() - 1) << std::endl;
    std::cout << layout.getMICenter(layout.getMIRows() - 1, 0) << std::endl;
}
