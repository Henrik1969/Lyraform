#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>
#include <sched.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <string_view>
#include <stdexcept>
#include <sys/random.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <sys/wait.h>

namespace {

constexpr std::string_view VERSION = "0.1.0";

struct Result { std::string name; bool ok; int error = 0; bool skipped = false; };

Result check_syscall(const char* name, long value) {
    if (value >= 0) return {name, true, 0};
    return {name, false, errno};
}

Result readonly_probe() {
    auto pid = check_syscall("getpid", syscall(SYS_getpid));
    if (!pid.ok) return pid;

    struct timespec clock_value{};
    if (syscall(SYS_clock_gettime, CLOCK_MONOTONIC, &clock_value) < 0) return {"clock_gettime", false, errno};

    struct utsname host{};
    if (syscall(SYS_uname, &host) < 0) return {"uname", false, errno};

    std::uint8_t entropy = 0;
    const auto random_result = syscall(SYS_getrandom, &entropy, sizeof(entropy), GRND_NONBLOCK);
    if (random_result < 0 && errno != EAGAIN) return {"getrandom", false, errno};

    return {"readonly", true, 0};
}

Result tempfs_probe() {
    char directory_template[] = "/tmp/flowcore-kernel-probe-XXXXXX";
    const int directory = mkdtemp(directory_template) == nullptr ? -1 : open(directory_template, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (directory < 0) return {"mkdir/openat", false, errno};

    const int file = openat(directory, "probe.bin", O_CREAT | O_EXCL | O_RDWR | O_CLOEXEC, 0600);
    if (file < 0) { const int saved = errno; close(directory); rmdir(directory_template); return {"openat", false, saved}; }

    const char payload[] = "flowcore-kernel";
    const auto written = write(file, payload, sizeof(payload) - 1);
    if (written != static_cast<ssize_t>(sizeof(payload) - 1)) { const int saved = errno; close(file); unlinkat(directory, "probe.bin", 0); close(directory); rmdir(directory_template); return {"write", false, saved}; }
    if (lseek(file, 0, SEEK_SET) < 0) { const int saved = errno; close(file); unlinkat(directory, "probe.bin", 0); close(directory); rmdir(directory_template); return {"lseek", false, saved}; }

    char observed[sizeof(payload)]{};
    const auto read_count = read(file, observed, sizeof(payload) - 1);
    const bool content_ok = read_count == static_cast<ssize_t>(sizeof(payload) - 1) && std::memcmp(observed, payload, sizeof(payload) - 1) == 0;
    const int saved = content_ok ? 0 : errno;
    close(file);
    const bool unlinked = unlinkat(directory, "probe.bin", 0) == 0;
    close(directory);
    const bool removed = rmdir(directory_template) == 0;
    if (!content_ok) return {"read/verify", false, saved};
    if (!unlinked) return {"unlinkat", false, errno};
    if (!removed) return {"rmdir", false, errno};
    return {"tempfs", true, 0};
}

Result ipc_probe() {
    int channel[2]{};
    if (pipe2(channel, O_CLOEXEC) < 0) return {"pipe2", false, errno};
    const pid_t child = fork();
    if (child < 0) { const int saved = errno; close(channel[0]); close(channel[1]); return {"fork", false, saved}; }
    if (child == 0) {
        close(channel[0]);
        const char message = 'F';
        const auto sent = write(channel[1], &message, 1);
        close(channel[1]);
        _exit(sent == 1 ? 0 : 111);
    }
    close(channel[1]);
    char observed = 0;
    const auto received = read(channel[0], &observed, 1);
    close(channel[0]);
    int child_status = 0;
    if (waitpid(child, &child_status, 0) < 0) return {"waitpid", false, errno};
    if (received != 1 || observed != 'F' || !WIFEXITED(child_status) || WEXITSTATUS(child_status) != 0) return {"pipe/fork verification", false, EIO};
    return {"ipc", true, 0};
}

Result socket_ipc_probe() {
    int pair[2]{};
    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, pair) < 0) return {"socketpair", false, errno};
    const char message = 'N';
    const auto sent = write(pair[1], &message, 1);
    char observed = 0;
    const auto received = read(pair[0], &observed, 1);
    close(pair[0]);
    close(pair[1]);
    if (sent != 1) return {"socket send", false, sent < 0 ? errno : EIO};
    if (received != 1) return {"socket recv", false, received < 0 ? errno : EIO};
    if (observed != 'N') return {"socket payload", false, EIO};
    return {"socket_ipc", true, 0};
}

