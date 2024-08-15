#include <filesystem>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "tlct.hpp"

namespace fs = std::filesystem;

TEST_CASE("CommonParamConfig")
{
    const fs::path testdata_dir{TLCT_TESTDATA_DIR};
    const fs::path cfg_path = testdata_dir / "config/TSPC/param.cfg";
    const auto config = tlct::ConfigMap::fromPath(cfg_path.string());

    CHECK(config.isEmpty() == false);
    CHECK(config.getPipelineType() == tlct::PipelineType::TLCT);
}
