#include "EngineTest.h"
#include "ThreadPool.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(ThreadPool)

uint64 basicAwaitState = 0;

Job basicAwaitFirst()
{
    BOOST_REQUIRE_EQUAL(basicAwaitState, 5);
    basicAwaitState = 10;
    co_return;
}
Job basicAwaitSecond()
{
    BOOST_REQUIRE_EQUAL(basicAwaitState, 15);
    basicAwaitState = 20;
    co_return;
}
Job basicAwaitThird()
{
    BOOST_REQUIRE_EQUAL(basicAwaitState, 25);
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
    basicThenState = 10;
    co_return;
}
Job basicThenSecond()
{
    BOOST_REQUIRE_EQUAL(basicThenState, 15);
    basicThenState = 20;
    co_return;
}
Job basicThenThird()
{
    BOOST_REQUIRE_EQUAL(basicThenState, 25);
    basicThenState = 30;
    co_return;
}

BOOST_AUTO_TEST_CASE(basic_thenchain)
{
    basicThenFirst()
        .then([]() -> Job
        {
            BOOST_REQUIRE_EQUAL(basicThenState, 10);
            basicThenState = 15;
            co_return;
        })
        .then(basicThenSecond())
        .then([]() -> Job
        {
            BOOST_REQUIRE_EQUAL(basicThenState, 20);
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
    basicAllState1 = 10;
    co_return;
}
Job basicAllSecond()
{
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

uint64 basicCallable = 0;

Job basicCallableFunc()
{
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

BOOST_AUTO_TEST_SUITE_END()