Result loopback_probe() {
    const int server = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (server < 0) {
        if (errno == EPERM || errno == EACCES) return {"loopback", true, errno, true};
        return {"socket", false, errno};
    }
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(0);
    if (bind(server, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) < 0) { const int saved = errno; close(server); return {"bind loopback", false, saved}; }
    if (listen(server, 1) < 0) { const int saved = errno; close(server); return {"listen", false, saved}; }
    socklen_t address_length = sizeof(address);
    if (getsockname(server, reinterpret_cast<sockaddr*>(&address), &address_length) < 0) { const int saved = errno; close(server); return {"getsockname", false, saved}; }

    const pid_t child = fork();
    if (child < 0) { const int saved = errno; close(server); return {"fork", false, saved}; }
    if (child == 0) {
        const int client = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
        if (client < 0) _exit(121);
        const bool connected = connect(client, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) == 0;
        const char message = 'L';
        const auto sent = connected ? write(client, &message, 1) : -1;
        close(client);
        _exit(sent == 1 ? 0 : 122);
    }

    pollfd wait_for_connection{server, POLLIN, 0};
    const int ready = poll(&wait_for_connection, 1, 2000);
    if (ready <= 0) { const int saved = ready == 0 ? ETIMEDOUT : errno; close(server); waitpid(child, nullptr, 0); return {"poll loopback", false, saved}; }
    const int peer = accept4(server, nullptr, nullptr, SOCK_CLOEXEC);
    close(server);
    if (peer < 0) { const int saved = errno; waitpid(child, nullptr, 0); return {"accept4", false, saved}; }
    char observed = 0;
    const auto received = read(peer, &observed, 1);
    close(peer);
    int child_status = 0;
    if (waitpid(child, &child_status, 0) < 0) return {"waitpid", false, errno};
    if (received != 1 || observed != 'L' || !WIFEXITED(child_status) || WEXITSTATUS(child_status) != 0) return {"loopback verification", false, EIO};
    return {"loopback", true, 0};
}

Result namespaces_probe() {
    int channel[2]{};
    if (pipe2(channel, O_CLOEXEC) < 0) return {"namespace result pipe", false, errno};
    const pid_t child = fork();
    if (child < 0) { const int saved = errno; close(channel[0]); close(channel[1]); return {"namespace fork", false, saved}; }
    if (child == 0) {
        close(channel[0]);
        const int result = unshare(CLONE_NEWUSER | CLONE_NEWUTS | CLONE_NEWIPC);
        int child_errno = result == 0 ? 0 : errno;
        if (result == 0) {
            const char private_hostname[] = "flowkernel-child";
            char observed_hostname[sizeof(private_hostname)]{};
            if (sethostname(private_hostname, sizeof(private_hostname) - 1) < 0 || gethostname(observed_hostname, sizeof(observed_hostname)) < 0 || std::strcmp(observed_hostname, private_hostname) != 0) child_errno = errno == 0 ? EIO : errno;
        }
        write(channel[1], &child_errno, sizeof(child_errno));
        close(channel[1]);
        _exit(result == 0 && child_errno == 0 ? 0 : (child_errno == EPERM || child_errno == EACCES ? 125 : 126));
    }
    close(channel[1]);
    int child_errno = 0;
    const auto received = read(channel[0], &child_errno, sizeof(child_errno));
    close(channel[0]);
    int child_status = 0;
    if (waitpid(child, &child_status, 0) < 0) return {"namespace waitpid", false, errno};
    if (received != static_cast<ssize_t>(sizeof(child_errno))) return {"namespace result", false, EIO};
    if (WIFEXITED(child_status) && WEXITSTATUS(child_status) == 125) return {"namespaces", true, child_errno, true};
    if (!WIFEXITED(child_status) || WEXITSTATUS(child_status) != 0) return {"unshare", false, child_errno == 0 ? EIO : child_errno};
    return {"namespaces", true, 0};
}

void print_result(const Result& result) {
    std::cout << "{\"name\":\"" << result.name << "\",\"status\":\"" << (result.skipped ? "skipped" : (result.ok ? "ok" : "error")) << "\"";
    if (!result.ok || result.skipped) std::cout << ",\"errno\":" << result.error << ",\"message\":\"" << std::strerror(result.error) << "\"";
    std::cout << "}";
}

