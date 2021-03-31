#include "EngineTest.h"
#include "Containers/Map.h"
#include <boost/test/unit_test.hpp>

using namespace Seele;

BOOST_AUTO_TEST_SUITE(CachedMap)

BOOST_AUTO_TEST_CASE(insert_find_basic)
{
	Map<int, int> map;
	map[2] = 3;
	map[1] = 5;
	map[6] = 4;
	map[4] = 7;
	BOOST_REQUIRE_EQUAL(map[2], 3);
	BOOST_REQUIRE_EQUAL(map[1], 5);
	BOOST_REQUIRE_EQUAL(map[6], 4);
	BOOST_REQUIRE_EQUAL(map[4], 7);
	map[2] = 5;
	map[4] = 4;
	BOOST_REQUIRE_EQUAL(map[2], 5);
	BOOST_REQUIRE_EQUAL(map[4], 4);
}

BOOST_AUTO_TEST_CASE(for_each)
{
	Map<int, int> map;
	map[2] = 3;
	map[1] = 5;
	map[6] = 4;
	map[4] = 7;
	int count = 0;
	for(auto it : map)
	{
		count++;
	}
	BOOST_REQUIRE_EQUAL(count, 4);
}

BOOST_AUTO_TEST_CASE(key_exists)
{
	Map<int, int> map;
	map[2] = 3;
	BOOST_REQUIRE_EQUAL(map.exists(2), true);
	BOOST_REQUIRE_EQUAL(map.exists(4), false);
	map.erase(2);
	BOOST_REQUIRE_EQUAL(map.exists(2), false);
}

BOOST_AUTO_TEST_CASE(custom_key)
{
	struct Key
	{
		int id;
		bool operator<(const Key& other) const
		{
			return id < other.id;
		}
	};
	Map<Key, int> map;
	map[Key{ 2 }] = 3;
	map[Key{ 3 }] = 4;
	BOOST_REQUIRE_EQUAL(map[Key{ 2 }], 3);
	BOOST_REQUIRE_EQUAL(map[Key{ 3 }], 4);

}

BOOST_AUTO_TEST_SUITE_END()
