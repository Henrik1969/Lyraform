#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace {
thread_local const char* activation = nullptr;
thread_local std::string stream_activation;
thread_local std::uint64_t active_operation = 0;
thread_local int failure_code = 0;
bool tracing() {
    const auto* value = std::getenv("FLOWCORE_GRAPH_TRACE");
    return value && std::strcmp(value, "1") == 0;
}
void record(const char* value) { if (value) { std::fputs(value, stderr); std::fputc('\n', stderr); } }
std::string quote(const char* value) {
    std::string result = "\"";
    if (value) for (const unsigned char* p = reinterpret_cast<const unsigned char*>(value); *p; ++p) {
        if (*p == '\\' || *p == '"') result += '\\';
        result += static_cast<char>(*p);
    }
    result += '"';
    return result;
}
}
extern "C" void flow_graph_enter(const char* value) {
    activation = value; active_operation = 0; failure_code = 0;
    if (tracing()) record(value);
}
extern "C" void flow_graph_operation(std::uint64_t operation) { active_operation = operation; }
extern "C" void flow_graph_event(const char* value) { if (tracing()) record(value); }
extern "C" void flow_graph_drop(const char* value) { record(value); }
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
extern "C" [[noreturn]] void flow_graph_fail(std::uint64_t operation, const char* reason) {
    // Both strings are compiler-serialized constants; no payload or raw pointer
    // is interpolated into the diagnostic. Failure never publishes an output.
    std::fprintf(stderr, "{\"format\":\"flowcore.graph_failure\",\"version\":1,\"operation_id\":%llu,\"reason\":\"%s\",\"code\":%d,\"activation\":%s}\n",
        static_cast<unsigned long long>(operation), reason, failure_code, activation ? activation : "null");
    std::exit(70);
}

extern "C" int flow_graph_raise(int code) {
    failure_code = code;
    flow_graph_fail(active_operation, "source_failure");
}
