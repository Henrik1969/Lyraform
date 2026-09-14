#include <tinyvm/artifact_v2.h>

#include <stdio.h>
#include <string.h>

int main(int argc,char **argv){
    if(argc!=2){fputs("usage: artifact-v2-allocation-fault ARTIFACT\n",stderr);return 2;}
    TinyvmArtifactV2 artifact;
    char diagnostic[160]={0};
    if(tinyvm_artifact_v2_read(argv[1],&artifact,diagnostic,sizeof diagnostic)){
        tinyvm_artifact_v2_destroy(&artifact);
        fputs("artifact read unexpectedly succeeded\n",stderr);
        return 1;
    }
    if(strcmp(diagnostic,"allocation failed")){fprintf(stderr,"unexpected diagnostic: %s\n",diagnostic);return 1;}
    if(artifact.code||artifact.constants||artifact.strings||artifact.storage||artifact.imports||artifact.provenance||artifact.graph_activations){
        fputs("allocation failure retained artifact ownership\n",stderr);
        tinyvm_artifact_v2_destroy(&artifact);
        return 1;
    }
    puts("TinyVM artifact v2 allocation fault refused with no artifact");
    return 0;
}
