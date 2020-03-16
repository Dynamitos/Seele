#include "EngineTest.h"
#include "Containers/Array.h"
#include <time.h>
#include <chrono>
#define BOOST_TEST_MODULE SeeleEngine
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

BOOST_AUTO_TEST_CASE(benchmark)
{
	using namespace std::chrono;
	srand(time(NULL));
	const uint32 TEST_SIZE = 1024 * 256;
	uint32* testSet = new uint32[TEST_SIZE];
#pragma loop( hint_parallel( 0 ) )
	for (int i = 0; i < TEST_SIZE; ++i)
	{
		testSet[i] = rand();
	}
	high_resolution_clock::time_point start_vector = high_resolution_clock::now();
	std::vector<uint32> vec;
	for (int i = 0; i < TEST_SIZE; ++i)
	{
		vec.push_back(testSet[i]);
	}
	high_resolution_clock::time_point end_vector = high_resolution_clock::now();
	float time_vector = duration_cast<milliseconds>(end_vector - start_vector).count();

	std::random_shuffle(testSet, testSet + TEST_SIZE);
	high_resolution_clock::time_point start_remove_vector = high_resolution_clock::now();
	for (int i = 0; i < TEST_SIZE; ++i)
	{
		vec.erase(std::find(vec.begin(), vec.end(), testSet[i]));
	}
	high_resolution_clock::time_point end_remove_vector = high_resolution_clock::now();
	float remove_vector = duration_cast<milliseconds>(end_remove_vector - start_remove_vector).count();

	high_resolution_clock::time_point start_array = high_resolution_clock::now();
	Array<uint32> arr;
	for (int i = 0; i < TEST_SIZE; ++i)
	{
		arr.add(testSet[i]);
	}
	high_resolution_clock::time_point end_array = high_resolution_clock::now();
	float time_array = duration_cast<milliseconds>(end_array - start_array).count();

	high_resolution_clock::time_point start_remove_array = high_resolution_clock::now();
	for (int i = 0; i < TEST_SIZE; ++i)
	{
		arr.remove(arr.find(testSet[i]), false);
	}
	high_resolution_clock::time_point end_remove_array = high_resolution_clock::now();
	float remove_array = duration_cast<milliseconds>(end_remove_array - start_remove_array).count();
	std::cout << "Vector: " << time_vector << "ms Array: " << time_array << "ms" << std::endl;
	std::cout << "Vector remove: " << remove_vector << "ms Array remove: " << remove_array << "ms" << std::endl;
	delete[] testSet;
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