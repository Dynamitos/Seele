set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

find_package(Boost COMPONENTS unit_test_framework REQUIRED)

include_directories(${ENGINE_ROOT})
include_directories(${Boost_INCLUDE_DIRS})

add_executable(Seele_unit_tests "")
add_subdirectory(Engine/)
target_link_libraries(Seele_unit_tests ${Boost_LIBRARIES})

enable_testing()
add_test(SeeleEngineTest Seele_unit_tests)
