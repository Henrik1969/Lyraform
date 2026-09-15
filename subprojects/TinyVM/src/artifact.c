#include <tinyvm/artifact.h>

#include <openssl/sha.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef TINYVM_ARTIFACT_V1_TEST_ALLOCATION_FAILURE
static void *tinyvm_artifact_v1_fault_malloc(size_t size){(void)size;return NULL;}
#define malloc tinyvm_artifact_v1_fault_malloc
#endif

enum { HEADER_BYTES = 512, WORD_BYTES = 32 };
static const uint8_t magic[8] = {'F','L','O','W','T','V','M',0};

static void diagnose(char *out, size_t capacity, const char *message) {
    if (out != NULL && capacity > 0) snprintf(out, capacity, "%s", message);
}

static void put32(uint8_t *out, uint32_t value) {
    for (unsigned i = 0; i < 4; ++i) out[i] = (uint8_t)(value >> (i * 8));
}
static void put64(uint8_t *out, uint64_t value) {
    for (unsigned i = 0; i < 8; ++i) out[i] = (uint8_t)(value >> (i * 8));
}
static uint32_t get32(const uint8_t *in) {
    uint32_t value = 0; for (unsigned i = 0; i < 4; ++i) value |= (uint32_t)in[i] << (i * 8); return value;
}
static uint64_t get64(const uint8_t *in) {
    uint64_t value = 0; for (unsigned i = 0; i < 8; ++i) value |= (uint64_t)in[i] << (i * 8); return value;
}
static int64_t get_i64(const uint8_t *in) {
    const uint64_t bits = get64(in); int64_t value; memcpy(&value, &bits, sizeof(value)); return value;
}

