#include "flow_common.h"
#include "flowmini_lexer.h"
#include "flowmini_parser.h"
#include "flowmini_ast.h"
#include "flowmini_runtime.h"
#include "flowmini_structural.h"
#include "flowmini_token_tree_bridge.h"
#include <flowcontracts/bounded_input.hpp>
#include <flowcontracts/diagnostics.hpp>

#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <new>
#include <sstream>
#include <string>
#include <string_view>
#include <sys/file.h>
#include <unistd.h>
#include <vector>

#include "flowmini_ast_builder.h"
#include "flowmini_frontend_bundle.h"
#include "flowmini_symbol_projection.h"

namespace {

    struct OutputError : std::runtime_error { using std::runtime_error::runtime_error; };
    struct OutputUncertain : std::runtime_error { using std::runtime_error::runtime_error; };

    [[nodiscard]] std::string readFile(const std::string& path) {
        std::ifstream input{path};
        if (!input) {
            throw flow::DiagnosticError{"cli", "could not open source file: " + path};
        }

        try {
            return flowcontracts::read_bounded(input, "source file");
        } catch (const std::exception& error) {
            throw flow::DiagnosticError{"source", error.what()};
        }
    }

    [[nodiscard]] auto trimCopy(const std::string& value) -> std::string {
        const auto first = value.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) { return {}; }
        const auto last = value.find_last_not_of(" \t\r\n");
        return value.substr(first, last - first + 1);
    }

    [[nodiscard]] bool startsWithWord(const std::string& text, const std::string& word) {
        if (text.rfind(word, 0) != 0) { return false; }
        if (text.size() == word.size()) { return true; }
        const char next = text.at(word.size());
        return next == ' ' || next == '\t' || next == '{' || next == '(';
    }

    [[nodiscard]] std::string maskCommentsForImportScanner(const std::string& source) {
        enum class Mode { Code, String, LineComment, BlockComment };

        std::string out;
        out.reserve(source.size());

        Mode mode = Mode::Code;
        std::size_t depth = 0;
        int line = 1;
        int column = 1;
        int blockLine = 1;
        int blockColumn = 1;

        for (std::size_t i = 0; i < source.size();) {
            const char c = source[i];
            const char next = (i + 1 < source.size()) ? source[i + 1] : '\0';

            if (mode == Mode::Code) {
                if (c == '"') {
                    mode = Mode::String;
                    out.push_back(c);
                    ++i;
                    ++column;
                    continue;
                }
                if (c == '/' && next == '/') {
                    mode = Mode::LineComment;
                    out.append("  ");
                    i += 2;
                    column += 2;
                    continue;
                }
                if (c == '/' && next == '*') {
                    mode = Mode::BlockComment;
                    depth = 1;
                    blockLine = line;
                    blockColumn = column;
                    out.append("  ");
                    i += 2;
                    column += 2;
                    continue;
                }
                out.push_back(c);
                if (c == '\n') { ++line; column = 1; }
                else { ++column; }
                ++i;
                continue;
            }

            if (mode == Mode::String) {
                out.push_back(c);
                if (c == '\\' && i + 1 < source.size()) {
                    out.push_back(source[i + 1]);
                    i += 2;
                    column += 2;
                    continue;
                }
                if (c == '"') { mode = Mode::Code; }
                if (c == '\n') { ++line; column = 1; }
                else { ++column; }
                ++i;
                continue;
            }

            if (mode == Mode::LineComment) {
                if (c == '\n') {
                    mode = Mode::Code;
                    out.push_back('\n');
                    ++i;
                    ++line;
                    column = 1;
                    continue;
                }
                out.push_back(' ');
                ++i;
                ++column;
                continue;
            }

            //if (mode == Mode::BlockComment) {
                if (c == '/' && next == '*') {
                    ++depth;
                    out.append("  ");
                    i += 2;
                    column += 2;
                    continue;
                }
                if (c == '*' && next == '/') {
                    --depth;
                    out.append("  ");
                    i += 2;
                    column += 2;
                    if (depth == 0) { mode = Mode::Code; }
                    continue;
                }
                if (c == '\n') {
                    out.push_back('\n');
                    ++i;
                    ++line;
                    column = 1;
                    continue;
                }
                out.push_back(' ');
                ++i;
                ++column;
              //  continue;
            //}
        }

        if (mode == Mode::BlockComment) {
            throw flow::DiagnosticError{
                "import",
                "unterminated block comment starting at line " + std::to_string(blockLine) +
                ", column " + std::to_string(blockColumn)
            };
        }

        return out;
    }

    struct ParsedImport final {
        std::string path;
        std::string alias;
    };

    [[nodiscard]] bool parseImportLine(const std::string& line, ParsedImport& parsed) {
        const std::string t = trimCopy(line);
        if (!startsWithWord(t, "import")) { return false; }

        std::size_t i = std::string{"import"}.size();
        while (i < t.size() && (t[i] == ' ' || t[i] == '\t')) { ++i; }
        if (i >= t.size() || t[i] != '"') {
            throw flow::DiagnosticError{"import", "expected quoted path after import"};
        }
        ++i;

        std::string path;
        bool escape = false;
        for (; i < t.size(); ++i) {
            const char c = t[i];
            if (escape) { path.push_back(c); escape = false; continue; }
            if (c == '\\') { escape = true; continue; }
            if (c == '"') { ++i; break; }
            path.push_back(c);
        }
        if (path.empty()) { throw flow::DiagnosticError{"import", "empty import path"}; }
        while (i < t.size() && (t[i] == ' ' || t[i] == '\t')) { ++i; }
        if (i != t.size()) {
            constexpr std::string_view asWord = "as";
            if (t.compare(i, asWord.size(), asWord) != 0 ||
                (i + asWord.size() < t.size() &&
                 std::isalnum(static_cast<unsigned char>(t[i + asWord.size()])))) {
                throw flow::DiagnosticError{"import", "unexpected text after import path: " + t.substr(i)};
            }
            i += asWord.size();
            while (i < t.size() && (t[i] == ' ' || t[i] == '\t')) { ++i; }
            const std::size_t aliasStart = i;
            if (i >= t.size() || !(std::isalpha(static_cast<unsigned char>(t[i])) || t[i] == '_')) {
                throw flow::DiagnosticError{"import", "expected identifier after import 'as'"};
            }
            ++i;
            while (i < t.size() && (std::isalnum(static_cast<unsigned char>(t[i])) || t[i] == '_')) { ++i; }
            parsed.alias = t.substr(aliasStart, i - aliasStart);
            while (i < t.size() && (t[i] == ' ' || t[i] == '\t')) { ++i; }
            if (i != t.size()) { throw flow::DiagnosticError{"import", "unexpected text after import alias: " + t.substr(i)}; }
        }
        parsed.path = std::move(path);
        return true;
    }

    [[nodiscard]] std::string namespaceAbiHeader(const std::string& line, const std::string& alias) {
        if (alias.empty()) { return line; }
        const std::string trimmed = trimCopy(line);
        if (!startsWithWord(trimmed, "abi")) { return line; }
        std::size_t nameStart = 3;
        while (nameStart < trimmed.size() && (trimmed[nameStart] == ' ' || trimmed[nameStart] == '\t')) { ++nameStart; }
        const std::size_t nameEnd = trimmed.find_first_of(" \t{", nameStart);
        if (nameStart == trimmed.size() || nameEnd == std::string::npos) { return line; }
        const std::size_t indent = line.find_first_not_of(" \t");
        return line.substr(0, indent == std::string::npos ? 0 : indent) +
               "abi " + alias + line.substr(nameEnd);
    }

    [[nodiscard]] std::filesystem::path canonicalImportPath(const std::filesystem::path& importerPath, const std::string& importText) {
        std::filesystem::path raw{importText};
        if (raw.is_relative()) {
            raw = importerPath.parent_path() / raw;
        }
        return std::filesystem::weakly_canonical(std::filesystem::absolute(raw));
    }


    enum class ImportState { Loading, Loaded };
    enum class SourceUnitKind { None, LegacyModule, Program, Unit };

    struct ExpandedSourceLine final {
        std::string text;
        flowmini::ast::FrontendSourceLineOrigin origin;
    };

    struct ExpandedSource final {
        std::vector<ExpandedSourceLine> lines;

        [[nodiscard]] std::string render() const {
            std::ostringstream out;
            for (const auto& line : lines) {
                out << line.text << '\n';
            }
            return out.str();
        }

        [[nodiscard]] std::vector<flowmini::ast::FrontendSourceLineOrigin> origins() const {
            std::vector<flowmini::ast::FrontendSourceLineOrigin> result;
            result.reserve(lines.size());
            for (const auto& line : lines) {
                result.push_back(line.origin);
            }
            return result;
        }

        void append(ExpandedSource source) {
            lines.insert(lines.end(),
                         std::make_move_iterator(source.lines.begin()),
                         std::make_move_iterator(source.lines.end()));
        }

        void appendGeneratedBlank() {
            lines.push_back(ExpandedSourceLine{"", {}});
        }
    };

    [[nodiscard]] std::string sourceDisplayPath(const std::filesystem::path& path) {
        const auto absolute = std::filesystem::weakly_canonical(std::filesystem::absolute(path));
        std::error_code error;
        const auto relative = std::filesystem::relative(absolute,
                                                        std::filesystem::current_path(),
                                                        error);
        if (!error && !relative.empty()) {
            return relative.generic_string();
        }
        return absolute.generic_string();
    }

    [[nodiscard]] ExpandedSource directExpandedSource(const std::string& sourcePath,
                                                      const std::string& source) {
        ExpandedSource expanded;
        const auto displayPath = sourceDisplayPath(sourcePath);
        std::istringstream input{source};
        std::string line;
        std::size_t lineNumber = 1;
        while (std::getline(input, line)) {
            expanded.lines.push_back(ExpandedSourceLine{
                std::move(line),
                flowmini::ast::FrontendSourceLineOrigin{displayPath, lineNumber},
            });
            ++lineNumber;
        }
        return expanded;
    }

    [[nodiscard]] SourceUnitKind detectUnitKind(const std::string& trimmed) {
        if (startsWithWord(trimmed, "program")) { return SourceUnitKind::Program; }
        if (startsWithWord(trimmed, "unit")) { return SourceUnitKind::Unit; }
        if (startsWithWord(trimmed, "module")) { return SourceUnitKind::LegacyModule; }
        return SourceUnitKind::None;
    }

    struct ImportExpander {
        explicit ImportExpander(const bool preserveImports)
            : preserveImportDeclarations{preserveImports} {}

        bool preserveImportDeclarations {false};
        std::map<std::string, ImportState> states;
        std::vector<std::string> stack;

        [[nodiscard]] ExpandedSource expandRoot(const std::string& sourcePath,
                                                const bool allowUnitRoot) {
            const auto rootPath = std::filesystem::weakly_canonical(std::filesystem::absolute(std::filesystem::path{sourcePath}));
            const std::string source = readFile(rootPath.string());
            const std::string scanSource = maskCommentsForImportScanner(source);
            std::istringstream input{source};
            std::istringstream scanInput{scanSource};

            std::vector<ExpandedSource> importExpansions;
            std::vector<ExpandedSourceLine> importDeclarations;
            std::vector<ExpandedSourceLine> rootBodyLines;
            ExpandedSourceLine headerLine;
            SourceUnitKind rootKind = SourceUnitKind::None;
            bool sawHeader = false;
            bool sawMain = false;
            bool sawTarget = false;
            int braceDepth = 0;
            int targetBaseDepth = -1;
            std::string line;
            std::size_t lineNumber = 1;
            const auto rootDisplayPath = sourceDisplayPath(rootPath);

            std::string scanLine;
            while (std::getline(input, line) && std::getline(scanInput, scanLine)) {
                ParsedImport importSpec;
                const std::string trimmed = trimCopy(scanLine);
                if (parseImportLine(scanLine, importSpec)) {
                    if (sawHeader) {
                        throw flow::DiagnosticError{"import", "import statements must appear before program/unit declaration in: " + rootPath.string()};
                    }
                    if (preserveImportDeclarations) {
                        importDeclarations.push_back(ExpandedSourceLine{
                            line,
                            flowmini::ast::FrontendSourceLineOrigin{
                                rootDisplayPath,
                                lineNumber,
                            },
                        });
                    }
                    importExpansions.push_back(expandLibrary(canonicalImportPath(rootPath, importSpec.path), importSpec.alias));
                    ++lineNumber;
                    continue;
                }

                const SourceUnitKind kind = detectUnitKind(trimmed);
                if (kind != SourceUnitKind::None) {
                    if (sawHeader) { throw flow::DiagnosticError{"import", "multiple program/unit declarations in: " + rootPath.string()}; }
                    headerLine = ExpandedSourceLine{
                        line,
                        flowmini::ast::FrontendSourceLineOrigin{rootDisplayPath, lineNumber},
                    };
                    rootKind = kind;
                    sawHeader = true;
                    ++lineNumber;
                    continue;
                }

                const bool insideTarget = targetBaseDepth >= 0 && braceDepth > targetBaseDepth;
                if (startsWithWord(trimmed, "target")) {
                    sawTarget = true;
                    if (trimmed.find('{') != std::string::npos) {
                        targetBaseDepth = braceDepth;
                    }
                }

                if (startsWithWord(trimmed, "main") && !insideTarget) {
                    if (sawMain) { throw flow::DiagnosticError{"import", "multiple main definitions found in root file: " + rootPath.string()}; }
                    sawMain = true;
                }

                if (!trimmed.empty() || sawHeader) {
                    rootBodyLines.push_back(ExpandedSourceLine{
                        line,
                        flowmini::ast::FrontendSourceLineOrigin{rootDisplayPath, lineNumber},
                    });
                }
                for (const char c : scanLine) {
                    if (c == '{') { ++braceDepth; }
                    else if (c == '}' && braceDepth > 0) { --braceDepth; }
                }
                if (targetBaseDepth >= 0 && braceDepth <= targetBaseDepth) {
                    targetBaseDepth = -1;
                }
                ++lineNumber;
            }

            if (!sawHeader) { throw flow::DiagnosticError{"import", "root source has no program/unit declaration: " + rootPath.string()}; }
            if (rootKind == SourceUnitKind::Unit && !allowUnitRoot) {
                throw flow::DiagnosticError{"import", "root source is a unit; units are defining/importable units and cannot be executed directly: " + rootPath.string()};
            }
            if (rootKind != SourceUnitKind::Unit && !sawMain && !sawTarget) {
                throw flow::DiagnosticError{"import", "root program has no main block: " + rootPath.string()};
            }

            ExpandedSource result;
            result.lines.push_back(std::move(headerLine));
            result.appendGeneratedBlank();
            result.lines.insert(result.lines.end(),
                                std::make_move_iterator(importDeclarations.begin()),
                                std::make_move_iterator(importDeclarations.end()));
            if (!importDeclarations.empty()) {
                result.appendGeneratedBlank();
            }
            for (auto& expanded : importExpansions) {
                if (!expanded.lines.empty()) {
                    result.append(std::move(expanded));
                    result.appendGeneratedBlank();
                }
            }
            result.lines.insert(result.lines.end(),
                                std::make_move_iterator(rootBodyLines.begin()),
                                std::make_move_iterator(rootBodyLines.end()));
            return result;
        }

        [[nodiscard]] ExpandedSource expandLibrary(const std::filesystem::path& libPath,
                                                   const std::string& namespaceAlias = {}) {
            const std::string key = libPath.string();
            const auto found = states.find(key);
            if (found != states.end()) {
                if (found->second == ImportState::Loaded) { return ExpandedSource{}; }

                std::ostringstream cycle;
                cycle << "import cycle detected: ";
                const auto pos = std::ranges::find(stack, key);
                if (pos != stack.end()) {
                    for (auto it = pos; it != stack.end(); ++it) { cycle << *it << " -> "; }
                }
                cycle << key;
                throw flow::DiagnosticError{"import", cycle.str()};
            }

            states[key] = ImportState::Loading;
            stack.push_back(key);

            const std::string source = readFile(key);
            const std::string scanSource = maskCommentsForImportScanner(source);
            std::istringstream input{source};
            std::istringstream scanInput{scanSource};
            std::vector<ExpandedSource> importExpansions;
            std::vector<ExpandedSourceLine> importDeclarations;
            std::vector<ExpandedSourceLine> bodyLines;
            SourceUnitKind libKind = SourceUnitKind::None;
            bool sawHeader = false;
            std::string line;
            std::size_t lineNumber = 1;
            const auto displayPath = sourceDisplayPath(libPath);

            std::string scanLine;
            while (std::getline(input, line) && std::getline(scanInput, scanLine)) {
                ParsedImport importSpec;
                const std::string trimmed = trimCopy(scanLine);

                if (parseImportLine(scanLine, importSpec)) {
                    if (sawHeader) {
                        throw flow::DiagnosticError{"import", "import statements must appear before unit declaration in imported file: " + key};
                    }
                    if (preserveImportDeclarations) {
                        importDeclarations.push_back(ExpandedSourceLine{
                            line,
                            flowmini::ast::FrontendSourceLineOrigin{displayPath, lineNumber},
                        });
                    }
                    importExpansions.push_back(expandLibrary(canonicalImportPath(libPath, importSpec.path), importSpec.alias));
                    ++lineNumber;
                    continue;
                }

                const SourceUnitKind kind = detectUnitKind(trimmed);
                if (kind != SourceUnitKind::None) {
                    if (sawHeader) { throw flow::DiagnosticError{"import", "multiple program/unit declarations in imported file: " + key}; }
                    libKind = kind;
                    sawHeader = true;
                    ++lineNumber;
                    continue;
                }

                if (startsWithWord(trimmed, "main")) {
                    throw flow::DiagnosticError{"import", "imported file defines main; only unit files may be imported: " + key};
                }

                if (!trimmed.empty() || sawHeader) {
                    bodyLines.push_back(ExpandedSourceLine{
                        namespaceAbiHeader(line, namespaceAlias),
                        flowmini::ast::FrontendSourceLineOrigin{displayPath, lineNumber},
                    });
                }
                ++lineNumber;
            }

            if (!sawHeader) { throw flow::DiagnosticError{"import", "imported file has no unit declaration: " + key}; }
            if (libKind == SourceUnitKind::Program) {
                throw flow::DiagnosticError{"import", "imported file is a program; only unit files may be imported: " + key};
            }

            ExpandedSource result;
            result.lines.insert(result.lines.end(),
                                std::make_move_iterator(importDeclarations.begin()),
                                std::make_move_iterator(importDeclarations.end()));
            if (!importDeclarations.empty()) {
                result.appendGeneratedBlank();
            }
            for (auto& expanded : importExpansions) {
                if (!expanded.lines.empty()) {
                    result.append(std::move(expanded));
                    result.appendGeneratedBlank();
                }
            }
            result.lines.insert(result.lines.end(),
                                std::make_move_iterator(bodyLines.begin()),
                                std::make_move_iterator(bodyLines.end()));

            stack.pop_back();
            states[key] = ImportState::Loaded;
            return result;
        }
    };

    [[nodiscard]] ExpandedSource expandImports(const std::string& sourcePath,
                                               const bool allowUnitRoot = false,
                                               const bool preserveImportDeclarations = false) {
        ImportExpander expander{preserveImportDeclarations};
        return expander.expandRoot(sourcePath, allowUnitRoot);
    }

    void printUsage(std::ostream& out) {
        out
            << "Usage:\n"
            << "  flowmini [--trace true|false] [--diagnostics json] [--emit-flowir <file|->] [--dump-token-tree <file|->] [--dump-token-tree-bridge [json|simple]] [--dump-ast] [--dump-frontend-bundle] [--dump-ast-symbols <file|->] [--dump-symbols <file|->] <program.flow|module.flowir> < input\n\n"
            << "Human .flow sugar examples:\n"
            << "  program demo\n"
            << "  stdin : stdin.text()\n"
            << "  one : int(1)\n"
            << "  arr : list<int>([1,2,3])\n"
            << "  add : int.add(lhs=\"sum\", rhs=\"current\", out=\"sum\")\n"
            << "  stdin.out => parse.in => init.in\n\n"
            << "Explicit .flowir remains valid:\n"
            << "  producer <id> : <atom.kind>\n"
            << "  node <id> : <atom.kind>\n"
            << "  sink <id> : <atom.kind>\n"
            << "  wire <from_node>.<from_port> => <to_node>.<to_port>\n"
            << "  policy <node>.<key> = <string|int|bool>\n";
    }

    void writeAtomicFile(const std::string& path, std::string_view contents) {
        const auto slash = path.find_last_of('/');
        const std::string parent = slash == std::string::npos ? "." :
                                   slash == 0 ? "/" : path.substr(0, slash);
        const std::string leaf = slash == std::string::npos ? path : path.substr(slash + 1);
        if (leaf.empty()) throw OutputError{"output path has no file name"};
        const std::string temporary = leaf + ".tmp.lyraform-v1";
        const int directory = open(parent.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
        if (directory < 0) throw OutputError{"cannot open output directory"};
        if (flock(directory, LOCK_EX) != 0) {
            close(directory);
            throw OutputError{"cannot lock output directory"};
        }
        if (unlinkat(directory, temporary.c_str(), 0) != 0 && errno != ENOENT) {
            close(directory);
            throw OutputError{"cannot remove stale private output file"};
        }
        const int descriptor = openat(directory, temporary.c_str(),
                                      O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
        if (descriptor < 0) {
            close(directory);
            throw OutputError{"cannot create private output file"};
        }
        FILE* file = fdopen(descriptor, "wb");
        bool published = false;
        if (file) {
            bool complete = std::fwrite(contents.data(), 1, contents.size(), file) == contents.size();
            if (complete && std::fflush(file) != 0) complete = false;
            if (complete && fsync(descriptor) != 0) complete = false;
            if (std::fclose(file) != 0) complete = false;
            if (complete && renameat(directory, temporary.c_str(), directory, leaf.c_str()) == 0) published = true;
        } else {
            close(descriptor);
        }
        if (!published) {
            unlinkat(directory, temporary.c_str(), 0);
            close(directory);
            throw OutputError{"cannot publish output file"};
        }
        bool durable = fsync(directory) == 0;
        if (close(directory) != 0) durable = false;
        if (!durable)
            throw OutputUncertain{"output file is published but parent directory durability is uncertain"};
    }

    template <typename Writer>
    void writeOutputFile(const std::string& path, Writer&& writer) {
        if (path == "-") {
            writer(std::cout);
            return;
        }
        std::ostringstream rendered;
        writer(rendered);
        writeAtomicFile(path, rendered.str());
    }

    void writeStructuredFailure(
        std::string_view code,
        std::string_view stage,
        std::string_view message,
        std::string_view disposition = "no_artifact"
    ) noexcept {
        std::fputs("{\"status\":\"failed\",\"code\":\"", stderr);
        flowcontracts::write_json_string(stderr, code);
        std::fputs("\",\"stage\":\"", stderr);
        flowcontracts::write_json_string(stderr, stage);
        std::fputs("\",\"message\":\"", stderr);
        flowcontracts::write_json_string(stderr, message);
        std::fputs("\",\"disposition\":\"", stderr);
        flowcontracts::write_json_string(stderr, disposition);
        std::fputs("\"}\n", stderr);
    }

} // namespace

/**
 *
 * @param argc
 * @param argv
 * @return
 */

int main(int argc, char** argv) {

    flow::PipelineContext ctx;
    const flow::LogSink log{std::cerr};
    bool structuredDiagnostics = false;

    try {
        for (int index = 1; index + 1 < argc; ++index) {
            if (std::strcmp(argv[index], "--diagnostics") == 0 &&
                std::strcmp(argv[index + 1], "json") == 0) {
                structuredDiagnostics = true;
            }
        }

#ifdef FLOWMINI_TEST_ALLOCATION_FAILURE
        throw std::bad_alloc();
#endif

        ctx.policies.set("runtime.trace", false);

        std::string sourcePath;
        std::string emitFlowIrPath;
        std::string dumpTokenTreePath;

        bool dumpTokenTreeBridge = false;
        bool dumpAst = false;
        bool dumpFrontendBundle = false;

        flowmini::TokenTreeBridgeDumpFormat dumpTokenTreeBridgeFormat = flowmini::TokenTreeBridgeDumpFormat::Json;
        std::string dumpSymbolsPath;
        std::string dumpAstSymbolsPath;
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];

            if (arg == "--help" || arg == "-h") {
                printUsage(std::cout);
                return 0;
            }

            if (arg == "--diagnostics") {
                const auto format = flow::requireArgValue(argc, argv, i, arg);
                if (format != "json") {
                    throw flow::DiagnosticError{"cli", "unsupported diagnostics format: " + format};
                }
                structuredDiagnostics = true;
                continue;
            }

            if (arg == "--trace") {
                ctx.policies.set("runtime.trace", flow::parseBool(flow::requireArgValue(argc, argv, i, arg)));
                continue;
            }

            if (arg == "--dump-ast") {
                dumpAst = true;
                continue;
            }

            if (arg == "--dump-frontend-bundle") {
                dumpFrontendBundle = true;
                continue;
            }

            if (arg == "--emit-flowir") {
                emitFlowIrPath = flow::requireArgValue(argc, argv, i, arg);
                continue;
            }

            if (arg == "--dump-token-tree") {
                dumpTokenTreePath = flow::requireArgValue(argc, argv, i, arg);
                continue;
            }

            if (arg == "--dump-token-tree-bridge") {
                dumpTokenTreeBridge = true;
                if (i + 1 < argc) {
                    const std::string next = argv[i + 1];
                    if (next == "json" || next == "simple") {
                        dumpTokenTreeBridgeFormat = flowmini::parseTokenTreeBridgeDumpFormat(next);
                        ++i;
                    }
                }
                continue;
            }

            if (arg == "--dump-ast-symbols") {
                dumpAstSymbolsPath = flow::requireArgValue(argc, argv, i, arg);
                continue;
            }

            if (arg == "--dump-symbols") {
                dumpSymbolsPath = flow::requireArgValue(argc, argv, i, arg);
                continue;
            }


            if (!sourcePath.empty()) {
                throw flow::DiagnosticError{"cli", "unexpected extra argument: " + arg};
            }

            sourcePath = arg;
        }

        if (sourcePath.empty()) {
            printUsage(std::cerr);
            return 2;
        }

        const bool structuralInspection =
            dumpAst ||
            dumpFrontendBundle ||
            !dumpAstSymbolsPath.empty() ||
            !dumpTokenTreePath.empty() ||
            dumpTokenTreeBridge;
        const bool preserveImportDeclarations =
            dumpAst || dumpFrontendBundle || !dumpAstSymbolsPath.empty();

        const std::filesystem::path inputPath{sourcePath};
        const auto expandedSource = (inputPath.extension() == ".flowir")
            ? directExpandedSource(sourcePath, readFile(sourcePath))
            : expandImports(sourcePath,
                            structuralInspection,
                            preserveImportDeclarations);
        const std::string source = expandedSource.render();
        const auto tokens = flowmini::lexSource(source);

        if (dumpAst) {
            const auto module = flowmini::ast::build_source_header_ast(tokens);
            flowmini::ast::dump_ast_json(std::cout, module);
            return 0;
        }

        if (dumpFrontendBundle) {
            const auto module = flowmini::ast::build_source_header_ast(tokens);
            const auto projection = flowmini::ast::build_symbol_projection(module);
            flowmini::ast::dump_frontend_bundle_json(std::cout,
                                                     module,
                                                     projection,
                                                     sourcePath,
                                                     expandedSource.origins());
            return 0;
        }

        if (!dumpAstSymbolsPath.empty()) {
            const auto module = flowmini::ast::build_source_header_ast(tokens);
            auto table = flowmini::ast::build_symbol_table_projection(module);
            writeOutputFile(dumpAstSymbolsPath, [&](std::ostream& out) { table.dump(out); });
            return 0;
        }

        if (!dumpTokenTreePath.empty()) {
            writeOutputFile(dumpTokenTreePath, [&](std::ostream& out) { flowmini::writeTokenTreeDump(tokens, out); });
            return 0;
        }

        if (dumpTokenTreeBridge) {
            flowmini::writeTokenTreeBridgeDump(tokens, dumpTokenTreeBridgeFormat, std::cout);
            return 0;
        }

        const auto module = flowmini::parseModule(tokens);

        if (!dumpSymbolsPath.empty()) {
            writeOutputFile(dumpSymbolsPath, [&](std::ostream& out) { flowmini::writeFlowIrSymbolTableDump(module, out); });
            return 0;
        }

        if (!emitFlowIrPath.empty()) {
            writeOutputFile(emitFlowIrPath, [&](std::ostream& out) { flowmini::writeFlowIr(module, out); });
            return 0;
        }

        auto registry = flowmini::makeCoreAtomRegistry();
        const auto runtime = flowmini::runModuleChecked(module, ctx, registry);
        if (!runtime.completed) {
            if (structuredDiagnostics) {
                writeStructuredFailure(runtime.code, runtime.stage, runtime.message);
            } else {
                std::cerr << "fatal in " << runtime.stage << ": " << runtime.message << '\n';
                log.write(ctx);
            }
            return 1;
        }
        log.write(ctx);
        return 0;

    } catch (const flow::DiagnosticError& err) {
        if (structuredDiagnostics) {
            writeStructuredFailure(err.code(), err.stage(), err.what());
        } else {
            log.writeFatal(err);
            log.write(ctx);
        }
        return 1;
    } catch (const std::bad_alloc&) {
        if (structuredDiagnostics) {
            writeStructuredFailure("FLOW_RESOURCE_EXHAUSTED", "runtime", "allocation failed");
        } else {
            std::cerr << "fatal in runtime: allocation failed\n";
            log.write(ctx);
        }
        return 1;
    } catch (const OutputError& err) {
        if (structuredDiagnostics) {
            writeStructuredFailure("FLOW_OUTPUT_FAILURE", "output", err.what());
        } else {
            std::cerr << "fatal in output: " << err.what() << '\n';
            log.write(ctx);
        }
        return 1;
    } catch (const OutputUncertain& err) {
        if (structuredDiagnostics) {
            writeStructuredFailure("FLOW_OUTPUT_DURABILITY_UNCERTAIN", "output", err.what(),
                                   "artifact_published_durability_uncertain");
        } else {
            std::cerr << "fatal in output: " << err.what() << '\n';
            log.write(ctx);
        }
        return 1;
    } catch (const std::exception& err) {
        if (structuredDiagnostics) {
            writeStructuredFailure("FLOW_UNEXPECTED_EXCEPTION", "runtime", err.what());
        } else {
            std::cerr << "fatal in unknown: " << err.what() << '\n';
            log.write(ctx);
        }
        return 1;
    } catch (...) {
        if (structuredDiagnostics) {
            writeStructuredFailure("FLOW_UNKNOWN_FAILURE", "runtime", "unknown non-standard failure");
        } else {
            std::cerr << "fatal in unknown: unknown non-standard failure\n";
            log.write(ctx);
        }
        return 1;
    }
}
