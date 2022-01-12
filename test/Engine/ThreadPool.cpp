#include "EngineTest.h"
#include "ThreadPool.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(ThreadPool)

uint64 basicJobState = 0;

Job basicThenFirst()
{
    basicJobState = 10;
    co_return;
}
Job basicThenSecond()
{
    BOOST_REQUIRE_EQUAL(basicJobState, 10);
    basicJobState = 20;
    co_return;
}
Job basicThenThird()
{
    BOOST_REQUIRE_EQUAL(basicJobState, 20);
    co_return;
}

Job basicThenBase()
{
    co_await basicThenFirst();
    co_await basicThenSecond();
    co_await basicThenThird();
}

BOOST_AUTO_TEST_CASE(basic_then)
{
    basicThenBase();
}
/*
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
        .then([=]() -> Job{
            BOOST_REQUIRE_EQUAL(basicCallable, 10);
            co_return;
        });
}*/

BOOST_AUTO_TEST_SUITE_END()