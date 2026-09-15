#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>

namespace {

bool selected(const char* name) {
    const char* mode = std::getenv("FLOWMINI_PUBLICATION_FAULT");
    return mode && std::strcmp(mode, name) == 0;
}

bool private_file(FILE* file) { return fileno(file) > STDERR_FILENO; }

}

extern "C" size_t __real_fwrite(const void*, size_t, size_t, FILE*);
extern "C" size_t __wrap_fwrite(const void* data, size_t width, size_t count, FILE* file) {
    if (private_file(file) && selected("write") && width && count) {
        const size_t bytes = width * count;
        return __real_fwrite(data, 1, bytes > 1 ? bytes / 2 : 0, file);
    }
    return __real_fwrite(data, width, count, file);
}

extern "C" int __real_fsync(int);
extern "C" int __wrap_fsync(int descriptor) {
    struct stat status {};
    const bool directory = fstat(descriptor, &status) == 0 && S_ISDIR(status.st_mode);
    if (descriptor > STDERR_FILENO &&
        ((selected("sync") && !directory) || (selected("directory-sync") && directory))) return -1;
    return __real_fsync(descriptor);
}

extern "C" int __real_close(int);
extern "C" int __wrap_close(int descriptor) {
    struct stat status {};
    const bool directory = fstat(descriptor, &status) == 0 && S_ISDIR(status.st_mode);
    const int result = __real_close(descriptor);
    return descriptor > STDERR_FILENO && directory && selected("directory-close") ? -1 : result;
}

extern "C" int __real_fclose(FILE*);
extern "C" int __wrap_fclose(FILE* file) {
    const bool fail = private_file(file) && selected("close");
    const int result = __real_fclose(file);
    return fail ? EOF : result;
}

extern "C" int __real_rename(const char*, const char*);
extern "C" int __wrap_rename(const char* source, const char* destination) {
    if (selected("rename")) return -1;
    return __real_rename(source, destination);
}
