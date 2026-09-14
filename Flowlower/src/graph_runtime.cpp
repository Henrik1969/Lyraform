#include <cstdint>
#include <algorithm>
#include <exception>
#include <new>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace {
constexpr std::int64_t max_parallel_workers = 256;
constexpr std::size_t max_diagnostic_bytes = 16U * 1024U * 1024U;
thread_local const char* activation = nullptr;
thread_local std::string stream_activation;
thread_local std::uint64_t active_operation = 0;
thread_local int failure_code = 0;
bool tracing() {
    const auto* value = std::getenv("FLOWCORE_GRAPH_TRACE");
    return value && std::strcmp(value, "1") == 0;
}
std::mutex trace_mutex;
std::size_t diagnostic_bytes = 0;
bool diagnostic_truncated = false;
void record(const char* value) {
    if (!value) return;
    std::lock_guard lock(trace_mutex);
    const auto length = ::strnlen(value, max_diagnostic_bytes + 1);
    if (length > max_diagnostic_bytes) {
        if (!diagnostic_truncated) {
            static constexpr char marker[] = "{\"format\":\"flowcore.graph_trace\",\"version\":1,\"event\":\"truncated\",\"limit_bytes\":16777216}";
            std::fputs(marker, stderr); std::fputc('\n', stderr);
            diagnostic_truncated = true;
        }
        return;
    }
    if (length + 1 <= max_diagnostic_bytes - diagnostic_bytes) {
        std::fputs(value, stderr); std::fputc('\n', stderr);
        diagnostic_bytes += length + 1;
    } else if (!diagnostic_truncated) {
        static constexpr char marker[] = "{\"format\":\"flowcore.graph_trace\",\"version\":1,\"event\":\"truncated\",\"limit_bytes\":16777216}";
        std::fputs(marker, stderr); std::fputc('\n', stderr);
        diagnostic_truncated = true;
    }
}
std::string quote(const char* value) {
    std::string result = "\"";
    if (value) {
        const auto limit = ::strnlen(value, max_diagnostic_bytes + 1);
        const auto length = std::min(limit, max_diagnostic_bytes);
        for (std::size_t index = 0; index < length; ++index) {
            const auto character = static_cast<unsigned char>(value[index]);
            if (character == '\\' || character == '"') result += '\\';
            result += static_cast<char>(character);
        }
        if (limit > max_diagnostic_bytes) result += "...";
    }
    result += '"';
    return result;
}
}
extern "C" [[noreturn]] void flow_graph_fail(std::uint64_t operation, const char* reason);
extern "C" void flow_graph_enter(const char* value) {
    activation = value; active_operation = 0; failure_code = 0;
    if (tracing()) record(value);
}
extern "C" void flow_graph_operation(std::uint64_t operation) { active_operation = operation; }
extern "C" void flow_graph_event(const char* value) { if (tracing()) record(value); }
extern "C" void flow_graph_drop(const char* value) { record(value); }
extern "C" void flow_graph_state_before(const char* node, std::int64_t activation_id, std::int64_t state) {
    if (!tracing()) return;
    const auto value = std::string{"{\"format\":\"flowcore.graph_state\",\"version\":1,\"event\":\"before\",\"node_id\":"} +
        quote(node) + ",\"activation_id\":" + std::to_string(activation_id) + ",\"state\":" + std::to_string(state) + "}";
    record(value.c_str());
}
extern "C" void flow_graph_state_after(const char* node, std::int64_t activation_id, std::int64_t state) {
    if (!tracing()) return;
    const auto value = std::string{"{\"format\":\"flowcore.graph_state\",\"version\":1,\"event\":\"after\",\"node_id\":"} +
        quote(node) + ",\"activation_id\":" + std::to_string(activation_id) + ",\"state\":" + std::to_string(state) + "}";
    record(value.c_str());
}
extern "C" void flow_graph_stream_item_enter(const char* node, std::int64_t activation_id,
                                              std::int64_t index, std::int64_t signal) {
    stream_activation = "{\"format\":\"flowcore.graph_activation\",\"version\":1,\"event\":\"enter\",\"node_id\":" + quote(node) +
        ",\"kind\":\"stream_item\",\"activation_id\":" + std::to_string(activation_id) +
        ",\"input_activation_id\":0,\"input_signal_id\":0,\"output_signal_id\":" + std::to_string(signal) +
        ",\"delivery_id\":0,\"stream_index\":" + std::to_string(index) + "}";
    activation = stream_activation.c_str(); active_operation = 0; failure_code = 0;
    if (tracing()) record(stream_activation.c_str());
}
extern "C" void flow_graph_stream_enter(const char* node, const char* wire, std::int64_t activation_id,
                                         std::int64_t index, std::int64_t signal, std::int64_t delivery) {
    stream_activation = "{\"format\":\"flowcore.graph_activation\",\"version\":1,\"event\":\"enter\",\"node_id\":" + quote(node) +
        ",\"kind\":\"stream_receiver\",\"activation_id\":" + std::to_string(activation_id) +
        ",\"input_activation_id\":0,\"input_signal_id\":" + std::to_string(signal) +
        ",\"output_signal_id\":" + std::to_string(activation_id + 1) + ",\"delivery_id\":" + std::to_string(delivery) +
        ",\"wire_id\":" + quote(wire) + ",\"stream_index\":" + std::to_string(index) + "}";
    activation = stream_activation.c_str(); active_operation = 0; failure_code = 0;
    if (tracing()) record(stream_activation.c_str());
}
extern "C" void flow_graph_stream_event(const char* node, std::int64_t activation_id,
                                         std::int64_t index, std::int64_t signal) {
    if (!tracing()) return;
    const auto value = std::string{"{\"format\":\"flowcore.graph_activation\",\"version\":1,\"event\":\"output\",\"node_id\":"} +
        quote(node) + ",\"activation_id\":" + std::to_string(activation_id) +
        ",\"output_signal_id\":" + std::to_string(signal) + ",\"stream_index\":" + std::to_string(index) + "}";
    record(value.c_str());
}
extern "C" void flow_graph_stream_drop(const char* node, const char* wire, std::int64_t activation_id,
                                        std::int64_t index, std::int64_t delivery) {
    const auto value = std::string{"{\"format\":\"flowcore.graph_activation\",\"version\":1,\"event\":\"drop\",\"node_id\":"} +
        quote(node) + ",\"activation_id\":" + std::to_string(activation_id) +
        ",\"wire_id\":" + quote(wire) + ",\"stream_index\":" + std::to_string(index) +
        ",\"delivery_id\":" + std::to_string(delivery) + "}";
    record(value.c_str());
}
extern "C" void flow_graph_parallel_run(
    void (*const* workers)(std::int64_t, std::int64_t*), const std::int64_t* inputs,
    std::int64_t* outputs, std::int64_t count) {
    if (!workers || !inputs || !outputs || count < 0) flow_graph_fail(active_operation, "invalid graph parallel invocation");
    if (count > max_parallel_workers) flow_graph_fail(active_operation, "graph parallel worker count exceeds the 256-worker limit");
    // jthread joins already-started workers if vector growth or a later
    // worker launch fails, preventing partial initialization from reaching
    // std::terminate with joinable threads.
    std::vector<std::jthread> threads;
    threads.reserve(static_cast<std::size_t>(count));
    for (std::int64_t index = 0; index < count; ++index) {
        if (!workers[index]) flow_graph_fail(active_operation, "graph parallel invocation contains a null worker");
        const auto operation = active_operation;
        threads.emplace_back([worker = workers[index], input = inputs[index], output = &outputs[index], operation] {
            try {
                worker(input, output);
            } catch (const std::exception&) {
                flow_graph_fail(operation, "graph parallel worker failed");
            } catch (...) {
                flow_graph_fail(operation, "graph parallel worker failed with a non-standard exception");
            }
        });
#if defined(FLOWCORE_GRAPH_RUNTIME_TEST_FAULT)
        if (index == 0) throw std::bad_alloc();
#endif
    }
    for (auto& thread : threads) thread.join();
}
extern "C" void flow_graph_parallel_result(std::int64_t activation_id, std::int64_t value) {
    if (!tracing()) return;
    const auto record_value = std::string{"{\"format\":\"flowcore.graph_parallel\",\"version\":1,\"event\":\"result\",\"activation_id\":"} +
        std::to_string(activation_id) + ",\"value\":" + std::to_string(value) + "}";
    record(record_value.c_str());
}
extern "C" [[noreturn]] void flow_graph_fail(std::uint64_t operation, const char* reason) {
    // Both strings are compiler-serialized constants; no payload or raw pointer
    // is interpolated into the diagnostic. Failure never publishes an output.
    const char* safe_activation = activation;
    if (safe_activation && ::strnlen(safe_activation, max_diagnostic_bytes + 1) > max_diagnostic_bytes)
        safe_activation = nullptr;
    std::fprintf(stderr, "{\"format\":\"flowcore.graph_failure\",\"version\":1,\"operation_id\":%llu,\"reason\":\"%s\",\"code\":%d,\"activation\":%s}\n",
        static_cast<unsigned long long>(operation), reason, failure_code, safe_activation ? safe_activation : "null");
    std::exit(70);
}

extern "C" int flow_graph_raise(int code) {
    failure_code = code;
    flow_graph_fail(active_operation, "source_failure");
}
