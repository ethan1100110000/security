#include "tinyxml2.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <array>
#include <unistd.h>
#include <cerrno>
#include <fcntl.h>

static int test_one_input(const uint8_t* data, size_t size)
{
    tinyxml2::XMLDocument doc;

    const tinyxml2::XMLError error =
        doc.Parse(reinterpret_cast<const char*>(data), size);

#ifdef HARNESS_DEBUG
    std::fprintf(
        stderr,
        "size=%zu error=%s(%d)\n",
        size,
        doc.ErrorName(),
        static_cast<int>(error)
    );
#endif
    (void)error;
    return 0;
}

static constexpr size_t MAX_INPUT_SIZE = 1024 * 1024;

enum class ReadStatus {
    Ok,
    TooLarge,
    Error
};

static ReadStatus read_all(
    int fd,
    std::array<uint8_t, MAX_INPUT_SIZE + 1>& buffer,
    size_t& total
)
{
    total = 0;

    while (total < buffer.size()) {
        const ssize_t result = read(
            fd,
            buffer.data() + total,
            buffer.size() - total
        );

        if (result > 0) {
            total += static_cast<size_t>(result);
            continue;
        }

        if (result == 0) {
            break;
        }

        if (errno == EINTR) {
            continue;
        }

        std::perror("read");
        return ReadStatus::Error;
    }

    if (total > MAX_INPUT_SIZE) {
        return ReadStatus::TooLarge;
    }

    return ReadStatus::Ok;
}

int main(int argc, char* argv[])
{
    int fd = STDIN_FILENO;
    bool close_fd = false;

    if (argc == 2) {
        fd = open(argv[1], O_RDONLY);

        if (fd < 0) {
            std::perror("open");
            return 1;
        }

        close_fd = true;
    }
    else if (argc != 1) {
        std::fprintf(
            stderr,
            "usage: %s [input_file]\n",
            argv[0]
        );
        return 2;
    }

    std::array<uint8_t, MAX_INPUT_SIZE + 1> buffer{};
    size_t input_size = 0;

    const ReadStatus status =
        read_all(fd, buffer, input_size);

    if (close_fd && close(fd) < 0) {
        std::perror("close");
        return 1;
    }

    if (status == ReadStatus::Error) {
        return 1;
    }

    if (status == ReadStatus::TooLarge) {
        std::fprintf(
            stderr,
            "input too large: maximum=%zu\n",
            MAX_INPUT_SIZE
        );
        return 1;
    }

    return test_one_input(
        buffer.data(),
        input_size
    );
}
