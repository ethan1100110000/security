#include "tinyxml2.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <array>
#include <unistd.h>


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

int main()
{
    std::array<uint8_t, MAX_INPUT_SIZE> buffer{};

    const ssize_t bytes_read =
        read(STDIN_FILENO, buffer.data(), buffer.size());

    if (bytes_read < 0) {
        std::perror("read");
        return 1;
    }

    return test_one_input(
        buffer.data(),
        static_cast<size_t>(bytes_read)
    );
}
