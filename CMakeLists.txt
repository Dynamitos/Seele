cmake_minimum_required(VERSION 3.7...3.23)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()
set(CMAKE_CXX_STANDARD 20)

# Handle superbuild first
option (USE_SUPERBUILD "Whether or not a superbuild should be invoked" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR OFF)
set(CMAKE_INSTALL_MESSAGE LAZY)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src/Engine)
set(EXTERNAL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(ASSIMP_ROOT ${EXTERNAL_ROOT}/assimp)
set(BOOST_ROOT ${EXTERNAL_ROOT}/boost)
set(SLANG_ROOT ${EXTERNAL_ROOT}/slang)
set(GLM_ROOT ${EXTERNAL_ROOT}/glm)
set(GLFW_ROOT ${EXTERNAL_ROOT}/glfw)
set(CRCPP_ROOT ${EXTERNAL_ROOT}/CRCpp)
set(JSON_ROOT ${EXTERNAL_ROOT}/json)
set(KTX_ROOT ${EXTERNAL_ROOT}/ktx)
set(STB_ROOT ${EXTERNAL_ROOT}/stb)
set(TTFPARSER_ROOT ${EXTERNAL_ROOT}/ttf-parser)
set(FREETYPE_ROOT ${EXTERNAL_ROOT}/freetype)
set(SPIRV_ROOT ${EXTERNAL_ROOT}/SPIRV-Cross)
set(ENTT_ROOT ${EXTERNAL_ROOT}/entt)
set(ODEINT_ROOT ${EXTERNAL_ROOT}/odeint)
set(THREADPOOL_ROOT ${EXTERNAL_ROOT}/thread-pool)
set(ZLIB_ROOT ${EXTERNAL_ROOT}/zlib)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
set(Boost_NO_WARN_NEW_VERSIONS 1)

project (Seele)

include(cmake/SuperBuild.cmake)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
#Workaround for vs, because it places artifacts into an additional subfolder
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_SOURCE_DIR}/bin)

find_package(Vulkan REQUIRED)

if(UNIX)
    find_package(Threads REQUIRED)
endif()

if(WIN32)
    add_compile_definitions(USE_EXTENSIONS)
endif()
if(CMAKE_DEBUG_POSTFIX)
    add_compile_definitions(ENABLE_VALIDATION)
    add_compile_definitions(SEELE_DEBUG)
endif()

add_library(Engine SHARED "")

target_compile_definitions(Engine PRIVATE GLFW_WINDOWS)
target_include_directories(Engine PRIVATE src/Engine)
target_link_libraries(Engine PUBLIC Vulkan::Vulkan)
target_link_libraries(Engine PUBLIC EnTT::EnTT)
target_link_libraries(Engine PUBLIC glfw)
target_link_libraries(Engine PUBLIC glm)
target_link_libraries(Engine PUBLIC freetype)
target_link_libraries(Engine PUBLIC slang)
target_link_libraries(Engine PUBLIC assimp)
target_link_libraries(Engine PUBLIC ktx)
target_link_libraries(Engine PUBLIC stb)
target_link_libraries(Engine PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(Engine PUBLIC dp::thread-pool)
target_link_libraries(Engine PUBLIC crcpp)
target_link_libraries(Engine PUBLIC odeint)
target_link_libraries(Engine PUBLIC zlib)
if(UNIX)
    target_link_libraries(Engine PUBLIC Threads::Threads)
    target_link_libraries(Engine PUBLIC dl)
endif()

add_executable(Editor "")
target_link_libraries(Editor PRIVATE Engine)
target_include_directories(Editor PRIVATE src/Editor)
target_compile_definitions(Editor PRIVATE EDITOR=1)

add_executable(AssetViewer "")
target_link_libraries(AssetViewer PRIVATE Engine)
target_include_directories(AssetViewer PRIVATE src/AssetViewer)

target_precompile_headers(Engine 
  PUBLIC    
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
    <thread>)
    
if(MSVC)
    set(_CRT_SECURE_NO_WARNINGS)
    target_compile_options(Engine PUBLIC /std:c++20 /Zi /MP14 /W4 /DEBUG "/WX-")
    target_compile_options(Editor PUBLIC /std:c++20 /Zi /MP14 /W4 /DEBUG "/WX-")
    target_sources(Engine INTERFACE   
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Seele.natvis>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/Seele.natvis>
    )
    install(FILES
        Seele.natvis
        DESTINATION .)
else()
    target_compile_options(Engine PUBLIC -g -Wall -Wextra -Wno-error -pedantic -std=c++20 -Wno-missing-field-initializers -Wno-unused-function)
    target_compile_options(Editor PUBLIC -g -Wall -Wextra -Wno-error -pedantic -std=c++20)
endif()

add_subdirectory(src/)

if(WIN32)
    add_custom_target(dll_copy ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:Engine> $<TARGET_FILE_DIR:Editor>
        COMMAND_EXPAND_LISTS
        DEPENDS Editor slang-build)
else()
    add_custom_target(dll_copy ALL
        COMMAND ${CMAKE_COMMAND} -E true)
endif()

add_custom_target(SeeleEngine ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:Engine>
    COMMAND_EXPAND_LISTS
	DEPENDS Editor slang-build dll_copy)

install(
    TARGETS
        AssetViewer
        Editor
        Engine
        EnTT
        glfw
        glm
        freetype
        assimp
        ktx
        stb
        crcpp
        nlohmann_json
        ThreadPool
        zlib
        IrrXML
        odeint
    EXPORT 
        EngineTargets
    FILE_SET HEADERS
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

export(EXPORT EngineTargets
    FILE "${CMAKE_BINARY_DIR}/EngineTargets.cmake"
    NAMESPACE Seele::)

include(CMakePackageConfigHelpers)

configure_package_config_file(cmake/EngineConfig.cmake.in
    "SeeleConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_BINARY_DIR})

install(EXPORT EngineTargets
    FILE
        EngineTargets.cmake
    NAMESPACE
        Seele::
    DESTINATION
        ${CMAKE_INSTALL_PREFIX}
)

install(
    FILES
        ${CMAKE_BINARY_DIR}/SeeleConfig.cmake
    DESTINATION
        ${CMAKE_INSTALL_PREFIX}
)

install(
    DIRECTORY
        ${CMAKE_SOURCE_DIR}/res
    DESTINATION
        ${CMAKE_INSTALL_PREFIX}
)
