#include "EngineTest.h"
#include "Containers/List.h"

using namespace Seele;

TEST(ListSuite, basic_add)
{
	List<int> list;
	list.add(2);
	ASSERT_EQ(list.size(), 1);
	list.add(4);
	ASSERT_EQ(list.size(), 2);
	List<int>::Iterator it = list.find(2);
	ASSERT_EQ(*it, 2);
}

TEST(ListSuite, basic_insert)
{
	List<int> list;
	list.add(2);
	list.add(3);
	list.add(4);
	List<int>::Iterator it = list.find(3);
	it = list.insert(it, 1);
	ASSERT_EQ(*it, 1);
	ASSERT_EQ(list.size(), 4);
}
TEST(ListSuite, basic_remove)
{
	List<int> list;
	list.add(2);
	list.add(3);
	list.add(4);
	List<int>::Iterator it = list.find(3);
	it = list.remove(it);
	ASSERT_EQ(*it, 4);
	ASSERT_EQ(list.size(), 2);
}

TEST(ListSuite, list_join)
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
	list1.moveElements(std::move(list2));
	ASSERT_EQ(list1.size(), 9);
	ASSERT_EQ(list2.size(), 0);
}
