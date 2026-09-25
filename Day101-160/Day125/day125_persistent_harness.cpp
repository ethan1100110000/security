#include "tinyxml2.h"
#include <unistd.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
__AFL_FUZZ_INIT();

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

#ifdef __AFL_HAVE_MANUAL_CONTROL
	__AFL_INIT();
#endif
	unsigned char* afl_buffer = __AFL_FUZZ_TESTCASE_BUF;

	while(__AFL_LOOP(1000)) {
		const size_t input_size = static_cast<size_t>(__AFL_FUZZ_TESTCASE_LEN);

		if(input_size > MAX_INPUT_SIZE) {
			continue;
		}
		
		test_one_input(reinterpret_cast<const uint8_t*>(afl_buffer), input_size);
	}

	return 0;
}
