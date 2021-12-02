#include "EngineTest.h"
#include "Containers/Array.h"
#include "MinimalEngine.h"
#include <algorithm>
#include <time.h>
#include <chrono>
#include <boost/test/unit_test.hpp>

using namespace Seele;

BOOST_AUTO_TEST_SUITE(Array_Suite)

BOOST_AUTO_TEST_CASE(empty_constructur)
{
	Array<uint8> array;
	BOOST_CHECK_EQUAL(array.size(), 0);
}

BOOST_AUTO_TEST_CASE(initialial_size)
{
	Array<uint8> array(3);
	BOOST_CHECK_EQUAL(array.size(), 3);
}

BOOST_AUTO_TEST_CASE(resize)
{
	Array<uint8> array;
	array.add(2);
	array.add(3);
	BOOST_CHECK_EQUAL(array.size(), 2);
	array.resize(5);
	BOOST_CHECK_EQUAL(array.size(), 5);
}
BOOST_AUTO_TEST_CASE(clear)
{
	Array<uint8> array;
	array.add(3);
	array.add(2);
	array.add(6);
	BOOST_CHECK_EQUAL(array.size(), 3);
	array.clear();
	BOOST_CHECK_EQUAL(array.size(), 0);
}
BOOST_AUTO_TEST_CASE(remove_keeporder)
{
	Array<uint8> array;
	array.add(1);
	array.add(2);
	array.add(3);
	array.add(4);
	array.add(5);
	array.remove(1);
	BOOST_CHECK_EQUAL(array[1], 3);
	BOOST_CHECK_EQUAL(array[2], 4);
	BOOST_CHECK_EQUAL(array[3], 5);
	BOOST_CHECK_EQUAL(array.size(), 4);
}

BOOST_AUTO_TEST_CASE(remove_swap)
{
	Array<uint8> array;
	array.add(1);
	array.add(2);
	array.add(3);
	array.add(4);
	array.add(5);
	array.remove(1, false);
	BOOST_CHECK_EQUAL(array[1], 5);
	BOOST_CHECK_EQUAL(array[2], 3);
	BOOST_CHECK_EQUAL(array[3], 4);
	BOOST_CHECK_EQUAL(array.size(), 4);
}

BOOST_AUTO_TEST_CASE(remove_iterator)
{
	Array<uint8> array;
	array.add(1);
	array.add(2);
	array.add(3);
	array.add(4);
	array.add(5);
	array.remove(array.find(1), false);
	BOOST_CHECK_EQUAL(array[0], 5);
	BOOST_CHECK_EQUAL(array[1], 2);
	BOOST_CHECK_EQUAL(array[2], 3);
	BOOST_CHECK_EQUAL(array[3], 4);
	BOOST_CHECK_EQUAL(array.size(), 4);
}

BOOST_AUTO_TEST_CASE(remove_iterator_keep_order)
{
	Array<uint8> array;
	array.add(1);
	array.add(2);
	array.add(3);
	array.add(4);
	array.add(5);
	array.remove(array.find(1));
	BOOST_CHECK_EQUAL(array[0], 2);
	BOOST_CHECK_EQUAL(array[1], 3);
	BOOST_CHECK_EQUAL(array[2], 4);
	BOOST_CHECK_EQUAL(array[3], 5);
	BOOST_CHECK_EQUAL(array.size(), 4);
}

BOOST_AUTO_TEST_CASE(random_access)
{
	Array<uint8> array;
	array.add(4);
	array.add(5);
	array.add(6);
	BOOST_CHECK_EQUAL(array[2], 6);
	BOOST_CHECK_EQUAL(array[0], 4);
}

class TestStruct
{
public:
	uint32 data;
	~TestStruct()
	{
		data = 1;
	}
};
DECLARE_REF(TestStruct);

BOOST_AUTO_TEST_CASE(refptr_interaction)
{
	uint32* dataPtr;
	{
		PTestStruct test = new TestStruct();
		test->data = 32123;
		dataPtr = &test->data;
		{
			Array<PTestStruct> arr(1);
			arr[0] = test;
		}
		BOOST_CHECK_EQUAL(test->data, 32123);
	}
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(StaticArray_Suite)

BOOST_AUTO_TEST_CASE(empty_constructur)
{
	StaticArray<uint8, 2> array;
	BOOST_CHECK_EQUAL(array.size(), 2);
}

BOOST_AUTO_TEST_CASE(initialial_size)
{
	StaticArray<uint8, 3> array(3);
	BOOST_CHECK_EQUAL(array.size(), 3);
	BOOST_CHECK_EQUAL(array[1], 3);
}

BOOST_AUTO_TEST_CASE(random_access)
{
	StaticArray<uint8, 3> array;
	array[0] = 4;
	array[1] = 5;
	array[2] = 6;
	BOOST_CHECK_EQUAL(array[0], 4);
	BOOST_CHECK_EQUAL(array[2], 6);
}

BOOST_AUTO_TEST_SUITE_END()