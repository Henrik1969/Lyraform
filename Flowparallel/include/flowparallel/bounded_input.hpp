#pragma once

#include <array>
#include <cstddef>
#include <istream>
#include <stdexcept>
#include <string>

namespace flowparallel {

inline constexpr std::size_t max_input_bytes = 16U * 1024U * 1024U;

inline std::string read_bounded(std::istream& input, const char* label) {
    std::string result;
    std::array<char, 8192> buffer{};
    while (input) {
        input.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        const auto count = input.gcount();
        if (count <= 0) break;
        const auto bytes = static_cast<std::size_t>(count);
        if (bytes > max_input_bytes - result.size())
            throw std::runtime_error(std::string(label) + " exceeds the 16 MiB input limit");
        result.append(buffer.data(), bytes);
    }
    if (input.bad()) throw std::runtime_error(std::string("failed while reading ") + label);
    return result;
}

} // namespace flowparallel
