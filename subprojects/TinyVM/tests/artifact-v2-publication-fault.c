#include <tinyvm/artifact_v2.h>

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

enum FaultMode {
    FAULT_NONE,
    FAULT_WRITE,
    FAULT_SYNC,
    FAULT_CLOSE,
    FAULT_RENAME,
    FAULT_DIRECTORY_SYNC,
    FAULT_DIRECTORY_CLOSE
};
static enum FaultMode fault_mode = FAULT_NONE;

extern size_t __real_fwrite(const void *, size_t, size_t, FILE *);
size_t __wrap_fwrite(const void *data, size_t width, size_t count, FILE *file) {
    if (fault_mode == FAULT_WRITE && width && count) {
        const size_t bytes = width * count;
        return __real_fwrite(data, 1, bytes > 1 ? bytes / 2 : 0, file);
    }
    return __real_fwrite(data, width, count, file);
}

extern int __real_fsync(int);
int __wrap_fsync(int descriptor) {
    struct stat status;
    const int is_directory = fstat(descriptor, &status) == 0 && S_ISDIR(status.st_mode);
    if ((fault_mode == FAULT_SYNC && !is_directory) ||
        (fault_mode == FAULT_DIRECTORY_SYNC && is_directory)) return -1;
    return __real_fsync(descriptor);
}

extern int __real_close(int);
int __wrap_close(int descriptor) {
    struct stat status;
    const int is_directory = fstat(descriptor, &status) == 0 && S_ISDIR(status.st_mode);
    const int result = __real_close(descriptor);
    return fault_mode == FAULT_DIRECTORY_CLOSE && is_directory ? -1 : result;
}

extern int __real_fclose(FILE *);
int __wrap_fclose(FILE *file) {
    const int result = __real_fclose(file);
    return fault_mode == FAULT_CLOSE ? EOF : result;
}

extern int __real_rename(const char *, const char *);
int __wrap_rename(const char *source, const char *destination) {
    if (fault_mode == FAULT_RENAME) return -1;
    return __real_rename(source, destination);
}

static void set_id(char output[64], const char *value) { snprintf(output, 64, "%s", value); }

static void require(int condition, const char *message) {
    if (!condition) { fprintf(stderr, "artifact v2 publication fault: %s\n", message); exit(1); }
}

static void write_previous(const char *path) {
    FILE *file = fopen(path, "wb");
    require(file != NULL, "cannot create prior destination");
    require(fwrite("previous\n", 1, 9, file) == 9, "cannot write prior destination");
    require(fclose(file) == 0, "cannot close prior destination");
}

static void require_previous(const char *path) {
    char bytes[10] = {0};
    FILE *file = fopen(path, "rb");
    require(file != NULL, "prior destination disappeared");
    require(fread(bytes, 1, 9, file) == 9, "prior destination was truncated");
    require(fclose(file) == 0, "cannot close prior destination after inspection");
    require(memcmp(bytes, "previous\n", 9) == 0, "prior destination was replaced");
}

static void require_no_temporary(const char *directory) {
    DIR *entries = opendir(directory);
    require(entries != NULL, "cannot inspect temporary directory");
    struct dirent *entry;
    while ((entry = readdir(entries)) != NULL)
        require(strncmp(entry->d_name, "publication.tvm.tmp.", 20) != 0,
                "temporary artifact remained after failure");
    require(closedir(entries) == 0, "cannot close temporary directory");
}

int main(int argc, char **argv) {
    require(argc == 2, "expected build-directory argument");
    char path[1024];
    require(snprintf(path, sizeof path, "%s/publication.tvm", argv[1]) > 0,
            "cannot construct destination path");

    InstrWord code[] = {{OP_ADD, 0, 1, 0}, {OP_HALT, 0, 0, 0}};
    TinyvmProvenance provenance[2] = {0};
    for (size_t index = 0; index < 2; ++index) {
        provenance[index].instruction = index;
        provenance[index].operation = index + 1;
        provenance[index].block = 1;
        provenance[index].line = (uint32_t)(10 + index);
        provenance[index].column = 3;
        set_id(provenance[index].source, "source-identity-1");
        set_id(provenance[index].derivation, "lowering-derivation-1");
    }
    TinyvmArtifactV2 artifact;
    tinyvm_artifact_v2_init(&artifact);
    set_id(artifact.artifact_id, "artifact-identity-1");
    set_id(artifact.source_id, "source-identity-1");
    set_id(artifact.target_policy_id, "tinyvm-portable");
    set_id(artifact.lowering_plan_id, "lowering-plan-1");
    set_id(artifact.optimization_id, "optimization-1");
    artifact.code = code;
    artifact.code_count = 2;
    artifact.provenance = provenance;
    artifact.provenance_count = 2;

    const enum FaultMode failures[] = {FAULT_WRITE, FAULT_SYNC, FAULT_CLOSE, FAULT_RENAME};
    char diagnostic[128];
    for (size_t index = 0; index < sizeof failures / sizeof failures[0]; ++index) {
        write_previous(path);
        fault_mode = failures[index];
        require(!tinyvm_artifact_v2_write(path, &artifact, diagnostic, sizeof diagnostic),
                "injected publication fault was accepted");
        fault_mode = FAULT_NONE;
        require(strcmp(diagnostic, "artifact write failed") == 0,
                "publication fault diagnostic changed");
        require_previous(path);
        require_no_temporary(argv[1]);
    }

    const enum FaultMode uncertain_failures[] = {FAULT_DIRECTORY_SYNC, FAULT_DIRECTORY_CLOSE};
    for (size_t index = 0; index < sizeof uncertain_failures / sizeof uncertain_failures[0]; ++index) {
        write_previous(path);
        fault_mode = uncertain_failures[index];
        require(tinyvm_artifact_v2_write_result(path, &artifact, diagnostic, sizeof diagnostic) ==
                    TINYVM_ARTIFACT_WRITE_DURABILITY_UNCERTAIN,
                "directory finalization fault did not produce an uncertain result");
        fault_mode = FAULT_NONE;
        require(strcmp(diagnostic, "artifact published but parent directory durability is uncertain") == 0,
                "directory finalization diagnostic changed");
        TinyvmArtifactV2 uncertain;
        tinyvm_artifact_v2_init(&uncertain);
        require(tinyvm_artifact_v2_read(path, &uncertain, diagnostic, sizeof diagnostic),
                "uncertain published artifact is not visible and valid");
        tinyvm_artifact_v2_destroy(&uncertain);
        require_no_temporary(argv[1]);
    }

    require(tinyvm_artifact_v2_write(path, &artifact, diagnostic, sizeof diagnostic),
            "successful atomic publication failed");
    TinyvmArtifactV2 read;
    tinyvm_artifact_v2_init(&read);
    require(tinyvm_artifact_v2_read(path, &read, diagnostic, sizeof diagnostic),
            "published artifact did not validate");
    tinyvm_artifact_v2_destroy(&read);
    require(remove(path) == 0, "cannot remove successful test artifact");
    require_no_temporary(argv[1]);
    puts("TinyVM artifact v2 atomic publication faults: PASS");
    return 0;
}
