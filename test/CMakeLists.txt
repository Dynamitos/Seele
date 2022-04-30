set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
find_package(Vulkan REQUIRED)

add_subdirectory(Engine/)

target_include_directories(Seele_unit_tests PRIVATE ${ENGINE_ROOT})

target_link_libraries(Seele_unit_tests PRIVATE Boost::unit_test_framework)
target_link_libraries(Seele_unit_tests PRIVATE Vulkan::Vulkan)
target_link_libraries(Seele_unit_tests PRIVATE nsam)
if(UNIX)
  target_compile_definitions(Seele_unit_tests PRIVATE -DBOOST_TEST_DYN_LINK)
endif()

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
