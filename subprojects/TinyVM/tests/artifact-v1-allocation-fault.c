#include <tinyvm/artifact.h>

#include <stdio.h>
#include <string.h>

int main(int argc,char **argv){
    if(argc!=2){fputs("usage: artifact-v1-allocation-fault OUTPUT\n",stderr);return 2;}
    InstrWord code[]={{OP_ADD,0,1,0},{OP_HALT,0,0,0}};
    TinyvmArtifact artifact;
    tinyvm_artifact_init(&artifact);
    snprintf(artifact.artifact_id,64,"fault-artifact-v1");
    snprintf(artifact.source_id,64,"fault-source-v1");
    snprintf(artifact.target,64,"tinyvm-portable");
    snprintf(artifact.lowering_plan_id,64,"fault-lowering-v1");
    snprintf(artifact.optimization_id,64,"fault-optimization-v1");
    artifact.code=code; artifact.code_count=2;
    char diagnostic[160]={0};
    if(tinyvm_artifact_write(argv[1],&artifact,diagnostic,sizeof diagnostic)){
        remove(argv[1]);
        fputs("artifact write unexpectedly succeeded\n",stderr);
        return 1;
    }
    if(strcmp(diagnostic,"allocation failed")){fprintf(stderr,"unexpected diagnostic: %s\n",diagnostic);return 1;}
    FILE *file=fopen(argv[1],"rb");
    if(file){fclose(file);remove(argv[1]);fputs("allocation failure published an artifact\n",stderr);return 1;}
    puts("TinyVM artifact v1 allocation fault refused with no artifact");
    return 0;
}
