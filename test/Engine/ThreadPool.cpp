#include "EngineTest.h"
#include "ThreadPool.h"
#include <boost/test/unit_test.hpp>
#include <numeric>

using namespace std::chrono_literals;

BOOST_AUTO_TEST_SUITE(ThreadPool)

uint64 basicAwaitState = 0;

Job basicAwaitFirst()
{
    BOOST_REQUIRE_EQUAL(basicAwaitState, 5);
    std::this_thread::sleep_for(500ms);
    basicAwaitState = 10;
    co_return;
}
Job basicAwaitSecond()
{
    BOOST_REQUIRE_EQUAL(basicAwaitState, 15);
    std::this_thread::sleep_for(500ms);
    basicAwaitState = 20;
    co_return;
}
Job basicAwaitThird()
{
    BOOST_REQUIRE_EQUAL(basicAwaitState, 25);
    std::this_thread::sleep_for(500ms);
    basicAwaitState = 30;
    co_return;
}

Job basicAwaitBase()
{
    basicAwaitState = 5;
    co_await basicAwaitFirst();
    BOOST_REQUIRE_EQUAL(basicAwaitState, 10);
    basicAwaitState = 15;
    co_await basicAwaitSecond();
    BOOST_REQUIRE_EQUAL(basicAwaitState, 20);
    basicAwaitState = 25;
    co_await basicAwaitThird();
    BOOST_REQUIRE_EQUAL(basicAwaitState, 30);
}

BOOST_AUTO_TEST_CASE(basic_coawait)
{
    basicAwaitBase();
}

uint64 basicThenState = 5;

Job basicThenFirst()
{
    BOOST_REQUIRE_EQUAL(basicThenState, 5);
    std::this_thread::sleep_for(500ms);
    basicThenState = 10;
    co_return;
}
Job basicThenSecond()
{
    BOOST_REQUIRE_EQUAL(basicThenState, 15);
    std::this_thread::sleep_for(500ms);
    basicThenState = 20;
    co_return;
}
Job basicThenThird()
{
    BOOST_REQUIRE_EQUAL(basicThenState, 25);
    std::this_thread::sleep_for(500ms);
    basicThenState = 30;
    co_return;
}

BOOST_AUTO_TEST_CASE(basic_thenchain)
{
    basicThenFirst()
        .then([]() -> Job
        {
            BOOST_REQUIRE_EQUAL(basicThenState, 10);
            std::this_thread::sleep_for(500ms);
            basicThenState = 15;
            co_return;
        })
        .then(basicThenSecond())
        .then([]() -> Job
        {
            BOOST_REQUIRE_EQUAL(basicThenState, 20);
            std::this_thread::sleep_for(500ms);
            basicThenState = 25;
            co_return;
        })
        .then(basicThenThird())
        .then([]() -> Job
        {
            BOOST_REQUIRE_EQUAL(basicThenState, 30);
            co_return;
        });
}

uint64 basicAllState1 = 0;
uint64 basicAllState2 = 0;
Job basicAllFirst()
{
    std::this_thread::sleep_for(500ms);
    basicAllState1 = 10;
    co_return;
}
Job basicAllSecond()
{
    std::this_thread::sleep_for(500ms);
    basicAllState2 = 10;
    co_return;
}
Job basicAllThen()
{
    BOOST_REQUIRE_EQUAL(basicAllState1, 10);
    BOOST_REQUIRE_EQUAL(basicAllState2, 10);
    co_return;
}

BOOST_AUTO_TEST_CASE(basic_all)
{
    Job::all(basicAllFirst(), basicAllSecond()).then(basicAllThen());
}

uint64 allThenState = 0;

Job allThenInitial()
{
    std::this_thread::sleep_for(500ms);
    allThenState = 10;
    co_return;
}

Job allThenIntermediate()
{
    std::this_thread::sleep_for(500ms);
    BOOST_REQUIRE_EQUAL(allThenState, 10);
    allThenState = 20;
    co_return;
}

Job allThenFinal()
{
    BOOST_REQUIRE_EQUAL(allThenState, 20);
    co_return;
}

BOOST_AUTO_TEST_CASE(all_then_interaction)
{
    Job::all(allThenInitial())
        .then(allThenIntermediate())
        .then(Job::all(allThenFinal()));
}

uint64 basicCallable = 0;

Job basicCallableFunc()
{
    std::this_thread::sleep_for(500ms);
    basicCallable = 10;
    co_return;
}

BOOST_AUTO_TEST_CASE(basic_callable)
{
    basicCallableFunc()
        .then([=]() -> Job
        {
            BOOST_REQUIRE_EQUAL(basicCallable, 10);
            co_return;
        });
}

struct Payload
{
    StaticArray<uint64, 1000> data;
    uint64 result = 0;
};

Job worker(Payload& payload)
{
    for(uint32 i = 0; i < 100000; ++i)
    {
        payload.result = std::accumulate(payload.data.begin(), payload.data.end(), (uint64)1, std::multiplies<uint64>());
    }
    co_return;
}

Job launchStressTest()
{
    Array<Payload> payloads(100);
    Array<Job> jobs;
    for(auto& payload : payloads)
    {
        for(uint64 i = 0; i < payload.data.size(); ++i)
        {
            payload.data[i] = i;
        }
    }
    auto startTime = std::chrono::high_resolution_clock::now();
    std::cout << "Starting up" << std::endl;
    co_await Job::launchJobs(&worker, payloads);
    std::cout << "Finished waiting" << std::endl;
    auto endTime = std::chrono::high_resolution_clock::now();
    float delta = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
    std::cout << "Update took " << delta << " seconds";
}

BOOST_AUTO_TEST_CASE(stress_test)
{
    //launchStressTest();
}

BOOST_AUTO_TEST_SUITE_END()