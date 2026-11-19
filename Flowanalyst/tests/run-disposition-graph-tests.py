#!/usr/bin/env python3
"""Preserve typed disposition graph projection and reject hostile artifacts."""
import copy,json,os,pathlib,subprocess,sys,tempfile
build=pathlib.Path(sys.argv[1]).resolve();root=pathlib.Path(__file__).resolve().parents[2]
env=dict(os.environ);env['LD_LIBRARY_PATH']=str(build/'flowtools/reference/text')+':'+env.get('LD_LIBRARY_PATH','')
def run(tool,args=(),data=None,okay=True):
    r=subprocess.run([str(build/tool),*map(str,args)],input=data,text=True,capture_output=True,env=env,cwd=root)
    if okay and r.returncode:raise AssertionError(f'{tool}: {r.returncode}\n{r.stdout}\n{r.stderr}')
    if not okay and r.returncode==0:raise AssertionError(f'{tool} accepted hostile artifact')
    return r
with tempfile.TemporaryDirectory(prefix='lyraform-disposition-graph-') as tmp:
    out=pathlib.Path(tmp);grants=out/'grants';grants.write_text('allow libc.so.6 puts c io Text c_int\nallow libflowtext.so flow_text_concat_value c memory Text,Text TextOutcome\n')
    count=0
    prior_binding=None
    for stem in ['text_outcome_consumer','text_outcome_two_routes']:
        src=root/'Lyraform/compiler/examples/disposition'/f'{stem}.flow';pol=src.parent/'policy'
        front=run('flowmini/flowmini',['--dump-frontend-bundle',src]).stdout
        args=['--lowering-plan-version','2','--graph-plan-version','2','--host-disposition-policy',pol/f'{stem}.host-policy.json']
        if stem=='text_outcome_two_routes':args+=['--disposition-policy',pol/f'{stem}.policy.json']
        sem=run('flowanalyst/flowanalyst',args,front).stdout
        assert sem==run('flowanalyst/flowanalyst',args,front).stdout
        plan=json.loads(sem)['lowering_plan'];graph=plan['disposition_graph']
        assert plan['status']=='blocked' and graph['execution']=='unsupported' and graph['schedule']=='serial'
        assert plan['disposition_graph_version']==graph['version']==3
        assert len(graph['nodes'])==6 and len(graph['edges'])==6
        selected=plan['source_disposition_policy']['selections'][0]['response_function_symbol_id']
        assert [n['function_symbol_id'] for n in graph['nodes'] if n['role']=='ordinary_response']==[selected]
        response=next(r for r in plan['source_disposition']['response_functions'] if r['function_symbol_id']==selected)
        node=next(n for n in graph['nodes'] if n['role']=='ordinary_response')
        function=next(f for f in plan['functions'] if f['symbol_id']==selected)
        assert node['parameter_symbol_id']==response['parameter_symbol_id']==function['parameters'][0]['symbol_id']
        assert node['input_projection']==function['parameters'][0]['input_projection']=='immutable_failure_envelope'
        assert node['incoming_failure_type']==response['incoming_failure_type']
        producer=plan['source_disposition']['producer']
        producer_fn=next(f for f in plan['functions'] if f['symbol_id']==producer['function_symbol_id'])
        returned=next(o for o in plan['operations'] if o['id']==producer['return_operation_id'])
        assert producer_fn['return_type']==producer['success_type']['spelling']
        assert producer_fn['result_projection']==producer['result_projection']=={'kind':'atomic_tagged_result','carrier_type':'TextOutcome'}
        assert returned['operands'][0]['kind']=='identifier' and returned['operands'][0]['type']=='TextOutcome'
        assert returned['operands'][0]['symbol_id']==producer['owner_symbol_id']
        previous=json.loads(sem);previous['lowering_plan']['disposition_graph_version']=2
        previous['lowering_plan']['disposition_graph']['version']=2
        del previous['lowering_plan']['disposition_graph']['nodes'][0]['result_projection']
        run('flowcontracts/flowvalidate',data=json.dumps(previous))
        # Version 1 remains an explicitly non-executable historical projection.
        legacy=json.loads(sem);lp=legacy['lowering_plan'];lp['disposition_graph_version']=1;lp['disposition_graph']['version']=1
        del lp['disposition_graph']['nodes'][0]['result_projection']
        for n in lp['disposition_graph']['nodes']:
            if n['role']=='ordinary_response':
                for key in ('parameter_symbol_id','input_projection','incoming_failure_type'):del n[key]
        for r in lp['source_disposition']['response_functions']:del r['parameter_symbol_id']
        for f in lp['functions']:
            for parameter in f['parameters']:parameter.pop('input_projection',None)
        run('flowcontracts/flowvalidate',data=json.dumps(legacy))

        bind=run('flowbind/flowbind',['--policy',grants],sem).stdout;binding=out/'binding.json';binding.write_text(bind)
        assert json.loads(bind)['disposition_projection']['disposition_graph']==graph
        parallel=run('flowtools/flowparallel/flowparallel',data=sem).stdout
        optimized=run('flowoptimize/flowoptimize',data=parallel).stdout
        prepared=run('flowlower/flowprepare',['--binding-report',binding],optimized).stdout
        if prior_binding is not None:
            foreign=out/'foreign-valid-binding.json';foreign.write_text(prior_binding)
            run('flowcontracts/flowvalidate',data=prior_binding)
            run('flowlower/flowprepare',['--binding-report',foreign],optimized,okay=False)
        prior_binding=bind
        for stage,raw in [('semantic',sem),('parallel',parallel),('optimized',optimized),('prepared',prepared)]:
            doc=json.loads(raw);assert doc['lowering_plan']['disposition_graph']==graph
            run('flowcontracts/flowvalidate',data=raw)
            for mutation in range(25):
                bad=copy.deepcopy(doc);p=bad['lowering_plan'];g=p['disposition_graph']
                if mutation==0:del p['disposition_graph']
                elif mutation==1:del p['disposition_graph_version']
                elif mutation==2:g['nodes'][0]['function_symbol_id']=999999
                elif mutation==3:g['edges'][0]['to']='foreign'
                elif mutation==4:g['schedule']='parallel'
                elif mutation==5:g['execution']='ready'
                elif mutation==6:g['obligation_id']='foreign'
                elif mutation==7:g['edges'].pop()
                elif mutation==8:next(n for n in g['nodes'] if n['role']=='ordinary_response')['parameter_symbol_id']=999999
                elif mutation==9:next(n for n in g['nodes'] if n['role']=='ordinary_response')['input_projection']='payload'
                elif mutation==10:del p['source_disposition']['response_functions'][0]['parameter_symbol_id']
                elif mutation==11:p['source_disposition']['response_functions'][0]['parameter_symbol_id']=999999
                elif mutation==12:next(f for f in p['functions'] if f['symbol_id']==selected)['parameters'][0]['input_projection']='payload'
                elif mutation==13:del next(f for f in p['functions'] if f['symbol_id']==selected)['parameters'][0]['input_projection']
                elif mutation==14:next(f for f in p['functions'] if f['symbol_id']==selected)['parameters'][0]['type']='c_int'
                elif mutation==15:next(f for f in p['functions'] if f['symbol_id']==selected)['return_type']='c_int'
                elif mutation==16:del p['disposition_facts']
                elif mutation==17:g['nodes'][0]['result_projection']['carrier_type']='Text'
                elif mutation==18:del p['source_disposition']['producer']['result_projection']
                elif mutation==19:del next(f for f in p['functions'] if f['symbol_id']==producer['function_symbol_id'])['result_projection']
                elif mutation==20:next(f for f in p['functions'] if f['symbol_id']==producer['function_symbol_id'])['result_projection']['carrier_type']='Text'
                elif mutation==21:next(o for o in p['operations'] if o['id']==producer['return_operation_id'])['operands'][0]['type']='Text'
                elif mutation==22:next(o for o in p['operations'] if o['id']==producer['return_operation_id'])['operands'][0]['symbol_id']=999999
                elif mutation==23:next(o for o in p['operations'] if o['id']==producer['return_operation_id'])['operands'][0]['kind']='conversion'
                else:next(f for f in p['functions'] if f['symbol_id']==selected)['result_projection']=producer['result_projection']
                encoded=json.dumps(bad);run('flowcontracts/flowvalidate',data=encoded,okay=False)
                if stage=='semantic':run('flowtools/flowparallel/flowparallel',data=encoded,okay=False);run('flowbind/flowbind',['--policy',grants],encoded,okay=False)
                elif stage=='parallel':run('flowoptimize/flowoptimize',data=encoded,okay=False)
                elif stage=='optimized':run('flowlower/flowprepare',['--binding-report',binding],encoded,okay=False)
                count+=1
        # A binding report for a different projection cannot authorize preparation.
        bad=json.loads(bind);bad['disposition_projection']['disposition_graph']['edges'][0]['to']='foreign';binding.write_text(json.dumps(bad))
        run('flowlower/flowprepare',['--binding-report',binding],optimized,okay=False)
        binding.write_text(bind)
        ready=out/'prepared.json';ready.write_text(prepared)
        run('flowlower/flowlower',['--emit-llvm',out/'forbidden.ll',ready],okay=False)
        run('tinyvm/flowtinylower',[ready,out/'forbidden.tvm'],okay=False)
        assert not (out/'forbidden.ll').exists() and not (out/'forbidden.tvm').exists()
    # An existing immutable envelope may be passed to another ordinary response;
    # the admission check must not mistake its payload spelling for a bare value.
    source=root/'Lyraform/compiler/examples/disposition/text_outcome_consumer.flow'
    text=source.read_text().replace('../../std',str(root/'Lyraform/compiler/std'))
    text=text.replace('fn use_fallback(', 'fn inner_fallback(').replace('fn display_text(',
        'fn use_fallback(problem : failure TextFailure): recover Text {\n'
        '    result : Text(inner_fallback(problem))\n    result -> return\n}\n\nfn display_text(')
    forwarded=out/'forwarded.flow';forwarded.write_text(text)
    front=run('flowmini/flowmini',['--dump-frontend-bundle',forwarded]).stdout
    initial=json.loads(run('flowanalyst/flowanalyst',['--lowering-plan-version','2'],front,okay=False).stdout)
    assert [d['code'] for d in initial['diagnostics']]==['FLOWANALYST_HOST_POLICY_MISSING']
    host=json.loads((source.parent/'policy/text_outcome_consumer.host-policy.json').read_text())
    module=initial['lowering_plan']['source_disposition']['module']
    host['module']={key:module[key] for key in ('identity','revision')}
    host_path=out/'forwarded.host.json';host_path.write_text(json.dumps(host))
    accepted=json.loads(run('flowanalyst/flowanalyst',
        ['--lowering-plan-version','2','--host-disposition-policy',host_path],front).stdout)
    assert accepted['status']=='ok' and accepted['lowering_plan']['status']=='blocked'
    print(f'Disposition graph PASS: 2 selections, 4 preserved plan stages, {count} hostile stage mutations, binding mismatch and execution refusal')
