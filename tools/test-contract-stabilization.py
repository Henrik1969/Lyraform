#!/usr/bin/env python3
"""Source and hostile-artifact tests for existing refinement/division admission."""
import argparse
import copy
import json
import pathlib
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument("--build", type=pathlib.Path, required=True)
args = parser.parse_args()
build = args.build.resolve()
tools = {name: build / path for name, path in {
    "frontend": "flowmini/flowmini", "analyze": "flowanalyst/flowanalyst",
    "bind": "flowbind/flowbind", "parallel": "flowtools/flowparallel/flowparallel",
    "optimize": "flowoptimize/flowoptimize", "prepare": "flowlower/flowprepare",
    "lower": "flowlower/flowlower", "validate": "flowcontracts/flowvalidate",
    "tiny_lower": "tinyvm/flowtinylower", "tiny_run": "tinyvm/flowtinyrun",
}.items()}


def run(command, data=None):
    return subprocess.run(list(map(str, command)), input=data, text=True,
                          capture_output=True, timeout=30)


def good(command, data=None):
    result = run(command, data)
    assert result.returncode == 0, (command, result.returncode, result.stdout, result.stderr)
    return result.stdout


def program(body, declarations=""):
    return "program stabilization\n" + declarations + "\nmain {\n" + body + "\n}\n"


def record_program(expression):
    return ("program stabilization\nrecord Box { value : int\n}\n"
            "target Demo {\nmain {\nitem : Box({value: " + expression +
            "})\nx : int(item.value)\nx -> return\n}\n}\n")


positive = [
    ("record_division", record_program("6 / 2"), 3),
    ("argument_division", program("f(6 / 2) -> return", "fn f(n : int): int {\nn -> return\n}"), 3),
    ("literal", program("6 / 2 -> return"), 3),
    ("local", program("a : int(6)\nb : int(2)\na / b -> return"), 3),
    ("signed", program("a : int(6)\nb : int(-2)\na / b -> return"), -3),
    ("minus_one", program("a : int(6)\na / -1 -> return"), -6),
    ("function", program("half(8) -> return", "fn half(n : int): int {\n n / 2 -> return\n}"), 4),
    ("unchanged_scalar", program("a : int(40)\na + 2 -> a\na -> return"), 42),
    ("static_guard", program("a : int(4)\nguard positive : a > 0\n5 -> a\na -> return"), 5),
]
refinement = "type Positive refines int {\n invariant value > 0\n}\n"
negative = [
    ("record_zero", record_program("1 / 0"), "INTEGER_ADMISSION"),
    ("record_overflow", record_program("-2147483648 / -1"), "INTEGER_ADMISSION"),
    ("argument_zero", program("f(1 / 0) -> return", "fn f(n : int): int {\nn -> return\n}"), "INTEGER_ADMISSION"),
    ("refined_placement", program("x : Positive(1)\n-1 -> x\n0 -> return", refinement), "REFINED_VALUE"),
    ("refined_construct", program("x : Positive(-1)\nx -> return", refinement), "REFINED_VALUE"),
    ("refined_generic", program("x : list<Positive>(-1)\n0 -> return", refinement), "REFINED_VALUE"),
    ("refined_shadow", program("Positive : int(0)\nx : Positive(-1)\n0 -> return", refinement), "REFINED_VALUE"),
    ("refined_unused", program("x : Positive(-1)\n0 -> return", refinement), "REFINED_VALUE"),
    ("refined_argument", program("f(-1) -> return", refinement + "fn f(x : Positive): int {\nx -> return\n}"), "REFINED_VALUE"),
    ("refined_return", program("f() -> return", refinement + "fn f(): Positive {\n-1 -> return\n}"), "REFINED_VALUE"),
    ("refined_inherited", program("x : Derived(-1)\nx -> return", refinement + "type Derived refines Positive {\n}\n"), "REFINED_VALUE"),
    ("refined_field", program("0 -> return", refinement + "type Container {\nfield value : Positive\n}\n"), "REFINED_VALUE"),
    ("zero_literal", program("1 / 0 -> return"), "INTEGER_ADMISSION"),
    ("zero_local", program("a : int(1)\nb : int(0)\na / b -> return"), "INTEGER_ADMISSION"),
    ("zero_placed", program("b : int(2)\n0 -> b\n1 / b -> return"), "INTEGER_ADMISSION"),
    ("dynamic", program("f(0) -> return", "fn f(n : int): int {\n1 / n -> return\n}"), "INTEGER_ADMISSION"),
    ("carrier_minimum_overflow", program("a : c_int(-2147483648)\na / -1 -> return"), "INTEGER_ADMISSION"),
    ("unknown_minimum", program("f(1) -> return", "fn f(n : int): int {\nn / -1 -> return\n}"), "INTEGER_ADMISSION"),
    ("nested", program("x : int(2 + 1 / 0)\nx -> return"), "INTEGER_ADMISSION"),
    ("guard_violation", program("x : int(1)\nguard positive : x > 0\n-1 -> x\nx -> return"), "GUARD_TRANSITION"),
]

