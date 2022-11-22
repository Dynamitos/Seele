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
    array.add(2);
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

BOOST_AUTO_TEST_CASE(find)
{
    Array<uint8> array;
    for(uint8 i = 0; i < 100; ++i)
    {
        array.add(i);
    }
    auto it = array.find([](uint8 elem){ return elem / 9 == 9; });
    BOOST_CHECK_EQUAL(*it, 81);
}

BOOST_AUTO_TEST_CASE(remove_swap)
{
    Array<uint8> array;
    array.add(1);
    array.add(2);
    array.add(3);
    array.add(4);
    array.add(5);
    array.removeAt(1, false);
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

BOOST_AUTO_TEST_CASE(copy)
{
    Array<uint8> array;
    array.add(0);
    array.add(1);
    array.add(2);
    array.add(3);
    Array<uint8> copy = array;
    BOOST_CHECK_EQUAL_COLLECTIONS(array.begin(), array.end(), copy.begin(), copy.end());
    Array<uint8> copy2(copy);
    BOOST_CHECK_EQUAL_COLLECTIONS(array.begin(), array.end(), copy2.begin(), copy2.end());
}

class BaseElement
{
    uint64 test;
};

class BaseContainer
{
    Array<BaseElement> elements;
};

class DerivedContainer : public BaseContainer
{
    uint64 test;
};

BOOST_AUTO_TEST_CASE(virtual_classes)
{
    Array<DerivedContainer> array(64);
    for(uint32 i = 0; i < 100; ++i)
    {
        Array<DerivedContainer> copy = array;
        BOOST_CHECK_EQUAL(array.size(), copy.size());
    }
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