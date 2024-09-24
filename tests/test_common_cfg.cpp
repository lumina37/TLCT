#include <filesystem>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "tlct.hpp"

namespace fs = std::filesystem;

TEST_CASE("CommonParamConfig")
{
    const fs::path testdata_dir{TLCT_TESTDATA_DIR};
    fs::current_path(testdata_dir);
    const auto config = tlct::ConfigMap::fromPath("config/TSPC/param.cfg");

    CHECK(config.isEmpty() == false);
    CHECK(config.getPipelineType() == tlct::PipelineType::TLCT);
}
