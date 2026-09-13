#include <tinyvm/artifact.h>
#include <tinyvm/artifact_v2.h>
#include <tinyvm/isa_v1.h>
#include <tinyvm/runtime_provider.h>

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static unsigned version(const char *path) {
    unsigned char header[12]; FILE *file=fopen(path,"rb"); if(!file)return 0;
    const size_t count=fread(header,1,sizeof header,file); fclose(file); if(count!=sizeof header)return 0;
    return (unsigned)header[8]|(unsigned)header[9]<<8|(unsigned)header[10]<<16|(unsigned)header[11]<<24;
}

static int run_v2(const char *path,const char *policy,const char *engine,int argument_count,char **arguments) {
    TinyvmArtifactV2 artifact; char diagnostic[160];
    if(!tinyvm_artifact_v2_read(path,&artifact,diagnostic,sizeof diagnostic)){fprintf(stderr,"flowtinyrun: %s\n",diagnostic);return 1;}
    if(artifact.isa_version==0) {
        vm_context context; vm_init(&context); context.pc=(size_t)artifact.entrypoint; context.regs[0]=19; context.regs[1]=23;
        const bool ok=vm_run_switch(&context,artifact.code,artifact.code_count);
        printf("{\"format\":\"flowtiny.execution_record\",\"version\":1,\"status\":\"%s\",\"artifact_format\":2,\"artifact_id\":\"%s\",\"target_policy_id\":\"%s\",\"result\":%" PRId64 ",\"pc\":%zu}\n",ok?"completed":"faulted",artifact.artifact_id,artifact.target_policy_id,context.regs[0],context.pc);
        tinyvm_artifact_v2_destroy(&artifact); return ok?0:1;
    }
    if(artifact.isa_version!=1&&artifact.isa_version!=2){tinyvm_artifact_v2_destroy(&artifact);return 2;}
    TinyvmRuntimeProvider provider={policy,(size_t)argument_count,(const char *const *)arguments,NULL,0,0};
    const char *fault=NULL;
    if(artifact.import_count&&!tinyvm_runtime_provider_preflight(&provider,&artifact,&fault)){fprintf(stderr,"flowtinyrun: %s\n",fault);tinyvm_artifact_v2_destroy(&artifact);return 2;}
    TinyvmIsaV1Context context;
    if(!tinyvm_isa_v1_context_init(&context,(size_t)artifact.data_words,UINT64_C(10000000))){tinyvm_artifact_v2_destroy(&artifact);return 1;}
    context.argument_count=provider.argument_count; context.arguments=provider.arguments;
    if(artifact.import_count){context.import_resolver=tinyvm_runtime_provider_resolve;context.import_user=&provider;}
    const bool ok=!strcmp(engine,"computed")?tinyvm_isa_v1_run_computed(&artifact,&context):tinyvm_isa_v1_run_switch(&artifact,&context);
    if(!ok&&context.fault)fprintf(stderr,"flowtinyrun: %s\n",context.fault);
    printf("{\"format\":\"flowtiny.execution_record\",\"version\":1,\"status\":\"%s\",\"artifact_format\":2,\"artifact_id\":\"%s\",\"target_policy_id\":\"%s\",\"carrier\":%u,\"result\":%" PRIu64 ",\"pc\":%" PRIu64 ",\"trap\":%u}\n",ok?"completed":"faulted",artifact.artifact_id,artifact.target_policy_id,context.result.carrier,context.result.bits,context.pc,context.trap);
    tinyvm_isa_v1_context_destroy(&context); tinyvm_runtime_provider_destroy(&provider); tinyvm_artifact_v2_destroy(&artifact); return ok?0:1;
}

static int run_v1(const char *path) {
    TinyvmArtifact artifact; char diagnostic[160];
    if(!tinyvm_artifact_read(path,&artifact,diagnostic,sizeof diagnostic)){fprintf(stderr,"flowtinyrun: %s\n",diagnostic);return 1;}
    vm_context context; vm_init(&context); context.pc=(size_t)artifact.entrypoint; context.regs[0]=19; context.regs[1]=23;
    const bool ok=vm_run_switch(&context,artifact.code,artifact.code_count);
    printf("{\"format\":\"flowtiny.execution_record\",\"version\":1,\"status\":\"%s\",\"artifact_id\":\"%s\",\"result\":%" PRId64 ",\"pc\":%zu}\n",ok?"completed":"faulted",artifact.artifact_id,context.regs[0],context.pc);
    tinyvm_artifact_destroy(&artifact); return ok?0:1;
}

int main(int argc,char **argv) {
    const char *policy=NULL,*engine="switch"; int first=1;
    while(first<argc&&!strncmp(argv[first],"--",2)) {
        if(!strcmp(argv[first],"--policy")&&first+1<argc){policy=argv[first+1];first+=2;}
        else if(!strcmp(argv[first],"--engine")&&first+1<argc){engine=argv[first+1];first+=2;}
        else {fprintf(stderr,"flowtinyrun: unknown or incomplete option\n");return 2;}
    }
    if(strcmp(engine,"switch")&&strcmp(engine,"computed")){fprintf(stderr,"flowtinyrun: engine must be switch or computed\n");return 2;}
    if(argc<=first){fprintf(stderr,"usage: %s [--policy FILE] [--engine switch|computed] ARTIFACT [PROGRAM-ARGUMENT ...]\n",argv[0]);return 2;}
    return version(argv[first])==2?run_v2(argv[first],policy,engine,argc-first,&argv[first]):run_v1(argv[first]);
}
