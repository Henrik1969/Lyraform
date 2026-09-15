#pragma once

#include <cstddef>
#include <cstdio>
#include <string_view>

namespace flowtools::reference {

inline constexpr std::size_t max_diagnostic_bytes = 4096;

inline void write_json_string(std::FILE* output, std::string_view value) noexcept {
    std::size_t emitted = 0;
    const auto put = [&](const char* text, const std::size_t length) {
        for (std::size_t index = 0; index < length; ++index) {
            (void)std::fputc(text[index], output);
            ++emitted;
        }
    };
    const auto truncated = [&] {
        if (emitted <= max_diagnostic_bytes - 3) put("...", 3);
    };
    for (const unsigned char character : value) {
        const char* escaped = nullptr;
        std::size_t escaped_length = 0;
        char control_escape[6]{};
        switch (character) {
        case '\\': escaped = "\\\\"; escaped_length = 2; break;
        case '"': escaped = "\\\""; escaped_length = 2; break;
        case '\n': escaped = "\\n"; escaped_length = 2; break;
        case '\r': escaped = "\\r"; escaped_length = 2; break;
        case '\t': escaped = "\\t"; escaped_length = 2; break;
        default:
            if (character < 0x20) {
                constexpr char hex[] = "0123456789abcdef";
                control_escape[0] = '\\';
                control_escape[1] = 'u';
                control_escape[2] = '0';
                control_escape[3] = '0';
                control_escape[4] = hex[(character >> 4) & 0xf];
                control_escape[5] = hex[character & 0xf];
                escaped = control_escape;
                escaped_length = 6;
            } else {
                if (emitted >= max_diagnostic_bytes - 3) { truncated(); return; }
                (void)std::fputc(character, output);
                ++emitted;
                continue;
            }
        }
        if (emitted > max_diagnostic_bytes - 3 ||
            escaped_length > max_diagnostic_bytes - 3 - emitted) {
            truncated();
            return;
        }
        put(escaped, escaped_length);
    }
}

} // namespace flowtools::reference
