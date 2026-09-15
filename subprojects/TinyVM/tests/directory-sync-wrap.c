#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>

extern int __real_fsync(int);

int __wrap_fsync(int descriptor) {
    struct stat status;
    const char *enabled = getenv("TINYVM_TEST_FAIL_DIRECTORY_SYNC");
    if (enabled != NULL && enabled[0] == '1' &&
        fstat(descriptor, &status) == 0 && S_ISDIR(status.st_mode)) {
        errno = EIO;
        return -1;
    }
    return __real_fsync(descriptor);
}
