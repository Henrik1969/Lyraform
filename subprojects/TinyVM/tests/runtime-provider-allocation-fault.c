#include <tinyvm/runtime_provider.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(int argc,char **argv){
    if(argc!=2){fputs("usage: runtime-provider-allocation-fault POLICY\n",stderr);return 2;}
    const char *arguments[]={"left","right"};
    TinyvmRuntimeProvider provider={argv[1],2,arguments,NULL,0,0,NULL,0,0,0,0};
    TinyvmImport import={0};
    import.id=1;
    snprintf(import.contract,64,"text_runtime");
    snprintf(import.library,64,"libc.so.6");
    snprintf(import.convention,64,"c");
    snprintf(import.symbol,64,"flow_text_concat_value");
    snprintf(import.effect,64,"memory");
    snprintf(import.parameters,64,"Text,Text");
    snprintf(import.result,64,"TextOutcome");
    snprintf(import.evidence,64,"runtime-provider-allocation-fault");
    TinyvmValue args[]={
        {TINYVM_CARRIER_OPAQUE_HANDLE,(UINT64_C(3)<<56)|0,true},
        {TINYVM_CARRIER_OPAQUE_HANDLE,(UINT64_C(3)<<56)|1,true}
    };
    TinyvmValue result={0};
    const char *fault=NULL;
    if(tinyvm_runtime_provider_resolve(&provider,NULL,&import,args,2,&result,&fault)){
        tinyvm_runtime_provider_destroy(&provider);
        fputs("runtime provider allocation fault unexpectedly succeeded\n",stderr);
        return 1;
    }
    if(!fault||strcmp(fault,"runtime provider allocation exhausted")||result.initialized||provider.text_outcome_count||provider.text_outcomes){
        fprintf(stderr,"unexpected allocation outcome: fault=%s initialized=%d outcomes=%zu\n",fault?fault:"<null>",result.initialized,provider.text_outcome_count);
        tinyvm_runtime_provider_destroy(&provider);
        return 1;
    }
    tinyvm_runtime_provider_destroy(&provider);
    puts("TinyVM runtime provider allocation fault refused with no outcome");
    return 0;
}
