#include "EngineTest.h"
#include "Containers/Array.h"
#include "MinimalEngine.h"
#include <algorithm>
#include <time.h>
#include <chrono>

using namespace Seele;

TEST(ArraySuite, empty_constructur)
{
    Array<uint8> array;
    ASSERT_EQ(array.size(), 0);
}

TEST(ArraySuite, initialial_size)
{
    Array<uint8> array(3);
    ASSERT_EQ(array.size(), 3);
}

TEST(ArraySuite, resize)
{
    Array<uint8> array;
    array.add(2);
    array.add(3);
    ASSERT_EQ(array.size(), 2);
    array.resize(5);
    ASSERT_EQ(array.size(), 5);
}
TEST(ArraySuite, clear)
{
    Array<uint8> array;
    array.add(3);
    array.add(2);
    array.add(6);
    ASSERT_EQ(array.size(), 3);
    array.clear();
    ASSERT_EQ(array.size(), 0);
    array.add(2);
}
TEST(ArraySuite, remove_keeporder)
{
    Array<uint8> array;
    array.add(1);
    array.add(2);
    array.add(3);
    array.add(4);
    array.add(5);
    array.remove(1);
    ASSERT_EQ(array[1], 3);
    ASSERT_EQ(array[2], 4);
    ASSERT_EQ(array[3], 5);
    ASSERT_EQ(array.size(), 4);
}

TEST(ArraySuite, find)
{
    Array<uint8> array;
    for(uint8 i = 0; i < 100; ++i)
    {
        array.add(i);
    }
    auto it = array.find([](uint8 elem){ return elem / 9 == 9; });
    ASSERT_EQ(*it, 81);
}

TEST(ArraySuite, remove_swap)
{
    Array<uint8> array;
    array.add(1);
    array.add(2);
    array.add(3);
    array.add(4);
    array.add(5);
    array.removeAt(1, false);
    ASSERT_EQ(array[1], 5);
    ASSERT_EQ(array[2], 3);
    ASSERT_EQ(array[3], 4);
    ASSERT_EQ(array.size(), 4);
}

TEST(ArraySuite, remove_iterator)
{
    Array<uint8> array;
    array.add(1);
    array.add(2);
    array.add(3);
    array.add(4);
    array.add(5);
    array.erase(array.find(1), false);
    ASSERT_EQ(array[0], 5);
    ASSERT_EQ(array[1], 2);
    ASSERT_EQ(array[2], 3);
    ASSERT_EQ(array[3], 4);
    ASSERT_EQ(array.size(), 4);
}

TEST(ArraySuite, remove_iterator_keep_order)
{
    Array<uint8> array;
    array.add(1);
    array.add(2);
    array.add(3);
    array.add(4);
    array.add(5);
    array.erase(array.find(1));
    ASSERT_EQ(array[0], 2);
    ASSERT_EQ(array[1], 3);
    ASSERT_EQ(array[2], 4);
    ASSERT_EQ(array[3], 5);
    ASSERT_EQ(array.size(), 4);
}

TEST(ArraySuite, random_access)
{
    Array<uint8> array;
    array.add(4);
    array.add(5);
    array.add(6);
    ASSERT_EQ(array[2], 6);
    ASSERT_EQ(array[0], 4);
}

TEST(ArraySuite, copy)
{
    Array<uint8> array;
    array.add(0);
    array.add(1);
    array.add(2);
    array.add(3);
    Array<uint8> copy = array;
    ASSERT_EQ(array, copy);
    Array<uint8> copy2(copy);
    ASSERT_EQ(array, copy2);
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

TEST(ArraySuite, virtual_classes)
{
    Array<DerivedContainer> array(64);
    for(uint32 i = 0; i < 100; ++i)
    {
        Array<DerivedContainer> copy = array;
        ASSERT_EQ(array.size(), copy.size());
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

TEST(StaticArraySuite, empty_constructur)
{
    StaticArray<uint8, 2> array;
    ASSERT_EQ(array.size(), 2);
}

TEST(StaticArraySuite, initialial_size)
{
    StaticArray<uint8, 3> array(3);
    ASSERT_EQ(array.size(), 3);
    ASSERT_EQ(array[1], 3);
}

TEST(StaticArraySuite, random_access)
{
    StaticArray<uint8, 3> array;
    array[0] = 4;
    array[1] = 5;
    array[2] = 6;
    ASSERT_EQ(array[0], 4);
    ASSERT_EQ(array[2], 6);
}