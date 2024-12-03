#include <filesystem>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "tlct.hpp"

namespace fs = std::filesystem;

static std::string get_harmony() { return "6"; }

TEST_CASE("tlct::ConfigMap")
{
    const fs::path testdata_dir{TLCT_TESTDATA_DIR};
    fs::current_path(testdata_dir);
    const auto config = tlct::ConfigMap::fromPath("test/param.cfg");

    CHECK(config.isEmpty() == false);
    CHECK(config.getPipelineType() == tlct::PipelineType::TLCT);

    // trivially-copyable variable
    CHECK(config.get<"views", int>() == 5);
    CHECK(config.get<"views", int>(37) == 5);
    CHECK(config.get<"not_exist">(37) == 37);

    // function as factory
    CHECK(config.get<"inFile", std::string_view>() == "./Cars/src.yuv");
    CHECK(config.get<"inFile">(get_harmony) == "./Cars/src.yuv");
    CHECK(config.get<"not_exist">(get_harmony) == "6");

    // lambda as factory
    const std::string hermes{"37"};
    const auto get_hermes = [&]() -> std::string_view { return hermes; };
    CHECK(config.get<"inFile">(get_hermes) == "./Cars/src.yuv");
    CHECK(config.get<"not_exist">(get_hermes) == "37");
}
