#pragma once

#include <cstddef>
#include <cstdio>
#include <string_view>

namespace flowtools::reference {

inline constexpr std::size_t max_diagnostic_bytes = 4096;

inline void write_json_string(std::FILE* output, std::string_view value) noexcept {
    std::size_t emitted = 0;
    const auto put = [&](const char* text) {
        for (const char* character = text; *character != '\0'; ++character) {
            if (emitted >= max_diagnostic_bytes) return false;
            (void)std::fputc(*character, output);
            ++emitted;
        }
        return true;
    };
    for (const unsigned char character : value) {
        const char* escaped = nullptr;
        switch (character) {
        case '\\': escaped = "\\\\"; break;
        case '"': escaped = "\\\""; break;
        case '\n': escaped = "\\n"; break;
        case '\r': escaped = "\\r"; break;
        case '\t': escaped = "\\t"; break;
        default:
            if (emitted >= max_diagnostic_bytes) { (void)put("..."); return; }
            (void)std::fputc(character, output);
            ++emitted;
            continue;
        }
        if (!put(escaped)) { (void)std::fputs("...", output); return; }
    }
}

} // namespace flowtools::reference
