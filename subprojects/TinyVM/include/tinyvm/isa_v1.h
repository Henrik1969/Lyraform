#pragma once

#include <tinyvm/artifact_v2.h>

typedef enum {
    TV1_NOP = 0,
    TV1_CONST = 1,
    TV1_MOVE = 2,
    TV1_CONVERT = 3,
    TV1_ADD = 4,
    TV1_SUB = 5,
    TV1_MUL = 6,
    TV1_SDIV = 7,
    TV1_CMP_EQ = 8,
    TV1_CMP_NE = 9,
    TV1_CMP_LT = 10,
    TV1_CMP_LE = 11,
    TV1_CMP_GT = 12,
    TV1_CMP_GE = 13,
    TV1_JMP = 14,
    TV1_BRANCH = 15,
    TV1_RETURN = 16,
    TV1_TRAP = 17,
    TV1_HALT = 18,
    TV1_STRING_HANDLE = 19,
    TV1_STORAGE_HANDLE = 20,
    TV1_CALL_IMPORT = 21,
    TV1_TEXT_OUTCOME_CODE = 22,
    TV1_TEXT_OUTCOME_VALUE = 23,
    TV1_GRAPH_ACTIVATE = 24,
    TV1_OPCODE_COUNT
} TinyvmIsaV1Opcode;

typedef enum {
    TV1_TRAP_EXPLICIT = 1,
    TV1_TRAP_UNINITIALIZED_SLOT = 2,
    TV1_TRAP_TYPE_MISMATCH = 3,
    TV1_TRAP_DIVISION_BY_ZERO = 4,
    TV1_TRAP_DIVISION_OVERFLOW = 5,
    TV1_TRAP_ARITHMETIC_OVERFLOW = 6,
    TV1_TRAP_UNRESOLVED_IMPORT = 7,
    TV1_TRAP_STEP_LIMIT = 8
} TinyvmIsaV1Trap;

typedef struct {
    uint32_t carrier;
    uint64_t bits;
    bool initialized;
} TinyvmValue;

typedef bool (*TinyvmImportResolver)(void *user,
                                    const TinyvmArtifactV2 *artifact,
                                    const TinyvmImport *import,
                                    const TinyvmValue *arguments,
                                    size_t argument_count,
                                    TinyvmValue *result,
                                    const char **fault);

typedef bool (*TinyvmTextOutcomeResolver)(void *user,
                                          const TinyvmValue *outcome,
                                          bool value_field,
                                          TinyvmValue *result,
                                          const char **fault);

typedef struct {
    TinyvmGraphActivation identity;
    uint64_t sequence;
    uint64_t stream_index;
} TinyvmGraphActivationRecord;

typedef void (*TinyvmGraphActivationObserver)(void *user,
                                              const TinyvmArtifactV2 *artifact,
                                              const TinyvmGraphActivationRecord *record);

typedef struct {
    TinyvmValue *slots;
    size_t slot_count;
    uint64_t pc;
    uint64_t steps;
    uint64_t step_limit;
    bool running;
    bool returned;
    TinyvmValue result;
    uint32_t trap;
    uint64_t trap_instruction;
    const char *fault;
    size_t argument_count;
    const char *const *arguments;
    TinyvmImportResolver import_resolver;
    void *import_user;
    TinyvmTextOutcomeResolver text_outcome_resolver;
    void *text_outcome_user;
    TinyvmGraphActivationObserver graph_observer;
    void *graph_observer_user;
    TinyvmGraphActivationRecord *graph_records;
    size_t graph_record_count;
    size_t graph_record_capacity;
} TinyvmIsaV1Context;

bool tinyvm_isa_v1_validate(const TinyvmArtifactV2 *artifact,
                            char *diagnostic, size_t capacity);
bool tinyvm_isa_v1_context_init(TinyvmIsaV1Context *context,
                                size_t slot_count, uint64_t step_limit);
void tinyvm_isa_v1_context_destroy(TinyvmIsaV1Context *context);
bool tinyvm_isa_v1_run_switch(const TinyvmArtifactV2 *artifact,
                              TinyvmIsaV1Context *context);
bool tinyvm_isa_v1_run_computed(const TinyvmArtifactV2 *artifact,
                                TinyvmIsaV1Context *context);
