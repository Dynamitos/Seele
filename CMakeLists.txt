cmake_minimum_required(VERSION 3.7...3.23)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()
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
set(ENTT_ROOT ${EXTERNAL_ROOT}/entt)
set(NSAM_ROOT ${EXTERNAL_ROOT}/aftermath)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
set(Boost_NO_WARN_NEW_VERSIONS 1)

project (Seele)

include(cmake/Superbuild.cmake)

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

if(WIN32)
  add_compile_definitions(USE_EXTENSIONS)
endif()
if(CMAKE_DEBUG_POSTFIX)
  add_compile_definitions(ENABLE_VALIDATION)
  add_compile_definitions(SEELE_DEBUG)
endif()
add_executable(Engine "")
target_compile_definitions(Engine PUBLIC GLFW_WINDOWS)
target_include_directories(Engine PUBLIC src/Engine)
target_link_libraries(Engine PUBLIC Vulkan::Vulkan)
target_link_libraries(Engine PUBLIC Boost::headers)
target_link_libraries(Engine PUBLIC Boost::serialization)
target_link_libraries(Engine PUBLIC EnTT::EnTT)
target_link_libraries(Engine PUBLIC glfw)
target_link_libraries(Engine PUBLIC glm::glm)
target_link_libraries(Engine PUBLIC freetype)
target_link_libraries(Engine PUBLIC slang)
target_link_libraries(Engine PUBLIC assimp)
target_link_libraries(Engine PUBLIC nsam)
target_link_libraries(Engine PUBLIC ktx)
target_link_libraries(Engine PUBLIC stb)
target_link_libraries(Engine PUBLIC nlohmann_json::nlohmann_json)
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

add_custom_target(SeeleEngine ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:Engine>
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:Engine> $<TARGET_FILE_DIR:Engine>
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:slang,SLANG_BINARY> $<TARGET_FILE_DIR:Engine>
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:slang,GLSLANG_BINARY> $<TARGET_FILE_DIR:Engine>
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:nsam,NSAM_BINARY> $<TARGET_FILE_DIR:Engine>
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:nsam,LLVM_BINARY> $<TARGET_FILE_DIR:Engine>
  COMMAND_EXPAND_LISTS
	DEPENDS Engine slang-build)

