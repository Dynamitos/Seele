set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

find_package(Boost COMPONENTS unit_test_framework REQUIRED)

include_directories(${ENGINE_ROOT})
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(Engine/)
target_link_libraries(Seele_unit_tests ${Boost_LIBRARIES})
target_compile_definitions(Seele_unit_tests PRIVATE -DBOOST_TEST_DYN_LINK)

target_precompile_headers(Seele_unit_tests 
  PRIVATE    
    <assert.h>
    <memory>
    <atomic>
    <coroutine>
    <thread>
    <cstring>
    <iostream>
    <string>
    <thread>
    <functional>
    <filesystem>
    <fstream>
    <mutex>
    <condition_variable>)


enable_testing()
add_test(SeeleEngineTest Seele_unit_tests)
