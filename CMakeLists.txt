cmake_minimum_required(VERSION 3.15)

# Handle superbuild first
option (USE_SUPERBUILD "Whether or not a superbuild should be invoked" ON)

set (GLOBAL_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(Boost_USE_STATIC_LIBS ON)
set(Boost_LIB_PREFIX lib)
set(Boost_USE_RELEASE_LIBS OFF)

if (USE_SUPERBUILD)
  project (SUPERBUILD NONE)
  # execute the superbuild (this script will be invoked again without the
  # USE_SUPERBUILD option this time)
  include (cmake/SuperBuild.cmake)
  return()                      # stop processing this file further
else()
  project (Seele)
endif()

find_package(Vulkan REQUIRED)
find_package(Boost REQUIRED COMPONENTS serialization)

include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${GLM_INCLUDE_DIRS})
include_directories(${Vulkan_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(${GLM_DEFINITIONS})
add_definitions(${VULKAN_DEFINITIONS})
add_definitions(${BOOST_DEFINITIONS})
add_executable(SeeleEngine src/Engine/main.cpp src/Engine/Graphics/Graphics.cpp)
target_link_libraries(SeeleEngine ${Boost_LIBRARIES})
target_link_libraries(SeeleEngine ${Vulkan_LIBRARY})
target_link_libraries(SeeleEngine ${GLFW_LIBRARY})

message(STATUS "TEST")