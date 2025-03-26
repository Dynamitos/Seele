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
    for(const auto& [key, val] : map)
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
    Map<std::string, int> map;
    map["Test"] = 2;
    map["Test2"] = 3;
    ASSERT_EQ(map["Test"], 2);
    ASSERT_EQ(map["Test2"], 3);
}

TEST(CachedMap, copy)
{
    Map<int, int> map;
    map[54] = 14;
    map[5123] = 51;
    map[262] = 14;
    map[9620] = 91283;
    map[141] = 415;
    Map<int, int> map2 = map;
    Map<int, int> map3;
    map3 = map;
    Map<int, int>::iterator i1 = map.begin();
    Map<int, int>::iterator i2 = map2.begin();
    Map<int, int>::iterator i3 = map3.begin();
    while(i1 != map.end())
    {
        ASSERT_EQ(i1->key, i2->key);
        ASSERT_EQ(i1->value, i2->value);
        ASSERT_EQ(i1->key, i3->key);
        ASSERT_EQ(i1->value, i3->value);
        i1++; i2++; i3++;
    }
}

TEST(CachedMap, move)
{
    Map<int, int> map;
    map[54] = 14;
    map[5123] = 51;
    map[262] = 14;
    map[9620] = 91283;
    map[141] = 415;
    Map<int, int> map2 = std::move(map);
    ASSERT_EQ(map2[54], 14);
    ASSERT_EQ(map2[5123], 51);
    ASSERT_EQ(map2[262], 14);
    ASSERT_EQ(map2[9620], 91283);
    ASSERT_EQ(map2[141], 415);
    Map<int, int> map3;
    map3 = std::move(map2);
    ASSERT_EQ(map3[54], 14);
    ASSERT_EQ(map3[5123], 51);
    ASSERT_EQ(map3[262], 14);
    ASSERT_EQ(map3[9620], 91283);
    ASSERT_EQ(map3[141], 415);
}

