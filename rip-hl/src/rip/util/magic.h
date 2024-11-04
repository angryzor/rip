#pragma once
#include <algorithm>
#include <array>
#include <string>
#include <string_view>

namespace rip {
    template <size_t N = 4>
    struct magic_t {
        std::array<char, N> buffer{};

        constexpr magic_t(const auto... characters) : buffer{ characters... } {}
        constexpr magic_t(const std::array<char, N> buffer) : buffer{ buffer } {}
        constexpr magic_t(const char(&str)[N + 1]) { std::copy_n(str, N, std::data(buffer)); }

        operator std::string() const { return std::string(std::data(buffer), N); }
        constexpr operator std::string_view() const { return std::string_view(std::data(buffer), N); }
    };
}
