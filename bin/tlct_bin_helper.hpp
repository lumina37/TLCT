#pragma once

#include <expected>
#include <filesystem>
#include <iostream>
#include <print>
#include <string>
#include <utility>

#include "tlct.hpp"

namespace fs = std::filesystem;

class Unwrap {
public:
    template <typename T>
    friend auto operator|(std::expected<T, tlct::Error>&& src, [[maybe_unused]] const Unwrap& _) {
        if (!src.has_value()) {
            const auto& err = src.error();
            const fs::path filePath{err.source.file_name()};
            const std::string fileName = filePath.filename().string();
            std::println(std::cerr, "{}:{} msg={} code={}", fileName, err.source.line(), err.msg, (int)err.code);
            std::exit(1);
        }
        if constexpr (!std::is_void_v<T>) {
            return std::forward_like<T>(src.value());
        }
    }
};

constexpr auto unwrap = Unwrap();