static bool bounded_text(const char field[64]) {
    const char *end = memchr(field, 0, 64);
    if (end == NULL || field[0] == 0) return false;
    for (const unsigned char *p = (const unsigned char *)field; *p; ++p)
        if (!( (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
               (*p >= '0' && *p <= '9') || *p == '.' || *p == '_' ||
               *p == ':' || *p == '/' || *p == '+' || *p == '-' )) return false;
    for (const char *p = end; p < field + 64; ++p) if (*p != 0) return false;
    return true;
}

static bool all_zero(const uint8_t *bytes, size_t count) {
    for (size_t i = 0; i < count; ++i) if (bytes[i] != 0) return false;
    return true;
}

static bool register_index(int64_t value) { return value >= 0 && value < 8; }

static bool validate_instruction(const InstrWord *word, size_t pc, size_t count,
                                 char *diagnostic, size_t capacity) {
    if (word->opcode < 0 || word->opcode >= OP_COUNT) {
        diagnose(diagnostic, capacity, "opcode is outside recovered ISA"); return false;
    }
    if (word->pad != 0) {
        diagnose(diagnostic, capacity, "reserved instruction field is nonzero"); return false;
    }
    switch (word->opcode) {
    case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV: case OP_MOD:
    case OP_AND: case OP_OR: case OP_XOR: case OP_SHL: case OP_SHR:
    case OP_CMP: case OP_LOAD: case OP_STORE: case OP_MOV:
        if (!register_index(word->a) || !register_index(word->b)) {
            diagnose(diagnostic, capacity, "instruction register is out of range"); return false;
        }
        break;
    case OP_NOT: case OP_PUSH: case OP_POP:
        if (!register_index(word->a)) {
            diagnose(diagnostic, capacity, "instruction register is out of range"); return false;
        }
        break;
    case OP_JMP: case OP_JZ: case OP_JNZ: {
        const int64_t target = (int64_t)pc + word->a;
        if (target < 0 || (uint64_t)target >= count) {
            diagnose(diagnostic, capacity, "jump target is out of artifact code range"); return false;
        }
        break;
    }
    default: break;
    }
    return true;
}

void tinyvm_artifact_init(TinyvmArtifact *artifact) {
    memset(artifact, 0, sizeof(*artifact));
    artifact->format_version = TINYVM_ARTIFACT_FORMAT_VERSION;
    artifact->isa_version = TINYVM_RECOVERED_ISA_VERSION;
    artifact->data_words = 256;
    artifact->stack_words = 256;
}

void tinyvm_artifact_destroy(TinyvmArtifact *artifact) {
    if (artifact == NULL) return;
    free(artifact->code);
    tinyvm_artifact_init(artifact);
}

bool tinyvm_artifact_validate(const TinyvmArtifact *artifact,
                              char *diagnostic, size_t capacity) {
    if (artifact == NULL) { diagnose(diagnostic, capacity, "artifact is null"); return false; }
    if (artifact->format_version != 1) { diagnose(diagnostic, capacity, "unsupported artifact version"); return false; }
    if (artifact->isa_version != 0) { diagnose(diagnostic, capacity, "unsupported ISA version"); return false; }
    if (!bounded_text(artifact->artifact_id) || !bounded_text(artifact->source_id) ||
        !bounded_text(artifact->target) || !bounded_text(artifact->lowering_plan_id) ||
        !bounded_text(artifact->optimization_id)) {
        diagnose(diagnostic, capacity, "required identity is empty, unterminated, or non-printable"); return false;
    }
    if (strcmp(artifact->target, "tinyvm-portable") != 0) { diagnose(diagnostic, capacity, "artifact target is not tinyvm-portable"); return false; }
    if (!tinyvm_validate_recovered_code(artifact->code, artifact->code_count,
                                        artifact->entrypoint, artifact->data_words,
                                        artifact->stack_words, diagnostic, capacity)) return false;
    diagnose(diagnostic, capacity, "valid"); return true;
}

bool tinyvm_validate_recovered_code(const InstrWord *code, size_t code_count,
                                    uint64_t entrypoint, uint64_t data_words,
                                    uint64_t stack_words,
                                    char *diagnostic, size_t capacity) {
    if (code == NULL || code_count == 0) { diagnose(diagnostic, capacity, "artifact code is empty"); return false; }
    if (entrypoint >= code_count) { diagnose(diagnostic, capacity, "entrypoint is outside code"); return false; }
    if (data_words > 256 || stack_words > 256) { diagnose(diagnostic, capacity, "declared recovered-VM resources exceed capacity"); return false; }
    for (size_t i = 0; i < code_count; ++i)
        if (!validate_instruction(&code[i], i, code_count, diagnostic, capacity)) return false;
    if (code[code_count - 1].opcode != OP_HALT) { diagnose(diagnostic, capacity, "artifact does not end in HALT"); return false; }
    return true;
}

static void encode(const TinyvmArtifact *artifact, uint8_t *bytes, size_t size) {
    memset(bytes, 0, size); memcpy(bytes, magic, 8);
    put32(bytes+8, artifact->format_version); put32(bytes+12, artifact->isa_version);
    put32(bytes+16, HEADER_BYTES); put32(bytes+20, 0); bytes[24]=1; bytes[25]=WORD_BYTES;
    put64(bytes+32, artifact->entrypoint); put64(bytes+40, artifact->code_count);
    put64(bytes+48, artifact->data_words); put64(bytes+56, artifact->stack_words);
    memcpy(bytes+64,artifact->artifact_id,64); memcpy(bytes+128,artifact->source_id,64);
    memcpy(bytes+192,artifact->target,64); memcpy(bytes+256,artifact->lowering_plan_id,64);
    memcpy(bytes+320,artifact->optimization_id,64);
    for (size_t i=0;i<artifact->code_count;++i) {
        uint8_t *word=bytes+HEADER_BYTES+i*WORD_BYTES;
        put64(word,(uint64_t)artifact->code[i].opcode); put64(word+8,(uint64_t)artifact->code[i].a);
        put64(word+16,(uint64_t)artifact->code[i].b); put64(word+24,(uint64_t)artifact->code[i].pad);
    }
    SHA256(bytes,size,bytes+384);
}

static char *parent_directory(const char *path){
    const char *slash=strrchr(path,'/');size_t length;
    if(!slash){path=".";length=1;}else{length=slash==path?1:(size_t)(slash-path);}
    char *parent=malloc(length+1);if(!parent)return NULL;
    memcpy(parent,path,length);parent[length]=0;return parent;
}

static TinyvmArtifactWriteResult publish_atomic(const char *path,const uint8_t *bytes,size_t size){
    static const char suffix[]=".tmp.XXXXXX";
    size_t path_size=strlen(path);
    if(path_size>SIZE_MAX-sizeof(suffix))return TINYVM_ARTIFACT_WRITE_FAILED;
    char *parent=parent_directory(path);
    if(!parent)return TINYVM_ARTIFACT_WRITE_FAILED;
    int directory=open(parent,O_RDONLY|O_DIRECTORY|O_CLOEXEC);free(parent);
    if(directory<0)return TINYVM_ARTIFACT_WRITE_FAILED;
    char *temporary=malloc(path_size+sizeof(suffix));
    if(!temporary){close(directory);return TINYVM_ARTIFACT_WRITE_FAILED;}
    memcpy(temporary,path,path_size);memcpy(temporary+path_size,suffix,sizeof(suffix));
    int descriptor=mkstemp(temporary);
    if(descriptor<0){free(temporary);close(directory);return TINYVM_ARTIFACT_WRITE_FAILED;}
    FILE *file=fdopen(descriptor,"wb");
    bool ok=false;
    if(file){
        ok=fwrite(bytes,1,size,file)==size;
        if(ok&&fflush(file)!=0)ok=false;
        if(ok&&fsync(descriptor)!=0)ok=false;
        if(fclose(file)!=0)ok=false;
    }else close(descriptor);
    if(!ok||rename(temporary,path)!=0){unlink(temporary);free(temporary);close(directory);return TINYVM_ARTIFACT_WRITE_FAILED;}
    free(temporary);
    bool durable=fsync(directory)==0;
    if(close(directory)!=0)durable=false;
    return durable?TINYVM_ARTIFACT_WRITE_PUBLISHED:TINYVM_ARTIFACT_WRITE_DURABILITY_UNCERTAIN;
}

TinyvmArtifactWriteResult tinyvm_artifact_write_result(
    const char *path, TinyvmArtifact *artifact,
    char *diagnostic, size_t capacity) {
    if (!tinyvm_artifact_validate(artifact,diagnostic,capacity)) return TINYVM_ARTIFACT_WRITE_FAILED;
    if (artifact->code_count > (SIZE_MAX-HEADER_BYTES)/WORD_BYTES) { diagnose(diagnostic,capacity,"artifact is too large"); return TINYVM_ARTIFACT_WRITE_FAILED; }
    const size_t size=HEADER_BYTES+artifact->code_count*WORD_BYTES;
    uint8_t *bytes=malloc(size); if(!bytes){diagnose(diagnostic,capacity,"allocation failed");return TINYVM_ARTIFACT_WRITE_FAILED;}
    encode(artifact,bytes,size); memcpy(artifact->digest,bytes+384,32);
    TinyvmArtifactWriteResult result=publish_atomic(path,bytes,size);
    free(bytes);
    diagnose(diagnostic,capacity,result==TINYVM_ARTIFACT_WRITE_PUBLISHED?"valid":
        result==TINYVM_ARTIFACT_WRITE_DURABILITY_UNCERTAIN?
        "artifact published but parent directory durability is uncertain":"artifact write failed");
    return result;
}

bool tinyvm_artifact_write(const char *path, TinyvmArtifact *artifact,
                           char *diagnostic, size_t capacity) {
    return tinyvm_artifact_write_result(path,artifact,diagnostic,capacity)==
           TINYVM_ARTIFACT_WRITE_PUBLISHED;
}

bool tinyvm_artifact_read(const char *path, TinyvmArtifact *artifact,
                          char *diagnostic, size_t capacity) {
    tinyvm_artifact_init(artifact);
    FILE *file=fopen(path,"rb"); if(!file){diagnose(diagnostic,capacity,"artifact open failed");return false;}
    if(fseek(file,0,SEEK_END)!=0){fclose(file);diagnose(diagnostic,capacity,"artifact seek failed");return false;}
    const long end=ftell(file); rewind(file);
    if(end<HEADER_BYTES){fclose(file);diagnose(diagnostic,capacity,"artifact is truncated");return false;}
    const size_t size=(size_t)end; uint8_t *bytes=malloc(size);
    if(!bytes||fread(bytes,1,size,file)!=size||fclose(file)!=0){free(bytes);diagnose(diagnostic,capacity,"artifact read failed");return false;}
    if(memcmp(bytes,magic,8)!=0||get32(bytes+16)!=HEADER_BYTES||get32(bytes+20)!=0||
       bytes[24]!=1||bytes[25]!=WORD_BYTES||!all_zero(bytes+26,6)||!all_zero(bytes+416,96)){
        free(bytes);diagnose(diagnostic,capacity,"artifact header is unsupported");return false;
    }
    const uint64_t count=get64(bytes+40);
    if(count>(SIZE_MAX-HEADER_BYTES)/WORD_BYTES||size!=HEADER_BYTES+(size_t)count*WORD_BYTES){free(bytes);diagnose(diagnostic,capacity,"artifact size disagrees with code count");return false;}
    uint8_t expected[32],stored[32]; memcpy(stored,bytes+384,32); memset(bytes+384,0,32); SHA256(bytes,size,expected);
    if(memcmp(stored,expected,32)!=0){free(bytes);diagnose(diagnostic,capacity,"artifact digest mismatch");return false;}
    artifact->format_version=get32(bytes+8); artifact->isa_version=get32(bytes+12);
    artifact->entrypoint=get64(bytes+32); artifact->code_count=(size_t)count;
    artifact->data_words=get64(bytes+48); artifact->stack_words=get64(bytes+56);
    memcpy(artifact->artifact_id,bytes+64,64); memcpy(artifact->source_id,bytes+128,64);
    memcpy(artifact->target,bytes+192,64); memcpy(artifact->lowering_plan_id,bytes+256,64);
    memcpy(artifact->optimization_id,bytes+320,64); memcpy(artifact->digest,stored,32);
    artifact->code=calloc(artifact->code_count,sizeof(*artifact->code));
    if(!artifact->code){free(bytes);diagnose(diagnostic,capacity,"allocation failed");return false;}
    for(size_t i=0;i<artifact->code_count;++i){const uint8_t *word=bytes+HEADER_BYTES+i*WORD_BYTES;artifact->code[i]=(InstrWord){get_i64(word),get_i64(word+8),get_i64(word+16),get_i64(word+24)};}
    free(bytes); if(!tinyvm_artifact_validate(artifact,diagnostic,capacity)){tinyvm_artifact_destroy(artifact);return false;} return true;
}
