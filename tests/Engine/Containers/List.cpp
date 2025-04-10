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