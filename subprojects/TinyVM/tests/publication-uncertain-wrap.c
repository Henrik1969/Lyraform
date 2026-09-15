#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int selected(const char *name) {
    const char *mode = getenv("TINYVM_TEST_PUBLICATION_FAULT");
    return mode != NULL && strcmp(mode, name) == 0;
}

extern size_t __real_fwrite(const void *, size_t, size_t, FILE *);
size_t __wrap_fwrite(const void *data, size_t width, size_t count, FILE *file) {
    if (selected("abrupt-write") && fileno(file) > STDERR_FILENO) _exit(86);
    return __real_fwrite(data, width, count, file);
}

extern int __real_fsync(int);
int __wrap_fsync(int descriptor) {
    struct stat status;
    if (selected("directory-sync") && fstat(descriptor, &status) == 0 &&
        S_ISDIR(status.st_mode)) {
        errno = EIO;
        return -1;
    }
    return __real_fsync(descriptor);
}

extern int __real_close(int);
int __wrap_close(int descriptor) {
    struct stat status;
    const int fail = selected("directory-close") &&
                     fstat(descriptor, &status) == 0 && S_ISDIR(status.st_mode);
    const int result = __real_close(descriptor);
    if (fail) {
        errno = EIO;
        return -1;
    }
    return result;
}