int run(std::string_view probe, bool require_complete) {
    if (probe != "readonly" && probe != "tempfs" && probe != "ipc" && probe != "socket_ipc" && probe != "loopback" && probe != "namespaces" && probe != "all") throw std::runtime_error("unknown probe; choose readonly, tempfs, ipc, socket_ipc, loopback, namespaces, or all");
    const bool run_readonly = probe == "readonly" || probe == "all";
    const bool run_tempfs = probe == "tempfs" || probe == "all";
    const bool run_ipc = probe == "ipc" || probe == "all";
    const bool run_socket_ipc = probe == "socket_ipc" || probe == "all";
    const bool run_loopback = probe == "loopback" || probe == "all";
    const bool run_namespaces = probe == "namespaces" || probe == "all";
    const auto readonly = run_readonly ? readonly_probe() : Result{"", true, 0};
    const auto tempfs = run_tempfs ? tempfs_probe() : Result{"", true, 0};
    const auto ipc = run_ipc ? ipc_probe() : Result{"", true, 0};
    const auto socket_ipc = run_socket_ipc ? socket_ipc_probe() : Result{"", true, 0};
    const auto loopback = run_loopback ? loopback_probe() : Result{"", true, 0};
    const auto namespaces = run_namespaces ? namespaces_probe() : Result{"", true, 0};
    std::cout << "{\"format\":\"flowkernel.probe_report\",\"version\":1,\"probe\":\"" << probe << "\",\"effects\":[\"read-only-kernel\"" << (run_tempfs ? ",\"private-tempfs\"" : "") << (run_ipc ? ",\"child-process-ipc\"" : "") << (run_socket_ipc ? ",\"local-socket-ipc\"" : "") << (run_loopback ? ",\"local-loopback-network\"" : "") << (run_namespaces ? ",\"child-namespaces\"" : "") << "],\"results\":[";
    bool first = true;
    if (run_readonly) { print_result(readonly); first = false; }
    if (run_tempfs) { if (!first) std::cout << ','; print_result(tempfs); }
    if (run_ipc) { if (!first || run_tempfs) std::cout << ','; print_result(ipc); }
    if (run_socket_ipc) { if (!first || run_tempfs || run_ipc) std::cout << ','; print_result(socket_ipc); }
    if (run_loopback) { if (!first || run_tempfs || run_ipc || run_socket_ipc) std::cout << ','; print_result(loopback); }
    if (run_namespaces) { if (!first || run_tempfs || run_ipc || run_socket_ipc || run_loopback) std::cout << ','; print_result(namespaces); }
    const bool all_ok = readonly.ok && tempfs.ok && ipc.ok && socket_ipc.ok && loopback.ok && namespaces.ok;
    const bool any_skipped = readonly.skipped || tempfs.skipped || ipc.skipped || socket_ipc.skipped || loopback.skipped || namespaces.skipped;
    std::cout << "] ,\"status\":\"" << (all_ok ? (any_skipped ? "ok-with-skips" : "ok") : "error") << "\"}\n";
    if (require_complete && any_skipped) return 3;
    return all_ok ? 0 : 2;
}

}

int main(int argc, char** argv) {
    try {
        if (argc == 2) {
            const std::string option = argv[1];
            if (option == "-h" || option == "--help" || option == "-?") { std::cout << "flowkernel - isolated Linux kernel boundary probes\n\nUsage: flowkernel --probe readonly|tempfs|ipc|socket_ipc|loopback|namespaces|all [--require-complete]\n\nOptions: -h, -?, --help  show help\n         -a, --about    show about information\n         -v, --version  print the raw version number\n         --require-complete  fail if any requested probe is skipped\n\nMore help: Flowkernel/README.md\n"; return 0; }
            if (option == "-a" || option == "--about") { std::cout << "Flowkernel runs explicitly scoped, non-privileged Linux boundary probes.\nMore help: Flowkernel/README.md\n"; return 0; }
            if (option == "-v" || option == "--version") { std::cout << VERSION << '\n'; return 0; }
        }
        const bool require_complete = argc == 4 && std::string(argv[3]) == "--require-complete";
        if ((argc != 3 && !require_complete) || (argc == 4 && !require_complete) || std::string(argv[1]) != "--probe")
            throw std::runtime_error("usage: flowkernel --probe readonly|tempfs|ipc|socket_ipc|loopback|namespaces|all [--require-complete]");
        return run(argv[2], require_complete);
    } catch (const std::exception& error) { std::cerr << "flowkernel error: " << error.what() << '\n'; return 1; }
}
