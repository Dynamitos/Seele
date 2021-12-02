#include "EngineTest.h"
#include "MinimalEngine.h"
#define BOOST_TEST_MODULE SeeleEngine
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

struct DeclStruct;
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
struct DeclStruct
{
	~DeclStruct()
	{
		data = 10;
	}
	uint32 data = 20;
};

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
	Seele::UniquePtr<TestStruct> uptr = new TestStruct();
	Seele::UniquePtr<TestStruct> uptr2 = std::move(uptr);
	BOOST_REQUIRE_EQUAL(uptr2->data, 10);
	BOOST_REQUIRE_EQUAL(uptr.isValid(), false);
}
struct ThisReference
{
	ThisReference()
	{
	}
	~ThisReference()
	{
		data = 12;
	}
	Seele::RefPtr<ThisReference> getRef()
	{
		return this;
	}
	uint32 data = 1;
};

BOOST_AUTO_TEST_CASE(this_reference)
{
	Seele::RefPtr<ThisReference> ptr2;
	{
		Seele::RefPtr<ThisReference> ptr = new ThisReference();
		BOOST_REQUIRE_EQUAL(ptr->data, 1);
		ptr2 = ptr->getRef();
		BOOST_REQUIRE_EQUAL(ptr2->data, 1);
		ptr2->data = 5;
		BOOST_REQUIRE_EQUAL(ptr->data, 5);
	}
	BOOST_REQUIRE_EQUAL(ptr2->data, 5);
}

BOOST_AUTO_TEST_SUITE_END()