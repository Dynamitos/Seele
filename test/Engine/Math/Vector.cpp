#include "EngineTest.h"
#include "Math/Vector.h"
#include <boost/test/unit_test.hpp>

using namespace Seele;
using namespace Seele::Math;

BOOST_AUTO_TEST_SUITE(VectorParse)

BOOST_AUTO_TEST_CASE(parse_vector_basic)
{
    const char* str = "float3(0, 0, 0)";
    Vector vec = parseVector(str);
    BOOST_CHECK_EQUAL(vec.x, 0);
    BOOST_CHECK_EQUAL(vec.y, 0);
    BOOST_CHECK_EQUAL(vec.z, 0);
}

BOOST_AUTO_TEST_CASE(parse_vector_literal)
{
    const char* str = "float3(0.0f, 0.0f, 0.0f)";
    Vector vec = parseVector(str);
    BOOST_CHECK_EQUAL(vec.x, 0);
    BOOST_CHECK_EQUAL(vec.y, 0);
    BOOST_CHECK_EQUAL(vec.z, 0);
}

BOOST_AUTO_TEST_CASE(parse_vector_no_literal)
{
    const char* str = "float3(0.0, 0.0, 0.0)";
    Vector vec = parseVector(str);
    BOOST_CHECK_EQUAL(vec.x, 0);
    BOOST_CHECK_EQUAL(vec.y, 0);
    BOOST_CHECK_EQUAL(vec.z, 0);
}

BOOST_AUTO_TEST_CASE(parse_vector_float)
{
    const char* str = "float3(0.01, 0.01f, 0.01f)";
    Vector vec = parseVector(str);
    BOOST_CHECK_EQUAL(vec.x, 0.01f);
    BOOST_CHECK_EQUAL(vec.y, 0.01f);
    BOOST_CHECK_EQUAL(vec.z, 0.01f);
}

BOOST_AUTO_TEST_SUITE_END()