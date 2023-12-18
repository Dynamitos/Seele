find_package(GTest CONFIG REQUIRED)

add_executable(SeeleUnitTests "")

add_subdirectory(Engine/)

target_link_libraries(SeeleUnitTests PRIVATE Engine)
target_link_libraries(SeeleUnitTests PRIVATE GTest::gtest_main)

enable_testing()
add_test(SeeleEngineTest SeeleUnitTests)

include(GoogleTest)
gtest_discover_tests(SeeleUnitTests)