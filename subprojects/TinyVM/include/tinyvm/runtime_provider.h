#pragma once

#include <tinyvm/isa_v1.h>

typedef struct {
    char *bytes;
    size_t length;
} TinyvmRuntimeString;

typedef struct {
    unsigned char *bytes;
    unsigned char *initialized;
    size_t length;
} TinyvmRuntimeStorage;

typedef struct {
    const char *policy_path;
    size_t argument_count;
    const char *const *arguments;
    TinyvmRuntimeString *owned_strings;
    size_t owned_string_count;
    size_t owned_string_capacity;
    TinyvmRuntimeStorage *storage;
    size_t storage_count;
    void *text_outcomes;
    size_t text_outcome_count;
    size_t text_outcome_capacity;
    int *file_descriptors;
    size_t file_descriptor_count;
    size_t file_descriptor_capacity;
} TinyvmRuntimeProvider;

void tinyvm_runtime_provider_destroy(TinyvmRuntimeProvider *provider);

bool tinyvm_runtime_provider_preflight(const TinyvmRuntimeProvider *provider,
                                       const TinyvmArtifactV2 *artifact,
                                       const char **fault);

bool tinyvm_runtime_provider_resolve(void *user,
                                     const TinyvmArtifactV2 *artifact,
                                     const TinyvmImport *import,
                                     const TinyvmValue *arguments,
                                     size_t argument_count,
                                     TinyvmValue *result,
                                     const char **fault);
bool tinyvm_runtime_provider_resolve_text_outcome(void *user,
                                                  const TinyvmValue *outcome,
                                                  bool value_field,
                                                  TinyvmValue *result,
                                                  const char **fault);
