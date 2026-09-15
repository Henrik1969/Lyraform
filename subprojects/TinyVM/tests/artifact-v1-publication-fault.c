#include <tinyvm/artifact.h>

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum FaultMode { FAULT_NONE, FAULT_WRITE, FAULT_SYNC, FAULT_CLOSE, FAULT_RENAME };
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
    if (fault_mode == FAULT_SYNC) return -1;
    return __real_fsync(descriptor);
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

static void require(int condition, const char *message) {
    if (!condition) { fprintf(stderr, "artifact v1 publication fault: %s\n", message); exit(1); }
}

static void set_id(char output[64], const char *value) { snprintf(output, 64, "%s", value); }

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
    static const char prefix[] = "publication-v1.tvm.tmp.";
    DIR *entries = opendir(directory);
    require(entries != NULL, "cannot inspect temporary directory");
    struct dirent *entry;
    while ((entry = readdir(entries)) != NULL)
        require(strncmp(entry->d_name, prefix, sizeof(prefix) - 1) != 0,
                "temporary artifact remained after failure");
    require(closedir(entries) == 0, "cannot close temporary directory");
}

int main(int argc, char **argv) {
    require(argc == 2, "expected build-directory argument");
    char path[1024];
    const int path_length = snprintf(path, sizeof path, "%s/publication-v1.tvm", argv[1]);
    require(path_length > 0 && (size_t)path_length < sizeof path,
            "cannot construct destination path");

    InstrWord code[] = {{OP_ADD, 0, 1, 0}, {OP_HALT, 0, 0, 0}};
    TinyvmArtifact artifact;
    tinyvm_artifact_init(&artifact);
    set_id(artifact.artifact_id, "artifact-identity-v1");
    set_id(artifact.source_id, "source-identity-v1");
    set_id(artifact.target, "tinyvm-portable");
    set_id(artifact.lowering_plan_id, "lowering-plan-v1");
    set_id(artifact.optimization_id, "optimization-v1");
    artifact.code = code;
    artifact.code_count = 2;

    const enum FaultMode failures[] = {FAULT_WRITE, FAULT_SYNC, FAULT_CLOSE, FAULT_RENAME};
    char diagnostic[128];
    for (size_t index = 0; index < sizeof failures / sizeof failures[0]; ++index) {
        write_previous(path);
        fault_mode = failures[index];
        require(!tinyvm_artifact_write(path, &artifact, diagnostic, sizeof diagnostic),
                "injected publication fault was accepted");
        fault_mode = FAULT_NONE;
        require(strcmp(diagnostic, "artifact write failed") == 0,
                "publication fault diagnostic changed");
        require_previous(path);
        require_no_temporary(argv[1]);
    }

    require(tinyvm_artifact_write(path, &artifact, diagnostic, sizeof diagnostic),
            "successful atomic publication failed");
    TinyvmArtifact read;
    tinyvm_artifact_init(&read);
    require(tinyvm_artifact_read(path, &read, diagnostic, sizeof diagnostic),
            "published artifact did not validate");
    tinyvm_artifact_destroy(&read);
    require(remove(path) == 0, "cannot remove successful test artifact");
    require_no_temporary(argv[1]);
    puts("TinyVM artifact v1 atomic publication faults: PASS");
    return 0;
}
