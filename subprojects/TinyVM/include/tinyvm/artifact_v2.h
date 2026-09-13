#pragma once

#include <tinyvm/artifact.h>

typedef enum {
    TINYVM_CARRIER_I1 = 1,
    TINYVM_CARRIER_I32 = 2,
    TINYVM_CARRIER_I64 = 3,
    TINYVM_CARRIER_OPAQUE_HANDLE = 4,
    TINYVM_CARRIER_TEXT_OUTCOME = 5
} TinyvmCarrier;

typedef struct { uint64_t id; uint32_t carrier; uint64_t bits; } TinyvmConstant;
typedef struct { uint64_t id; uint8_t *bytes; size_t length; } TinyvmString;
typedef struct { uint64_t id, bytes, alignment; uint32_t kind; } TinyvmStorage;
typedef struct {
    uint64_t id;
    char contract[64], library[64], convention[64], symbol[64];
    char effect[64], parameters[64], result[64], evidence[64];
} TinyvmImport;
typedef struct {
    uint64_t instruction, operation, block, symbol;
    uint32_t line, column;
    char source[64], derivation[64];
} TinyvmProvenance;

typedef struct {
    uint32_t isa_version;
    uint64_t entrypoint, data_words, stack_words;
    char artifact_id[64], source_id[64], target_policy_id[64];
    char lowering_plan_id[64], optimization_id[64];
    uint8_t digest[32];
    InstrWord *code; size_t code_count;
    TinyvmConstant *constants; size_t constant_count;
    TinyvmString *strings; size_t string_count;
    TinyvmStorage *storage; size_t storage_count;
    TinyvmImport *imports; size_t import_count;
    TinyvmProvenance *provenance; size_t provenance_count;
} TinyvmArtifactV2;

void tinyvm_artifact_v2_init(TinyvmArtifactV2 *artifact);
void tinyvm_artifact_v2_destroy(TinyvmArtifactV2 *artifact);
bool tinyvm_artifact_v2_validate(const TinyvmArtifactV2 *artifact,
                                 char *diagnostic, size_t capacity);
bool tinyvm_artifact_v2_write(const char *path, TinyvmArtifactV2 *artifact,
                              char *diagnostic, size_t capacity);
bool tinyvm_artifact_v2_read(const char *path, TinyvmArtifactV2 *artifact,
                             char *diagnostic, size_t capacity);
