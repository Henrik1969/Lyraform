#pragma once

#include <tinyvm/tinyvm.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TINYVM_ARTIFACT_FORMAT_VERSION 1U
#define TINYVM_RECOVERED_ISA_VERSION 0U
#define TINYVM_ARTIFACT_ID_CAPACITY 64U

typedef struct {
    uint32_t format_version;
    uint32_t isa_version;
    uint64_t entrypoint;
    uint64_t data_words;
    uint64_t stack_words;
    char artifact_id[TINYVM_ARTIFACT_ID_CAPACITY];
    char source_id[TINYVM_ARTIFACT_ID_CAPACITY];
    char target[TINYVM_ARTIFACT_ID_CAPACITY];
    char lowering_plan_id[TINYVM_ARTIFACT_ID_CAPACITY];
    char optimization_id[TINYVM_ARTIFACT_ID_CAPACITY];
    uint8_t digest[32];
    InstrWord *code;
    size_t code_count;
} TinyvmArtifact;

typedef enum {
    TINYVM_ARTIFACT_WRITE_FAILED = 0,
    TINYVM_ARTIFACT_WRITE_PUBLISHED = 1,
    TINYVM_ARTIFACT_WRITE_DURABILITY_UNCERTAIN = 2
} TinyvmArtifactWriteResult;

void tinyvm_artifact_init(TinyvmArtifact *artifact);
void tinyvm_artifact_destroy(TinyvmArtifact *artifact);
bool tinyvm_artifact_validate(const TinyvmArtifact *artifact,
                              char *diagnostic, size_t capacity);
bool tinyvm_artifact_write(const char *path, TinyvmArtifact *artifact,
                           char *diagnostic, size_t capacity);
TinyvmArtifactWriteResult tinyvm_artifact_write_result(
    const char *path, TinyvmArtifact *artifact,
    char *diagnostic, size_t capacity);
bool tinyvm_artifact_read(const char *path, TinyvmArtifact *artifact,
                          char *diagnostic, size_t capacity);
bool tinyvm_validate_recovered_code(const InstrWord *code, size_t code_count,
                                    uint64_t entrypoint, uint64_t data_words,
                                    uint64_t stack_words,
                                    char *diagnostic, size_t capacity);
