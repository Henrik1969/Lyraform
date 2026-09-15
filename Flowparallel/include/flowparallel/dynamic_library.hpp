#pragma once

#include <cerrno>
#include <dlfcn.h>
#include <stdexcept>

namespace flowparallel {

// Owns one dlopen handle. close() is the observable cleanup boundary; the
// destructor is only a last-resort leak guard and cannot publish its status.
class DynamicLibrary {
public:
    using Close = int (*)(void*);

    explicit DynamicLibrary(const char* name)
        : handle_(dlopen(name, RTLD_NOW | RTLD_LOCAL)), close_(::dlclose) {
        if (!handle_) throw std::runtime_error("dynamic library load failed");
    }

    DynamicLibrary(void* handle, Close close) noexcept
        : handle_(handle), close_(close) {}

    ~DynamicLibrary() { (void)close(); }
    DynamicLibrary(const DynamicLibrary&) = delete;
    DynamicLibrary& operator=(const DynamicLibrary&) = delete;

    int close() noexcept {
        if (!handle_) return close_status_;
        if (!close_) {
            handle_ = nullptr;
            close_status_ = EINVAL;
            return close_status_;
        }
        try {
            close_status_ = close_(handle_);
        } catch (...) {
            close_status_ = EFAULT;
        }
        handle_ = nullptr;
        return close_status_;
    }

    void* handle() const noexcept { return handle_; }
    int close_status() const noexcept { return close_status_; }

    template <typename Function>
    Function symbol(const char* name) const {
        auto* value = dlsym(handle_, name);
        if (!value) throw std::runtime_error("dynamic library symbol lookup failed");
        return reinterpret_cast<Function>(value);
    }

private:
    void* handle_ = nullptr;
    Close close_ = nullptr;
    int close_status_ = 0;
};

} // namespace flowparallel
