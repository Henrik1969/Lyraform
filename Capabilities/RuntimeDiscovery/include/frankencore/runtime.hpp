#pragma once

#include <cstdint>
#include <string>

namespace frankencore::runtime {

struct CpuFacts { std::uint64_t logical_processors = 0; };
// Values are parsed from the Linux kB representation with strict numeric
// validation and saturating conversion; malformed fields become 0.
struct MemoryFacts { std::uint64_t total_bytes = 0; std::uint64_t available_bytes = 0; };
struct CudaFacts {
    std::string status = "unknown";
    std::string driver;
    std::uint64_t device_count = 0;
    std::string diagnostic;
};
struct Capabilities {
    std::string format = "frankencore.runtime_capabilities";
    int version = 1;
    CpuFacts cpu;
    MemoryFacts memory;
    CudaFacts cuda;
};

struct DiscoveryResult {
    bool valid = false;
    Capabilities capabilities;
    std::string error;
};

struct JsonResult {
    bool valid = false;
    std::string json;
    std::string error;
};

// Read-only discovery. It does not enable providers, allocate workers, or
// resolve policy. Those decisions belong to a later runtime planner.
Capabilities discover();

// Non-throwing capability-discovery boundary. Invalid results contain no
// admitted capability snapshot.
DiscoveryResult discover_checked() noexcept;

// JSON is an inspectable projection; the C++ structure remains canonical.
std::string to_json(const Capabilities& capabilities);

// Checked public projection boundary. On failure, json is empty and no
// serialized artifact is admitted.
JsonResult to_json_checked(const Capabilities& capabilities) noexcept;

} // namespace frankencore::runtime
