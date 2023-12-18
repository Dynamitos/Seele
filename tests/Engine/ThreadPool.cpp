#include "EngineTest.h"
#include "ThreadPool.h"
#include <numeric>

TEST(ThreadPool, RunBatch)
{
    ThreadPool t(10);
    uint32 test = 20;
    std::mutex m;
    List<Task> work;
    for (uint32 i = 0; i < 400; ++i)
    {
        work.add([&]() -> Task {
            std::unique_lock l(m);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            test++;
            co_return;
        }());
    }
    t.runAndWait(std::move(work));
    ASSERT_EQ(test, 420);
}