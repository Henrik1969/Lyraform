#include "flowmini_frontend_bundle.h"

#include <iomanip>
#include <map>
#include <ostream>
#include <string>
#include <vector>

namespace flowmini::ast {
namespace {

void dump_json_string(std::ostream& out, const std::string_view value) {
    out << '"';
    for (const unsigned char ch : value) {
        switch (ch) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (ch < 0x20) {
                    out << "\\u00"
                        << std::hex << std::setw(2) << std::setfill('0')
                        << static_cast<unsigned int>(ch)
                        << std::dec << std::setfill(' ');
                } else {
                    out << static_cast<char>(ch);
                }
        }
    }
    out << '"';
}

void dump_origin_descriptor(std::ostream& out, const AstOriginDescriptor& descriptor) {
    out << "\"entity_kind\": ";
    dump_json_string(out, to_string(descriptor.entity_kind));
    out << ", \"role\": ";
    dump_json_string(out, to_string(descriptor.role));
    out << ", \"ast_id\": ";
    if (descriptor.ast_id) {
        out << *descriptor.ast_id;
    } else {
        out << "null";
    }
    out << ", \"source_location\": {\"line\": " << descriptor.location.line
        << ", \"column\": " << descriptor.location.column << '}';
}

} // namespace

void dump_frontend_bundle_json(std::ostream& out,
                               const AstModule& module,
                               const SymbolProjection& projection,
                               const std::string_view sourcePath,
                               const std::vector<FrontendSourceLineOrigin>& lineOrigins) {
    std::map<std::string, std::size_t> fileIds;
    std::vector<std::string> files;
    for (const auto& origin : lineOrigins) {
        if (origin.source_path.empty() || fileIds.contains(origin.source_path)) {
            continue;
        }
        const auto id = files.size();
        files.push_back(origin.source_path);
        fileIds.emplace(origin.source_path, id);
    }

    out << "{\n"
        << "  \"format\": \"flowmini.frontend_bundle\",\n"
        << "  \"version\": 2,\n"
        << "  \"parse_validity\": {\"format\":\"lyraform.parse_validity\",\"version\":1,\"state\":";
    dump_json_string(out, module.parse_validity.state);
    out << ",\"scope\":"; dump_json_string(out, module.parse_validity.scope);
    out << ",\"coverage\":"; dump_json_string(out, module.parse_validity.coverage);
    out << ",\"recovery_used\":" << (module.parse_validity.recovery ? "true" : "false");
    out << ",\"message\":"; dump_json_string(out, module.parse_validity.message);
    out << "},\n  \"source\": {\"path\": ";
    dump_json_string(out, sourcePath);
    out << "},\n"
        << "  \"source_map\": {\n"
        << "    \"coordinate_space\": \"expanded_lines\",\n"
        << "    \"files\": [\n";

    for (std::size_t index = 0; index < files.size(); ++index) {
        out << "      {\"id\": " << index << ", \"path\": ";
        dump_json_string(out, files[index]);
        out << '}';
        if (index + 1 < files.size()) {
            out << ',';
        }
        out << '\n';
    }

    out << "    ],\n"
        << "    \"lines\": [\n";

    for (std::size_t index = 0; index < lineOrigins.size(); ++index) {
        const auto& origin = lineOrigins[index];
        out << "      {\"expanded_line\": " << (index + 1)
            << ", \"source_id\": ";
        if (origin.source_path.empty()) {
            out << "null, \"source_line\": null";
        } else {
            out << fileIds.at(origin.source_path)
                << ", \"source_line\": " << origin.source_line;
        }
        out << '}';
        if (index + 1 < lineOrigins.size()) {
            out << ',';
        }
        out << '\n';
    }

    out << "    ]\n"
        << "  },\n"
        << "  \"ast\": ";
    dump_ast_json(out, module);
    out << ",\n"
        << "  \"symbol_table\": ";
    projection.table.dumpJson(out);
    out << ",\n"
        << "  \"origin_contract\": {\"format\": \"flowmini.structural_origins\", "
           "\"version\": 1},\n"
        << "  \"symbol_origins\": [\n";

    for (std::size_t index = 0; index < projection.symbol_origins.size(); ++index) {
        const auto& origin = projection.symbol_origins[index];
        out << "    {\"symbol_id\": " << origin.symbol_id.value
            << ", \"ast_path\": ";
        dump_json_string(out, origin.ast_path);
        out << ", ";
        dump_origin_descriptor(out, origin.descriptor);
        out << '}';
        if (index + 1 < projection.symbol_origins.size()) {
            out << ',';
        }
        out << '\n';
    }

    out << "  ],\n"
        << "  \"scope_origins\": [\n";

    for (std::size_t index = 0; index < projection.scope_origins.size(); ++index) {
        const auto& origin = projection.scope_origins[index];
        out << "    {\"scope_id\": " << origin.scope_id.value
            << ", \"ast_path\": ";
        dump_json_string(out, origin.ast_path);
        out << ", ";
        dump_origin_descriptor(out, origin.descriptor);
        out << '}';
        if (index + 1 < projection.scope_origins.size()) {
            out << ',';
        }
        out << '\n';
    }

    out << "  ],\n  \"graph_syntax\": {\"format\":\"flowmini.graph_syntax\",\"version\":1,\"nodes\":[";
    auto provenance = [&](const SourceLocation& location) {
        const auto index = location.line ? location.line - 1 : lineOrigins.size();
        const bool mapped = index < lineOrigins.size();
        out << "{\"source\":";
        dump_json_string(out, mapped ? std::string_view(lineOrigins[index].source_path) : sourcePath);
        out << ",\"line\":" << (mapped ? lineOrigins[index].source_line : location.line)
            << ",\"column\":" << location.column << '}';
    };
    for (std::size_t i = 0; i < module.graph_nodes.size(); ++i) {
        const auto& node = module.graph_nodes[i];
        if (i) out << ',';
        out << "{\"node_id\":"; dump_json_string(out, node.name);
        out << ",\"role\":"; dump_json_string(out, node.role);
        out << ",\"implementation_kind\":"; dump_json_string(out, node.source_function ? "source_function" : "provider_atom");
        out << ",\"implementation_name\":"; dump_json_string(out, node.implementation);
        if (node.persistent) out << ",\"persistent\":true";
        out << ",\"provenance\":"; provenance(node.location);
        out << '}';
    }
    out << "],\"wires\":[";
    auto endpoint = [&](const GraphEndpointSyntax& value) {
        out << "{\"node_id\":"; dump_json_string(out, value.node);
        out << ",\"port_id\":"; dump_json_string(out, value.port);
        out << ",\"provenance\":"; provenance(value.location);
        out << '}';
    };
    for (std::size_t i = 0; i < module.graph_wires.size(); ++i) {
        const auto& wire = module.graph_wires[i];
        if (i) out << ',';
        out << "{\"wire_id\":"; dump_json_string(out, "wire:" + std::to_string(i));
        out << ",\"from\":"; endpoint(wire.from);
        out << ",\"to\":"; endpoint(wire.to);
        out << ",\"provenance\":"; provenance(wire.location);
        out << '}';
    }
    out << "],\"policies\":[";
    for (std::size_t i = 0; i < module.graph_policies.size(); ++i) {
        const auto& policy = module.graph_policies[i];
        if (i) out << ',';
        out << "{\"node_id\":"; dump_json_string(out, policy.node);
        out << ",\"key\":"; dump_json_string(out, policy.key);
        out << ",\"value_kind\":"; dump_json_string(out, policy.value_kind);
        out << ",\"value_text\":"; dump_json_string(out, policy.value_text);
        out << ",\"provenance\":"; provenance(policy.location);
        out << '}';
    }
    out << "],\"states\":[";
    for (std::size_t i = 0; i < module.graph_states.size(); ++i) {
        const auto& state = module.graph_states[i];
        if (i) out << ',';
        out << "{\"node_id\":"; dump_json_string(out, state.node);
        out << ",\"type\":"; dump_json_string(out, state.type);
        out << ",\"value_text\":"; dump_json_string(out, state.value_text);
        out << ",\"provenance\":"; provenance(state.location);
        out << '}';
    }
    out << "]},\n  \"diagnostics\": [";
    for (std::size_t index = 0; index < module.unsupported_graph_locations.size(); ++index) {
        const auto& location = module.unsupported_graph_locations[index];
        if (index) out << ',';
        out << "{\"code\":\"FLOWMINI_GRAPH_LOWERING_UNSUPPORTED\",\"severity\":\"error\","
               "\"message\":\"graph syntax is interpreter-only; no structured graph lowering contract is implemented\","
               "\"provenance\":{\"source\":";
        const auto line_index = static_cast<std::size_t>(location.line - 1);
        const bool mapped = location.line > 0 && line_index < lineOrigins.size();
        dump_json_string(out, mapped ? std::string_view(lineOrigins[line_index].source_path) : sourcePath);
        out << ",\"line\":" << (mapped ? lineOrigins[line_index].source_line : location.line)
            << ",\"column\":" << location.column << "}}";
    }
    out << "]\n}\n";
}

} // namespace flowmini::ast
