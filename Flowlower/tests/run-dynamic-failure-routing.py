#!/usr/bin/env python3
"""Exercise one real provider failure through the declared recovery route.

This is deliberately a callable-ABI/runtime-frame slice.  It does not claim
source-graph or TinyVM disposition execution; those require the next backend
integration boundary.
"""

import json
import os
import pathlib
import subprocess
import tempfile


build = pathlib.Path(__import__("sys").argv[1]).resolve()
root = pathlib.Path(__import__("sys").argv[2]).resolve()
unit = pathlib.Path(__import__("sys").argv[3]).resolve()
environment = dict(os.environ)
environment["LD_LIBRARY_PATH"] = ":".join(
    [
        str(build / "flowtools/reference/text"),
        str(build / "flowtools/reference/disposition"),
        environment.get("LD_LIBRARY_PATH", ""),
    ]
)


def run(tool, args=(), data=None, okay=True):
    result = subprocess.run(
        [str(tool), *map(str, args)],
        input=data,
        text=True,
        capture_output=True,
        cwd=root,
        env=environment,
    )
    if (result.returncode == 0) != okay:
        raise AssertionError(
            f"{tool}: {result.returncode}\n{result.stdout}\n{result.stderr}"
        )
    return result.stdout


def c_string(value):
    return json.dumps(value)


