#include <enoki-thread/thread.h>
#include <stdexcept>

#if defined(_WIN32)
#  include <windows.h>
#else
#  include <unistd.h>
#endif

void my_sleep(uint32_t amt) {
#if defined(_WIN32)
    Sleep(amt);
#else
    usleep(amt * 1000);
#endif
}

namespace ek = enoki;

void test01() {
    try {
        ek::parallel_for(
            ek::blocked_range<uint32_t>(0, 1000, 5),
            [](ek::blocked_range<uint32_t> range) {
                throw std::runtime_error("Hello world!");
            }
        );
    } catch (std::exception &e) {
        printf("Test 1: success: %s\n", e.what());
        return;
    }
    abort();
}

void test02(bool wait) {
    auto work1 = ek::parallel_for_async(
        ek::blocked_range<uint32_t>(0, 10, 1),
        [](ek::blocked_range<uint32_t> range) {
            my_sleep(10);
            throw std::runtime_error("Hello world!");
        }
    );

    if (wait)
        my_sleep(100);

    auto work2 = ek::parallel_for_async(
        ek::blocked_range<uint32_t>(0, 10, 1),
        [](ek::blocked_range<uint32_t> range) {
            printf("Should never get here!\n");
            abort();
        },
        { work1 }
    );

    task_release(work1);

    try {
        task_wait_and_release(work2);
    } catch (std::exception &e) {
        printf("Test 2: success: %s\n", e.what());
        return;
    }
    abort();
}

int main(int arc, char** argv) {
    test01();
    test02(false);
    test02(true);
}
