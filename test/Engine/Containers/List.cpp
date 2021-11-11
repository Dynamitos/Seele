#include "EngineTest.h"
#include "Containers/List.h"
#include <boost/test/unit_test.hpp>

using namespace Seele;

BOOST_AUTO_TEST_SUITE(List_Suite)

BOOST_AUTO_TEST_CASE(basic_add)
{
	List<int> list;
	list.add(2);
	BOOST_REQUIRE_EQUAL(list.size(), 1);
	list.add(4);
	BOOST_REQUIRE_EQUAL(list.size(), 2);
	List<int>::Iterator it = list.find(2);
	BOOST_REQUIRE_EQUAL(*it, 2);
}

BOOST_AUTO_TEST_CASE(basic_insert)
{
	List<int> list;
	list.add(2);
	list.add(3);
	list.add(4);
	List<int>::Iterator it = list.find(3);
	it = list.insert(it, 1);
	BOOST_REQUIRE_EQUAL(*it, 1);
	BOOST_REQUIRE_EQUAL(list.size(), 4);
}
BOOST_AUTO_TEST_CASE(basic_remove)
{
	List<int> list;
	list.add(2);
	list.add(3);
	list.add(4);
	List<int>::Iterator it = list.find(3);
	it = list.remove(it);
	BOOST_REQUIRE_EQUAL(*it, 4);
	BOOST_REQUIRE_EQUAL(list.size(), 2);
}

BOOST_AUTO_TEST_CASE(list_join)
{
	List<int> list1;
	List<int> list2;
	list1.add(2);
	list1.add(1);
	list1.add(3);
	list1.add(4);
	list2.add(5);
	list2.add(7);
	list2.add(8);
	list2.add(6);
	list2.add(9);
	list1.moveElements(list2);
	BOOST_REQUIRE_EQUAL(list1.size(), 9);
	BOOST_REQUIRE_EQUAL(list2.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()