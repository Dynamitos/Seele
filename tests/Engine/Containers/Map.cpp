#include "EngineTest.h"
#include "Containers/Map.h"

using namespace Seele;

TEST(CachedMap, insert_find_basic)
{
    Map<int, int> map;
    map[2] = 3;
    map[1] = 5;
    map[6] = 4;
    map[4] = 7;
    ASSERT_EQ(map[2], 3);
    ASSERT_EQ(map[1], 5);
    ASSERT_EQ(map[6], 4);
    ASSERT_EQ(map[4], 7);
    map[2] = 5;
    map[4] = 4;
    ASSERT_EQ(map[2], 5);
    ASSERT_EQ(map[4], 4);
    ASSERT_EQ(map.find(2)->value, 5);
    ASSERT_EQ(map.find(4)->value, 4);
    //ASSERT_EQ(map.find(5), map.end());
}

TEST(CachedMap, for_each)
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
    ASSERT_EQ(count, 4);
}

TEST(CachedMap, remove_entry)
{
    Map<int, int> map;
    map[1] = 5;
    map[4] = 1;
    map[2] = 6;
    map[3] = 4;
    map.erase(2);
    ASSERT_EQ(map.size(), 3);
    ASSERT_EQ(map[1], 5);
    ASSERT_EQ(map[4], 1);
    ASSERT_EQ(map[3], 4);
}

TEST(CachedMap, key_exists)
{
    Map<int, int> map;
    map[2] = 3;
    ASSERT_EQ(map.exists(2), true);
    ASSERT_EQ(map.exists(4), false);
    map.erase(2);
    ASSERT_EQ(map.exists(2), false);
}

TEST(CachedMap, custom_key)
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
    ASSERT_EQ(map[Key{ 2 }], 3);
    ASSERT_EQ(map[Key{ 3 }], 4);

}

TEST(CachedMap, string_key)
{
    std::map<std::string, int> map;
    map["Test"] = 2;
    map["Test2"] = 3;
    ASSERT_EQ(map["Test"], 2);
    ASSERT_EQ(map["Test2"], 3);
}

