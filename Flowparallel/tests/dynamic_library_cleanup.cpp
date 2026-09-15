#include <flowparallel/dynamic_library.hpp>

#include <cassert>

namespace {
int close_calls = 0;
int close_result = 0;
int fake_close(void*) {
    ++close_calls;
    return close_result;
}
}

int main() {
    close_calls = 0;
    close_result = 17;
    flowparallel::DynamicLibrary failed{reinterpret_cast<void*>(1), fake_close};
    assert(failed.close() == 17);
    assert(failed.close_status() == 17);
    assert(failed.handle() == nullptr);
    assert(failed.close() == 17);
    assert(close_calls == 1);

    close_calls = 0;
    close_result = 0;
    flowparallel::DynamicLibrary clean{reinterpret_cast<void*>(1), fake_close};
    assert(clean.close() == 0);
    assert(clean.handle() == nullptr);
    assert(close_calls == 1);
    return 0;
}
