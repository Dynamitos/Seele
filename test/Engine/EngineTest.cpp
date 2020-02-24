#include "EngineTest.h"
#include "MinimalEngine.h"
#include <boost/test/unit_test.hpp>

using namespace Seele;

BOOST_AUTO_TEST_SUITE(RefPtr)

struct TestStruct
{
	TestStruct()
		: data(10)
	{
	}
	~TestStruct()
	{
	}
	uint32 data;
};
BOOST_AUTO_TEST_CASE(basic_refcount)
{
	{
		Seele::RefPtr<TestStruct> ptr = new TestStruct();
		BOOST_REQUIRE_EQUAL(ptr->data, 10);
		{
			Seele::RefPtr<TestStruct> secondPtr = ptr;
			BOOST_REQUIRE_EQUAL(secondPtr->data, 10);
			BOOST_REQUIRE_EQUAL(ptr->data, 10);
		}
		BOOST_REQUIRE_EQUAL(ptr->data, 10);
	}
}

BOOST_AUTO_TEST_CASE(unique_ptr)
{
	UniquePtr<TestStruct> uptr = new TestStruct();
	UniquePtr<TestStruct> uptr2 = std::move(uptr);
	BOOST_REQUIRE_EQUAL(uptr2->data, 10);
	BOOST_REQUIRE_EQUAL(uptr.isValid(), false);
}

BOOST_AUTO_TEST_SUITE_END()