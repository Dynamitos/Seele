cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 20)

# Handle superbuild first
option (USE_SUPERBUILD "Whether or not a superbuild should be invoked" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src/Engine)
set(EXTERNAL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(ASSIMP_ROOT ${EXTERNAL_ROOT}/assimp)
set(BOOST_ROOT ${EXTERNAL_ROOT}/boost)
set(SLANG_ROOT ${EXTERNAL_ROOT}/slang)
set(GLM_ROOT ${EXTERNAL_ROOT}/glm)
set(GLFW_ROOT ${EXTERNAL_ROOT}/glfw)
set(JSON_ROOT ${EXTERNAL_ROOT}/json)
set(KTX_ROOT ${EXTERNAL_ROOT}/ktx)
set(STB_ROOT ${EXTERNAL_ROOT}/stb)
set(TTFPARSER_ROOT ${EXTERNAL_ROOT}/ttf-parser)
set(FREETYPE_ROOT ${EXTERNAL_ROOT}/freetype)
set(SPIRV_ROOT ${EXTERNAL_ROOT}/SPIRV-Cross)
set(NSAM_ROOT ${EXTERNAL_ROOT}/aftermath)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
set(Boost_NO_WARN_NEW_VERSIONS 1)

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

set(Boost_USE_STATIC_LIBS OFF)
if(WIN32)
  set(Boost_LIB_PREFIX lib)
endif()

find_package(Vulkan REQUIRED)
find_package(Boost REQUIRED COMPONENTS serialization)
find_package(Threads REQUIRED)
find_package(Assimp REQUIRED)
find_package(JSON REQUIRED)
find_package(GLFW REQUIRED)
find_package(FreeType REQUIRED)
find_package(TTFParser REQUIRED)
find_package(GLM REQUIRED)
find_package(KTX REQUIRED)
find_package(STB REQUIRED)
find_package(SLANG REQUIRED)
find_package(Aftermath REQUIRED)

add_definitions(${Vulkan_DEFINITIONS})
add_definitions(${Boost_DEFINITIONS})
add_compile_definitions(GLFW_WINDOWS)
if(WIN32)
  add_compile_definitions(USE_EXTENSIONS)
endif()
if(CMAKE_DEBUG_POSTFIX)
  add_compile_definitions(ENABLE_VALIDATION)
  add_compile_definitions(SEELE_DEBUG)
endif()
add_executable(Engine "")
target_include_directories(Engine PRIVATE ${GLM_INCLUDE_DIRS})
target_include_directories(Engine PRIVATE ${KTX_INCLUDE_DIRS})
target_include_directories(Engine PRIVATE ${STB_INCLUDE_DIRS})
target_include_directories(Engine PRIVATE ${Vulkan_INCLUDE_DIR})
target_include_directories(Engine PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(Engine PRIVATE ${GLFW_INCLUDE_DIRS})
target_include_directories(Engine PRIVATE ${FREETYPE_INCLUDE_DIRS})
target_include_directories(Engine PRIVATE ${SLANG_INCLUDE_DIRS})
target_include_directories(Engine PRIVATE ${ASSIMP_INCLUDE_DIRS})
target_include_directories(Engine PRIVATE ${JSON_INCLUDE_DIRS})
target_include_directories(Engine PRIVATE ${NSAM_INCLUDE_DIRS})
target_include_directories(Engine PRIVATE ${TTFPARSER_INCLUDE_DIRS})
target_include_directories(Engine PRIVATE ${ENGINE_ROOT})
target_include_directories(Engine PRIVATE src/Engine)

target_link_libraries(Engine ${Vulkan_LIBRARY})
target_link_libraries(Engine ${Boost_LIBRARIES})
target_link_libraries(Engine ${GLFW_LIBRARIES})
target_link_libraries(Engine ${FREETYPE_LIBRARIES})
target_link_libraries(Engine ${SLANG_LIBRARIES})
target_link_libraries(Engine ${ASSIMP_LIBRARIES})
target_link_libraries(Engine ${NSAM_LIBRARIES})
target_link_libraries(Engine ${KTX_LIBRARIES})
if(UNIX)
  target_link_libraries(Engine Threads::Threads)
  target_link_libraries(Engine dl)
endif()
target_precompile_headers(Engine 
  PRIVATE    
    <assert.h>
    <atomic>
    <condition_variable>
    <coroutine>
    <cstring>
    <filesystem>
    <functional>
    <fstream>
    <iostream>
    <memory>
    <mutex>
    <string>
    <thread>
    <boost/serialization/serialization.hpp>
    <boost/crc.hpp>)

add_subdirectory(src/)

if(MSVC)
  set(_CRT_SECURE_NO_WARNINGS)
  target_compile_options(Engine PRIVATE -Zi -MP -DEBUG)
else()
  target_compile_options(Engine PRIVATE -g -Wall -Wextra -Wno-delete-incomplete -pedantic -fcoroutines)
endif()

add_executable(Seele_unit_tests "")
add_subdirectory(test/)

if(WIN32)
  add_custom_target(copy-binaries ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SLANG_BINARIES} $<TARGET_FILE_DIR:Engine>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SLANG_GLSLANG} $<TARGET_FILE_DIR:Engine>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GLFW_BINARY} $<TARGET_FILE_DIR:Engine>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FREETYPE_BINARY} $<TARGET_FILE_DIR:Engine>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ASSIMP_BINARY} $<TARGET_FILE_DIR:Engine>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${NSAM_BINARIES} $<TARGET_FILE_DIR:Engine>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${NSAM_LLVM} $<TARGET_FILE_DIR:Engine>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${KTX_BINARIES} $<TARGET_FILE_DIR:Engine>)
else()
  add_custom_target(copy-binaries ALL COMMAND "")
endif()

add_custom_target(SeeleEngine ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:Engine>
	DEPENDS Engine copy-binaries)

  