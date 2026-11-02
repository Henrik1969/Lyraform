#include "frankencore/provenance.hpp"

#include <chrono>
#include <new>
#include <stdexcept>

namespace frankencore::provenance {
namespace {

constexpr char ENCODING[] = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";

std::uint64_t now_milliseconds() {
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}

void fill_random(std::array<std::uint8_t, 10>& bytes, std::random_device& source) {
    for (auto& byte : bytes) byte = static_cast<std::uint8_t>(source());
}

void increment_random(std::array<std::uint8_t, 10>& bytes) {
    for (auto index = bytes.size(); index-- > 0;) {
        if (++bytes[index] != 0) return;
    }
    throw std::runtime_error("ULID monotonic entropy exhausted");
}

std::string encode(std::uint64_t timestamp, const std::array<std::uint8_t, 10>& random) {
    std::string result(26, '0');
    for (int index = 9; index >= 0; --index) {
        result[static_cast<std::size_t>(index)] = ENCODING[timestamp & 31U];
        timestamp >>= 5;
    }
    std::uint32_t buffer = 0;
    int bits = 0;
    std::size_t output = 10;
    for (const auto byte : random) {
        buffer = (buffer << 8U) | byte;
        bits += 8;
        while (bits >= 5 && output < result.size()) {
            bits -= 5;
            result[output++] = ENCODING[(buffer >> bits) & 31U];
        }
    }
    return result;
}

} // namespace

UlidGenerator::UlidGenerator() {
    fill_random(last_random_, random_device_);
}

Ulid UlidGenerator::generate() {
    std::lock_guard lock(mutex_);
    auto timestamp = now_milliseconds();
    if (timestamp < last_timestamp_ms_) timestamp = last_timestamp_ms_;
    if (timestamp == last_timestamp_ms_) increment_random(last_random_);
    else fill_random(last_random_, random_device_);
    last_timestamp_ms_ = timestamp;
    return encode(timestamp, last_random_);
}

UlidResult UlidGenerator::generate_checked() noexcept {
    try {
        return {true, generate(), {}};
    } catch (const std::bad_alloc&) {
        return {false, {}, "ULID generation exhausted memory"};
    } catch (const std::exception& error) {
        (void)error;
        return {false, {}, "ULID generation failed"};
    } catch (...) {
        return {false, {}, "ULID generation failed with unknown non-standard failure"};
    }
}

Ulid generate_ulid() {
    static UlidGenerator generator;
    return generator.generate();
}

UlidResult generate_ulid_checked() noexcept {
    try {
        static UlidGenerator generator;
        return generator.generate_checked();
    } catch (const std::bad_alloc&) {
        return {false, {}, "ULID generator initialization exhausted memory"};
    } catch (const std::exception& error) {
        (void)error;
        return {false, {}, "ULID generator initialization failed"};
    } catch (...) {
        return {false, {}, "ULID generator initialization failed with unknown non-standard failure"};
    }
}

bool is_valid_ulid(const std::string& value) {
    if (value.size() != 26 || value[0] > '7') return false;
    for (const char character : value) {
        bool found = false;
        for (const char allowed : ENCODING)
            if (character == allowed) found = true;
        if (!found) return false;
    }
    return true;
}

} // namespace frankencore::provenance
