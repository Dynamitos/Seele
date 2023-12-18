#include "EngineTest.h"
#include "Math/Vector.h"

TEST(VectorParse, parse_vector_basic)
{
    const char* str = "float3(0, 0, 0)";
    Vector vec = parseVector(str);
    ASSERT_EQ(vec.x, 0);
    ASSERT_EQ(vec.y, 0);
    ASSERT_EQ(vec.z, 0);
}

TEST(VectorParse, parse_vector_literal)
{
    const char* str = "float3(0.0f, 0.0f, 0.0f)";
    Vector vec = parseVector(str);
    ASSERT_EQ(vec.x, 0);
    ASSERT_EQ(vec.y, 0);
    ASSERT_EQ(vec.z, 0);
}

TEST(VectorParse, parse_vector_no_literal)
{
    const char* str = "float3(0.0, 0.0, 0.0)";
    Vector vec = parseVector(str);
    ASSERT_EQ(vec.x, 0);
    ASSERT_EQ(vec.y, 0);
    ASSERT_EQ(vec.z, 0);
}

TEST(VectorParse, parse_vector_float)
{
    const char* str = "float3(0.01, 0.01f, 0.01f)";
    Vector vec = parseVector(str);
    ASSERT_EQ(vec.x, 0.01f);
    ASSERT_EQ(vec.y, 0.01f);
    ASSERT_EQ(vec.z, 0.01f);
}
