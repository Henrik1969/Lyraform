#include <tinyvm/runtime_provider.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(int argc,char **argv){
    if(argc!=2){fputs("usage: runtime-provider-allocation-fault POLICY\n",stderr);return 2;}
    const char *arguments[]={"left","right","/dev/null"};
    TinyvmRuntimeProvider provider={argv[1],3,arguments,NULL,0,0,NULL,0,0,0,0};
    uint8_t bytes[]={'l','e','f','t'};
    TinyvmString strings[]={{1,bytes,sizeof bytes}};
    TinyvmArtifactV2 artifact={0};
    artifact.strings=strings;
    artifact.string_count=1;
    TinyvmImport import={0};
    import.id=1;
    snprintf(import.contract,64,"libc");
    snprintf(import.library,64,"libc.so.6");
    snprintf(import.convention,64,"c");
    snprintf(import.symbol,64,"strlen");
    snprintf(import.effect,64,"pure");
    snprintf(import.parameters,64,"c_string");
    snprintf(import.result,64,"c_size_t");
    snprintf(import.evidence,64,"runtime-provider-allocation-fault");
    TinyvmValue args[]={{TINYVM_CARRIER_OPAQUE_HANDLE,(UINT64_C(1)<<56)|1,true}};
    TinyvmValue result={0};
    const char *fault=NULL;
    if(tinyvm_runtime_provider_resolve(&provider,&artifact,&import,args,1,&result,&fault)){
        tinyvm_runtime_provider_destroy(&provider);
        fputs("runtime provider allocation fault unexpectedly succeeded\n",stderr);
        return 1;
    }
    if(!fault||strcmp(fault,"runtime provider allocation exhausted")||result.initialized||provider.text_outcome_count||provider.text_outcomes){
        fprintf(stderr,"unexpected allocation outcome: fault=%s initialized=%d outcomes=%zu\n",fault?fault:"<null>",result.initialized,provider.text_outcome_count);
        tinyvm_runtime_provider_destroy(&provider);
        return 1;
    }
    memset(&import,0,sizeof import);
    import.id=1;
    snprintf(import.contract,64,"text_runtime");
    snprintf(import.library,64,"libc.so.6");
    snprintf(import.convention,64,"c");
    snprintf(import.symbol,64,"flow_text_concat_value");
    snprintf(import.effect,64,"memory");
    snprintf(import.parameters,64,"Text,Text");
    snprintf(import.result,64,"TextOutcome");
    snprintf(import.evidence,64,"runtime-provider-allocation-fault");
    TinyvmValue text_args[]={
        {TINYVM_CARRIER_OPAQUE_HANDLE,(UINT64_C(3)<<56)|0,true},
        {TINYVM_CARRIER_OPAQUE_HANDLE,(UINT64_C(3)<<56)|1,true}
    };
    result=(TinyvmValue){0};
    fault=NULL;
    if(tinyvm_runtime_provider_resolve(&provider,&artifact,&import,text_args,2,&result,&fault)||!fault||strcmp(fault,"runtime provider allocation exhausted")||result.initialized||provider.text_outcome_count||provider.text_outcomes){
        fprintf(stderr,"unexpected outcome-retention allocation result: fault=%s initialized=%d outcomes=%zu\n",fault?fault:"<null>",result.initialized,provider.text_outcome_count);
        tinyvm_runtime_provider_destroy(&provider);
        return 1;
    }
    uint8_t storage_bytes[8]={0};
    TinyvmStorage storage[]={{1,sizeof storage_bytes,8,2}};
    artifact.storage=storage;
    artifact.storage_count=1;
    memset(&import,0,sizeof import);
    import.id=1;
    snprintf(import.contract,64,"memory");
    snprintf(import.library,64,"libc.so.6");
    snprintf(import.convention,64,"c");
    snprintf(import.symbol,64,"memset");
    snprintf(import.effect,64,"io");
    snprintf(import.parameters,64,"c_pointer,c_int,c_size_t");
    snprintf(import.result,64,"c_pointer");
    snprintf(import.evidence,64,"runtime-provider-allocation-fault");
    TinyvmValue memory_args[]={
        {TINYVM_CARRIER_OPAQUE_HANDLE,(UINT64_C(2)<<56)|1,true},
        {TINYVM_CARRIER_I32,0x41,true},
        {TINYVM_CARRIER_I64,sizeof storage_bytes,true}
    };
    result=(TinyvmValue){0};
    fault=NULL;
    if(tinyvm_runtime_provider_resolve(&provider,&artifact,&import,memory_args,3,&result,&fault)||!fault||strcmp(fault,"runtime provider allocation exhausted")||result.initialized||provider.storage||provider.storage_count){
        fprintf(stderr,"unexpected storage allocation result: fault=%s initialized=%d storage=%p count=%zu\n",fault?fault:"<null>",result.initialized,(void *)provider.storage,provider.storage_count);
        tinyvm_runtime_provider_destroy(&provider);
        return 1;
    }
    memset(&import,0,sizeof import);
    import.id=1;
    snprintf(import.contract,64,"file_io");
    snprintf(import.library,64,"libc.so.6");
    snprintf(import.convention,64,"c");
    snprintf(import.symbol,64,"open");
    snprintf(import.effect,64,"io");
    snprintf(import.parameters,64,"c_string,c_int");
    snprintf(import.result,64,"c_int");
    snprintf(import.evidence,64,"runtime-provider-allocation-fault");
    TinyvmValue open_args[]={
        {TINYVM_CARRIER_OPAQUE_HANDLE,(UINT64_C(3)<<56)|2,true},
        {TINYVM_CARRIER_I32,0,true}
    };
    result=(TinyvmValue){0};
    fault=NULL;
    if(tinyvm_runtime_provider_resolve(&provider,&artifact,&import,open_args,2,&result,&fault)||!fault||strcmp(fault,"runtime provider allocation exhausted")||result.initialized||provider.file_descriptor_count||provider.file_descriptors){
        fprintf(stderr,"unexpected descriptor allocation result: fault=%s initialized=%d descriptors=%zu\n",fault?fault:"<null>",result.initialized,provider.file_descriptor_count);
        tinyvm_runtime_provider_destroy(&provider);
        return 1;
    }
    tinyvm_runtime_provider_destroy(&provider);
    puts("TinyVM runtime provider allocation faults refused with no outcome");
    return 0;
}
