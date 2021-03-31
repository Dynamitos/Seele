#include "EngineTest.h"
#include "MinimalEngine.h"
#include <boost/test/unit_test.hpp>

using namespace Seele;
BOOST_TEST_GLOBAL_FIXTURE(GlobalFixture);
BOOST_AUTO_TEST_SUITE(RefPtr)

struct TestStruct
{
	TestStruct()
		: data(10)
	{
	}
	virtual ~TestStruct()
	{
		data = 15;
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

struct DerivedStruct : public TestStruct
{
	DerivedStruct()
		:data2(20)
	{

	}
	~DerivedStruct()
	{
		data2 = 30;
	}
	uint32 data2;
};

BOOST_AUTO_TEST_CASE(inheritance_cast)
{
	Seele::RefPtr<DerivedStruct> backCast;
	{
		Seele::RefPtr<DerivedStruct> derived = new DerivedStruct();
		Seele::RefPtr<TestStruct> base = derived;
		BOOST_REQUIRE_EQUAL(base->data, 10);
		backCast = base.cast<DerivedStruct>();
		BOOST_REQUIRE_EQUAL(backCast->data, 10);
		BOOST_REQUIRE_EQUAL(backCast->data2, 20);
	}
	BOOST_REQUIRE_EQUAL(backCast->data, 10);
	BOOST_REQUIRE_EQUAL(backCast->data2, 20);
}

BOOST_AUTO_TEST_CASE(unique_ptr)
{
	UniquePtr<TestStruct> uptr = new TestStruct();
	UniquePtr<TestStruct> uptr2 = std::move(uptr);
	BOOST_REQUIRE_EQUAL(uptr2->data, 10);
	BOOST_REQUIRE_EQUAL(uptr.isValid(), false);
}

BOOST_AUTO_TEST_SUITE_END()