with tempfile.TemporaryDirectory(prefix="lyraform-stabilization-") as directory:
    temp = pathlib.Path(directory)
    semantic_for_attack = None
    for name, source, expected in positive:
        path = temp / (name + ".flow")
        path.write_text(source)
        target = ["--target", "Demo"] if "target Demo" in source else []
        frontend = good([tools["frontend"], "--dump-frontend-bundle", path])
        semantic = good([tools["analyze"], "--lowering-plan-version", "2"], frontend)
        assert semantic == good([tools["analyze"], "--lowering-plan-version", "2"], frontend)
        assert json.loads(semantic)["lowering_plan"]["status"] == "ready"
        if name == "local":
            semantic_for_attack = json.loads(semantic)
        good([tools["bind"]], semantic)
        parallel = good([tools["parallel"]], semantic)
        optimized = good([tools["optimize"]], parallel)
        llvm = temp / (name + ".ll")
        good([tools["lower"], *target, "--emit-llvm", llvm], optimized)
        # Preserve the complete signed result, rather than testing only exit's
        # low eight bits. This harness renames only the emitted entry symbol.
        full = temp / "entry.ll"
        full.write_text(llvm.read_text().replace("@main(", "@review_entry("))
        driver = temp / "driver.c"
        driver.write_text('#include <stdio.h>\nextern int review_entry(void);\nint main(void) { printf("%d\\n", review_entry()); return 0; }\n')
        for optimization in ("-O0", "-O2"):
            binary = temp / "native"
            good(["clang-18", optimization, full, driver, "-o", binary])
            assert int(good([binary]).strip()) == expected, (name, optimization)
        optimized_file = temp / "optimized.json"
        optimized_file.write_text(optimized)
        prepared = good([tools["prepare"], *target, optimized_file])
        tiny = temp / "program.tvm"
        good([tools["tiny_lower"], "-", tiny], prepared)
        observed = json.loads(good([tools["tiny_run"], tiny]))
        signed = observed["result"]
        if signed >= 1 << 63:
            signed -= 1 << 64
        assert observed["status"] == "completed" and signed == expected, (name, observed)

    for name, source, code in negative:
        path = temp / (name + ".flow")
        path.write_text(source)
        frontend = good([tools["frontend"], "--dump-frontend-bundle", path])
        analyzed = run([tools["analyze"], "--lowering-plan-version", "2"], frontend)
        assert analyzed.returncode == 2, (name, analyzed.stdout, analyzed.stderr)
        report = json.loads(analyzed.stdout)
        assert report["status"] == "error" and report["lowering_plan"]["status"] == "blocked", name
        assert any(code in item["code"] for item in report["diagnostics"]), (name, report["diagnostics"])
        if code == "INTEGER_ADMISSION":
            diagnostic = next(d for d in report["diagnostics"] if code in d["code"])
            origin = diagnostic["provenance"]
            assert origin["line"] > 0 and origin["column"] > 0, (name, origin)
            assert origin["ast_path"].startswith("/statement_pool/"), origin
            assert pathlib.Path(origin["source"]).name == path.name, origin
            assert diagnostic["operation_id"] >= 0


    # The immutable divisor declaration is the only literal 2 in this specimen.
    # Mutate the actual operand, not just its outer admission status.
    def zero_divisor(value):
        if isinstance(value, dict):
            if value.get("kind") == "integer_literal" and value.get("value") == "2":
                value["value"] = "0"
            for child in value.values():
                zero_divisor(child)
        elif isinstance(value, list):
            for child in value:
                zero_divisor(child)

    hostile_count = 0
    for remove_facts in (False, True):
        for version in (1, 2):
            hostile = copy.deepcopy(semantic_for_attack)
            plan = hostile["lowering_plan"]
            plan["version"] = version
            zero_divisor(plan)
            if remove_facts:
                for operation in plan["operations"]:
                    operation.pop("scalar_fact", None)
            encoded = json.dumps(hostile)
            for stage in ("validate", "bind", "parallel"):
                result = run([tools[stage]], encoded)
                assert result.returncode != 0, (stage, remove_facts, version)
                hostile_count += 1
    for status in (None, "blocked", "invalid"):
        hostile = copy.deepcopy(semantic_for_attack)
        zero_divisor(hostile["lowering_plan"])
        if status is None:
            hostile["lowering_plan"].pop("status", None)
        else:
            hostile["lowering_plan"]["status"] = status
        for stage in ("validate", "bind", "parallel"):
            result = run([tools[stage]], json.dumps(hostile))
            assert result.returncode != 0, (stage, status, "readiness bypass")
            hostile_count += 1
    # Mutate each captured later-stage envelope independently, after legitimate
    # source generation, to exercise every execution boundary directly.
    semantic = json.dumps(semantic_for_attack)
    parallel = good([tools["parallel"]], semantic)
    optimized = good([tools["optimize"]], parallel)
    path = temp / "optimized.json"
    path.write_text(optimized)
    prepared = good([tools["prepare"], path])
    for stage, artifact, extra in [
        ("optimize", parallel, []),
        ("prepare", optimized, []),
        ("lower", optimized, ["--emit-llvm", temp / "bad.ll"]),
        ("lower", prepared, ["--emit-llvm", temp / "bad-prepared.ll"]),
        ("tiny_lower", prepared, ["-", temp / "bad.tvm"]),
    ]:
        hostile = json.loads(artifact)
        zero_divisor(hostile["lowering_plan"])
        result = run([tools[stage], *extra], json.dumps(hostile))
        assert result.returncode != 0, (stage, result.stdout, result.stderr)
        hostile_count += 1
    # Identity and type evidence cannot be removed or made stale while retaining
    # the same division. Exercise each separately invocable artifact consumer.
    def corrupt_divisor(value, attack):
        if isinstance(value, dict):
            value.pop("scalar_fact", None)
            if value.get("kind") == "binary" and value.get("operator") == "/":
                if attack == "identity":
                    value["right"]["symbol_id"] += 10000
                else:
                    value["right"].pop("type", None)
            for child in value.values():
                corrupt_divisor(child, attack)
        elif isinstance(value, list):
            for child in value:
                corrupt_divisor(child, attack)

    for attack in ("identity", "missing_type"):
        for stage, artifact, extra in [
            ("validate", semantic, []), ("bind", semantic, []),
            ("parallel", semantic, []), ("optimize", parallel, []),
            ("prepare", optimized, []),
            ("lower", optimized, ["--emit-llvm", temp / "bad.ll"]),
            ("lower", prepared, ["--emit-llvm", temp / "bad-prepared.ll"]),
            ("tiny_lower", prepared, ["-", temp / "bad.tvm"]),
        ]:
            hostile = json.loads(artifact)
            corrupt_divisor(hostile["lowering_plan"], attack)
            result = run([tools[stage], *extra], json.dumps(hostile))
            assert result.returncode != 0, (stage, attack, result.stdout, result.stderr)
            hostile_count += 1
    assert not (temp / "bad.ll").exists()
    assert not (temp / "bad-prepared.ll").exists()
    assert not (temp / "bad.tvm").exists()
    print(json.dumps({"positive_sources": len(positive), "negative_sources": len(negative),
                      "hostile_boundaries": hostile_count, "native_optimizations": 2,
                      "tinyvm_parity": True}))
