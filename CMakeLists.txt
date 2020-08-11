cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

# Handle superbuild first
option (USE_SUPERBUILD "Whether or not a superbuild should be invoked" ON)

set(ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src/Engine)
set(EXTERNAL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(ASSIMP_ROOT ${EXTERNAL_ROOT}/assimp)
set(BOOST_ROOT ${EXTERNAL_ROOT}/boost)
set(SLANG_ROOT ${EXTERNAL_ROOT}/slang)
set(GLM_ROOT ${EXTERNAL_ROOT}/glm)
set(GLFW_ROOT ${EXTERNAL_ROOT}/glfw)
set(JSON_ROOT ${EXTERNAL_ROOT}/json)
set(STB_ROOT ${EXTERNAL_ROOT}/stb)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_LIB_PREFIX lib)

if (USE_SUPERBUILD)
  project (SUPERBUILD NONE)
  # execute the superbuild (this script will be invoked again without the
  # USE_SUPERBUILD option this time)
  include (cmake/SuperBuild.cmake)
  return()                      # stop processing this file further
else()
  project (Seele)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
#Workaround for vs, because it places artifacts into an additional subfolder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin/Release)

find_package(Vulkan REQUIRED)
find_package(Boost REQUIRED COMPONENTS serialization)
find_package(Threads REQUIRED)
find_package(assimp REQUIRED ${ASSIMP_ROOT})
find_package(JSON REQUIRED)
find_package(GLFW REQUIRED)

include_directories(${GLM_INCLUDE_DIRS})
include_directories(${STB_INCLUDE_DIRS})
include_directories(${Vulkan_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${SLANG_INCLUDE_DIRS})
include_directories(${ASSIMP_INCLUDE_DIRS})
include_directories(${JSON_INCLUDE_DIRS})
include_directories(${ENGINE_ROOT})
include_directories(src/Engine)
add_definitions(${GLM_DEFINITIONS})
add_definitions(${Vulkan_DEFINITIONS})
add_definitions(${Boost_DEFINITIONS})
add_compile_definitions(GLFW_WINDOWS)
if(WIN32)
	add_compile_definitions(USE_EXTENSIONS)
endif()
add_executable(SeeleEngine "")
target_link_libraries(SeeleEngine ${Vulkan_LIBRARY})
target_link_libraries(SeeleEngine ${Boost_LIBRARIES})
target_link_libraries(SeeleEngine ${GLFW_LIBRARIES})
target_link_libraries(SeeleEngine ${SLANG_LIBRARY})
target_link_libraries(SeeleEngine ${ASSIMP_LIBRARIES})
target_link_libraries(SeeleEngine ${JSON_LIBRARY})

#target_precompile_headers(SeeleEngine 
#  PRIVATE    
#    <assert.h>
#    <memory>
#    <atomic>
#    <cstring>
#    <iostream>
#    <string>
#    <thread>
#    <functional>
#    <filesystem>
#    <fstream>
#    <mutex>
#    <condition_variable>
#    <boost/serialization/serialization.hpp>
#    <boost/crc.hpp>)

add_subdirectory(src/)

if(MSVC)
  set(_CRT_SECURE_NO_WARNINGS)
#  target_compile_options(SeeleEngine PRIVATE /DEBUG:FASTLINK /Zi)
else()
  target_compile_options(SeeleEngine PRIVATE -Wall -Wextra -pedantic)
endif()

add_executable(Seele_unit_tests "")
add_subdirectory(test/)

add_custom_target(copy-runtime-files ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:SeeleEngine>
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SLANG_ROOT}/${SLANG_BINARY} $<TARGET_FILE_DIR:SeeleEngine>
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SLANG_ROOT}/${SLANG_GLSLANG} $<TARGET_FILE_DIR:SeeleEngine>
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GLFW_BINARY} $<TARGET_FILE_DIR:SeeleEngine>
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ASSIMP_BINARY} $<TARGET_FILE_DIR:SeeleEngine>
	DEPENDS SeeleEngine)