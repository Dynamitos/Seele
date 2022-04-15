set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
find_package(Aftermath REQUIRED)

add_subdirectory(Engine/)

target_include_directories(Seele_unit_tests PRIVATE ${ENGINE_ROOT})
target_include_directories(Seele_unit_tests PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(Seele_unit_tests PRIVATE ${GLM_INCLUDE_DIRS})
target_include_directories(Seele_unit_tests PRIVATE ${Vulkan_INCLUDE_DIR})
target_include_directories(Seele_unit_tests PRIVATE ${NSAM_INCLUDE_DIRS})

target_link_libraries(Seele_unit_tests ${Boost_LIBRARIES})
target_link_libraries(Seele_unit_tests ${Vulkan_LIBRARY})
target_link_libraries(Seele_unit_tests ${NSAM_LIBRARIES})
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