def serialized(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"))


with tempfile.TemporaryDirectory(prefix="lyraform-dynamic-failure-") as directory:
    temp = pathlib.Path(directory)
    source = (
        root / "Lyraform/compiler/examples/disposition/text_outcome_consumer.flow"
    ).read_text()
    source = source.replace(
        "../../std/abi/libc.flow", str(root / "Lyraform/compiler/std/abi/libc.flow")
    ).replace(
        "../../std/abi/text.flow", str(root / "Lyraform/compiler/std/abi/text.flow")
    )
    oversized = "A" * 4096
    source = source.replace(
        'concat_outcome("Lyra", "form")',
        f'concat_outcome("{oversized}", "x")',
    )
    source_path = temp / "dynamic-failure.flow"
    source_path.write_text(source)

    frontend = run(build / "flowmini/flowmini", ["--dump-frontend-bundle", source_path])
    initial = json.loads(
        run(
            build / "flowanalyst/flowanalyst",
            ["--lowering-plan-version", "2"],
            frontend,
            okay=False,
        )
    )
    assert [item["code"] for item in initial["diagnostics"]] == [
        "FLOWANALYST_HOST_POLICY_MISSING"
    ]

    topology = initial["lowering_plan"]["source_disposition"]
    host_policy = json.loads(
        (
            root
            / "Lyraform/compiler/examples/disposition/policy/text_outcome_consumer.host-policy.json"
        ).read_text()
    )
    host_policy["module"] = {
        key: topology["module"][key] for key in ("identity", "revision")
    }
    host_policy_path = temp / "host-policy.json"
    host_policy_path.write_text(json.dumps(host_policy))
    semantic = run(
        build / "flowanalyst/flowanalyst",
        [
            "--lowering-plan-version",
            "2",
            "--graph-plan-version",
            "2",
            "--host-disposition-policy",
            host_policy_path,
        ],
        frontend,
    )
    report = json.loads(semantic)
    plan = report["lowering_plan"]
    assert report["status"] == "ok"
    assert plan["status"] == "blocked"
    assert plan["source_disposition"]["execution"] == "unsupported"

    grants = temp / "grants"
    grants.write_text(
        "allow libc.so.6 puts c io Text c_int\n"
        "allow libflowtext.so flow_text_concat_value c memory Text,Text TextOutcome\n"
    )
    binding = temp / "binding.json"
    binding.write_text(
        run(build / "flowbind/flowbind", ["--policy", grants], semantic)
    )
    parallel = run(build / "flowtools/flowparallel/flowparallel", data=semantic)
    optimized = run(build / "flowoptimize/flowoptimize", data=parallel)
    prepared = temp / "prepared.json"
    prepared.write_text(
        run(
            build / "flowlower/flowprepare",
            ["--binding-report", binding],
            optimized,
        )
    )
    llvm = temp / "callables.ll"
    llvm.write_text(run(unit, [prepared]))

    topology = plan["source_disposition"]
    producer = topology["producer"]
    response = topology["response_functions"][0]
    selection = plan["source_disposition_policy"]["selections"][0]
    containment = topology["containment"]
    junction = topology["junction"]
    authority = {
        "module": topology["module"]["identity"],
        "revision": topology["module"]["revision"],
        "activation": "activation:dynamic-failure-test:1",
        "producer": producer["graph_node_id"],
        "obligation": producer["obligation_id"],
        "success_type": producer["success_type"]["identity"],
        "failure_type": response["incoming_failure_type"]["identity"],
        "fault_type": producer["fault_types"][0]["identity"],
        "failure_wire": junction["failure_wire_id"],
        "fault_wire": containment["wire_id"],
        "containment": containment["node_id"],
        "policy": serialized(plan["source_disposition_policy"]),
        "host_policy": serialized(plan["host_disposition_policy"]),
        "provenance": serialized(producer["provenance"]),
        "commit_law": producer["commit_law"],
        "response_provenance": serialized(response["provenance"]),
        "operation": producer["operation_id"],
        "function": producer["function_symbol_id"],
        "disposition": producer["disposition_id"],
        "response_function": response["function_symbol_id"],
        "response_parameter": response["parameter_symbol_id"],
        "route": selection["route_id"],
    }
    driver = temp / "driver.c"
    driver.write_text(
        "#include <stdint.h>\n"
        "#include <stddef.h>\n"
        "#include <string.h>\n"
        '#include "activation.h"\n'
        '#include "text_runtime.h"\n'
        f'extern FlowTextOutcome producer(void) __asm__("flow.function.{producer["function_symbol_id"]}");\n'
        f'extern const char *response(const FlowActivationEnvelope *) __asm__("flow.function.{response["function_symbol_id"]}");\n'
        "int __wrap_main(void) {\n"
        "    FlowTextOutcome value = producer();\n"
        "    if (value.code != FLOW_TEXT_EXHAUSTED || value.value != NULL) return 1;\n"
        "    FlowActivationAuthority authority = {\n"
        f'        {c_string(authority["module"])}, {c_string(authority["revision"])}, {c_string(authority["activation"])},\n'
        f'        {c_string(authority["producer"])}, {c_string(authority["obligation"])},\n'
        f'        {c_string(authority["success_type"])}, {c_string(authority["failure_type"])}, {c_string(authority["fault_type"])},\n'
        f'        {c_string(authority["failure_wire"])}, {c_string(authority["fault_wire"])}, {c_string(authority["containment"])},\n'
        f'        {c_string(authority["policy"])}, {c_string(authority["host_policy"])},\n'
        f'        {c_string(authority["provenance"])}, {c_string(authority["commit_law"])},\n'
        f'        {c_string(authority["response_provenance"])},\n'
        f'        {authority["operation"]}, {authority["function"]}, {authority["disposition"]},\n'
        f'        {authority["response_function"]}, {authority["response_parameter"]}, {authority["route"]}\n'
        "    };\n"
        "    FlowActivationFrame frame = {0};\n"
        '    if (!flow_activation_begin(&frame, &authority, "attempt:dynamic-1", "correlation:dynamic-1")) return 2;\n'
        "    FlowActivationValue failed = {authority.failure_type, &value.code, sizeof(value.code)};\n"
        '    if (!flow_activation_fail(&frame, failed, "no_commit")) return 3;\n'
        "    const FlowActivationEnvelope *envelope = flow_activation_failure(&frame);\n"
        "    if (!envelope || envelope->payload.data != &value.code) return 4;\n"
        "    const char *fallback = response(envelope);\n"
        '    if (!fallback || strcmp(fallback, "text unavailable") != 0) return 5;\n'
        "    FlowActivationValue recovered = {authority.success_type, fallback, strlen(fallback) + 1};\n"
        f'    if (!flow_activation_recover(&frame, {authority["response_function"]}, {authority["response_parameter"]}, {authority["route"]}, "response:dynamic-1", authority.response_provenance, recovered)) return 6;\n'
        "    const FlowActivationResult *result = flow_activation_result(&frame);\n"
        "    if (!result || result->disposition != FLOW_ACTIVATION_PUBLISHED || !result->obligation_closed || result->envelope != envelope) return 7;\n"
        "    return 0;\n"
        "}\n"
    )
    binary = temp / "dynamic-failure"
    run(
        "clang-18",
        [
            llvm,
            driver,
            "-I" + str(root / "Flowtools/reference/disposition"),
            "-I" + str(root / "Flowtools/reference/text"),
            "-L" + str(build / "flowtools/reference/disposition"),
            "-Wl,-rpath," + str(build / "flowtools/reference/disposition"),
            "-lflowactivation",
            "-L" + str(build / "flowtools/reference/text"),
            "-Wl,-rpath," + str(build / "flowtools/reference/text"),
            "-lflowtext",
            "-Wl,--wrap=main",
            "-o",
            binary,
        ],
    )
    run(binary)

print("dynamic failure routing: PASS (real provider failure, exact envelope, resolved recovery, closed obligation)")
