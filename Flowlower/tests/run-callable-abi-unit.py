#!/usr/bin/env python3
"""Execute source callable ABIs in a C unit host, not the source graph executor."""
import json,os,pathlib,shlex,subprocess,sys,tempfile
build=pathlib.Path(sys.argv[1]).resolve();root=pathlib.Path(sys.argv[2]).resolve();unit=sys.argv[3]
sanitize='--sanitize' in sys.argv[4:];memcheck='--valgrind' in sys.argv[4:]
link_flags=shlex.split(os.environ.get('FLOWCALLABLE_C_FLAGS',''))
if sanitize:link_flags += ['-fsanitize=address,undefined','-fno-omit-frame-pointer']
assert not (sanitize and memcheck)
env=dict(os.environ);env['LD_LIBRARY_PATH']=str(build/'flowtools/reference/text')+':'+env.get('LD_LIBRARY_PATH','')
def run(tool,args=(),data=None,okay=True):
    result=subprocess.run([str(tool),*map(str,args)],input=data,text=True,capture_output=True,env=env,cwd=root)
    if (result.returncode==0)!=okay:raise AssertionError(f'{tool}: {result.returncode}\n{result.stdout}\n{result.stderr}')
    return result.stdout
with tempfile.TemporaryDirectory(prefix='lyraform-callable-abi-') as tmp:
    out=pathlib.Path(tmp);grants=out/'grants'
    grants.write_text('allow libc.so.6 puts c io Text c_int\nallow libflowtext.so flow_text_concat_value c memory Text,Text TextOutcome\n')
    original=(root/'Lyraform/compiler/examples/disposition/text_outcome_consumer.flow').read_text()
    for variant,expected in [('direct','text unavailable'),('forwarded','text unavailable'),('renamed','changed response')]:
        source=original.replace('../../std',str(root/'Lyraform/compiler/std'))
        if variant=='forwarded':
            source=source.replace('fn use_fallback(', 'fn inner_fallback(').replace('fn display_text(',
                'fn use_fallback(problem : failure TextFailure): recover Text {\n'
                '    result : Text(inner_fallback(problem))\n    result -> return\n}\n\nfn display_text(')
        if variant=='renamed':
            source=source.replace('use_fallback','alternate_response').replace('compose_text','other_producer').replace('text unavailable',expected)
        src=out/(variant+'.flow');src.write_text(source)
        front=run(build/'flowmini/flowmini',['--dump-frontend-bundle',src])
        initial=json.loads(run(build/'flowanalyst/flowanalyst',['--lowering-plan-version','2'],front,okay=False))
        assert [d['code'] for d in initial['diagnostics']]==['FLOWANALYST_HOST_POLICY_MISSING']
        module=initial['lowering_plan']['source_disposition']['module']
        host=json.loads((root/'Lyraform/compiler/examples/disposition/policy/text_outcome_consumer.host-policy.json').read_text())
        host['module']={k:module[k] for k in ('identity','revision')}
        host_path=out/'host.json';host_path.write_text(json.dumps(host))
        sem=run(build/'flowanalyst/flowanalyst',['--lowering-plan-version','2','--graph-plan-version','2','--host-disposition-policy',host_path],front)
        plan=json.loads(sem)['lowering_plan'];topology=plan['source_disposition']
        producer=topology['producer']['function_symbol_id']
        response=topology['response_functions'][0];selected=response['function_symbol_id'];parameter=response['parameter_symbol_id']
        binding=out/'binding.json';binding.write_text(run(build/'flowbind/flowbind',['--policy',grants],sem))
        parallel=run(build/'flowtools/flowparallel/flowparallel',data=sem)
        optimized=run(build/'flowoptimize/flowoptimize',data=parallel)
        prepared=out/'prepared.json';prepared.write_text(run(build/'flowlower/flowprepare',['--binding-report',binding],optimized))
        forbidden=out/'forbidden.ll'
        run(build/'flowlower/flowlower',['--emit-llvm',forbidden,prepared],okay=False)
        assert not forbidden.exists()
        llvm=run(unit,[prepared]);ll=out/'callables.ll';ll.write_text(llvm)
        assert f'define ptr @flow.function.{selected}(ptr %flow_arg_{parameter})' in llvm
        assert f'define {{ i32, ptr }} @flow.function.{producer}()' in llvm
        # The C unit host supplies an actual envelope address. For forwarding,
        # a linker wrapper observes the exact pointer at the inner source call.
        calls=[op for op in plan['operations'] if op['kind']=='call' and op['function_symbol_id']==selected]
        wrapper='';link_args=[];forward_check='';modules=[ll]
        if variant=='forwarded':
            assert len(calls)==1;inner=calls[0]['callee_symbol_id']
            wrapper=f'''static const FlowActivationEnvelope *expected_envelope;
static int observed;
extern const char *inner_real(const FlowActivationEnvelope *) __asm__("__real_flow.function.{inner}");
const char *inner_wrap(const FlowActivationEnvelope *) __asm__("__wrap_flow.function.{inner}");
const char *inner_wrap(const FlowActivationEnvelope *value) {{
    if (value != expected_envelope) return "wrong envelope address";
    observed++;
    return inner_real(value);
}}
'''
            link_args=[f'-Wl,--wrap=flow.function.{inner}'];forward_check='expected_envelope=&envelope;'
            # Keep the actual emitted bodies, but put the inner definition in a
            # separate object so the test linker can observe this call boundary.
            inner_ll=out/'inner.ll';outer_ll=out/'outer.ll'
            run('llvm-extract-18',[f'--func=flow.function.{inner}','-S',ll,'-o',inner_ll])
            run('llvm-extract-18',['--delete',f'--func=flow.function.{inner}','-S',ll,'-o',outer_ll])
            modules=[inner_ll,outer_ll]
        driver=out/'driver.c'
        driver.write_text(f'''#include <string.h>
#include "activation.h"
#include "text_runtime.h"
extern FlowTextOutcome producer(void) __asm__("flow.function.{producer}");
extern const char *response(const FlowActivationEnvelope *) __asm__("flow.function.{selected}");
{wrapper}
int __wrap_main(void) {{
    FlowTextOutcome value=producer();
    if (value.code!=FLOW_TEXT_SUCCESS || !value.value || strcmp(value.value,"Lyraform")) return 1;
    flow_text_dispose(value.value);
    int payload=2;
    FlowActivationEnvelope envelope={{0}};
    envelope.payload.data=&payload; envelope.payload.size=sizeof(payload);
    envelope.payload.type="type:TextFailure"; envelope.attempt="attempt:unit";
    envelope.correlation="correlation:unit"; envelope.commit="no_commit";
    unsigned char before[sizeof(envelope)]; memcpy(before,&envelope,sizeof(envelope));
    {forward_check}
    const char *result=response(&envelope);
    if (!result || strcmp(result,{json.dumps(expected)})) return 2;
    if (memcmp(before,&envelope,sizeof(envelope)) || payload!=2) return 3;
    {'if (observed!=1) return 4;' if variant=='forwarded' else ''}
    return 0;
}}
''')
        binary=out/'unit-program'
        run('clang-18',link_flags+[*modules,driver,'-I'+str(root/'Flowtools/reference/disposition'),'-I'+str(root/'Flowtools/reference/text'),
            '-L'+str(build/'flowtools/reference/disposition'),'-Wl,-rpath,'+str(build/'flowtools/reference/disposition'),'-lflowactivation',
            '-L'+str(build/'flowtools/reference/text'),'-Wl,-rpath,'+str(build/'flowtools/reference/text'),'-lflowtext',
            '-Wl,--wrap=main',*link_args,'-o',binary])
        if memcheck:
            checked=subprocess.run(['valgrind','--error-exitcode=99','--leak-check=full','--errors-for-leak-kinds=all',str(binary)],text=True,capture_output=True,env=env)
            if checked.returncode:raise AssertionError(checked.stderr)
            print(checked.stderr)
        else:
            run(binary)
        if variant == 'direct':
            bridge_driver=out/'bridge-driver.c'
            bridge_driver.write_text(
                'extern int route(void) __asm__("flow.source.disposition");\n'
                'int __wrap_main(void) { int status = route(); return status == 70 ? 1 : 0; }\n')
            bridge_binary=out/'bridge-program'
            run('clang-18',link_flags+[ll,bridge_driver,
                '-I'+str(root/'Flowtools/reference/disposition'),'-I'+str(root/'Flowtools/reference/text'),
                '-L'+str(build/'flowtools/reference/disposition'),'-Wl,-rpath,'+str(build/'flowtools/reference/disposition'),'-lflowactivation',
                '-L'+str(build/'flowtools/reference/text'),'-Wl,-rpath,'+str(build/'flowtools/reference/text'),'-lflowtext',
                '-Wl,--wrap=main','-o',bridge_binary])
            if run(bridge_binary) != 'Lyraform\n':
                raise AssertionError('generated native disposition bridge did not publish success')
    print('PASS: 3 native source-callable ABI units (direct, forwarded envelope identity, renamed/changed body); generated native disposition bridge verified; graph execution remains blocked')

subprocess.run([sys.executable, str(root/'Flowlower/tests/run-native-record-unit.py'),
                str(build), str(root), str(unit), *sys.argv[4:]], check=True)
