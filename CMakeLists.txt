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

set(VCPKG_INSTALL_OPTIONS "--allow-unsupported")

set(ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src/Engine)
set(EXTERNAL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/external)

set(CMAKE_TOOLCHAIN_FILE ${EXTERNAL_ROOT}/vcpkg/scripts/buildsystems/vcpkg.cmake)

set(CRCPP_ROOT ${EXTERNAL_ROOT}/CRCpp)
set(TTFPARSER_ROOT ${EXTERNAL_ROOT}/ttf-parser)

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
find_package(assimp CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(FreeType CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Ktx CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)


if(UNIX)
    find_package(Threads REQUIRED)
endif()

if(WIN32)
    add_compile_definitions(USE_EXTENSIONS)
endif()

add_library(Engine SHARED "")

target_compile_definitions(Engine PRIVATE GLFW_WINDOWS)
target_include_directories(Engine PRIVATE src/Engine)
target_link_libraries(Engine PUBLIC Vulkan::Vulkan)
target_link_libraries(Engine PUBLIC EnTT::EnTT)
target_link_libraries(Engine PUBLIC glfw)
target_link_libraries(Engine PUBLIC glm::glm)
target_link_libraries(Engine PUBLIC freetype)
target_link_libraries(Engine PUBLIC assimp::assimp)
target_link_libraries(Engine PUBLIC KTX::ktx)
target_link_libraries(Engine PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(Engine PUBLIC crcpp)
target_link_libraries(Engine PUBLIC fmt::fmt)
target_link_libraries(Engine PUBLIC GPUOpen::VulkanMemoryAllocator)
target_link_libraries(Engine PUBLIC shader-slang)
if(APPLE)
    target_link_directories(Engine PUBLIC /usr/local/lib)
endif()
if(UNIX)
    target_link_libraries(Engine PUBLIC Threads::Threads)
    target_link_libraries(Engine PUBLIC dl)
endif()

add_executable(Editor "")
target_link_libraries(Editor PRIVATE Engine)
target_include_directories(Editor PRIVATE src/Editor)
target_include_directories(Editor PRIVATE ${Stb_INCLUDE_DIR})
target_compile_definitions(Editor PRIVATE EDITOR=1)

add_executable(AssetViewer "")
target_link_libraries(AssetViewer PRIVATE Engine)
target_include_directories(AssetViewer PRIVATE src/AssetViewer)
    
if(MSVC)
    set(_CRT_SECURE_NO_WARNINGS)
    target_compile_options(Engine PUBLIC /std:c++20 /Zi /MP14 /W4 /DEBUG /wd4505)
    target_compile_options(Editor PUBLIC /std:c++20 /Zi /MP14 /W4 /DEBUG)
    target_sources(Engine INTERFACE   
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Seele.natvis>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/Seele.natvis>
    )
    install(FILES
        Seele.natvis
        DESTINATION .)
else()
    target_compile_options(Engine PUBLIC -g -Wall -Wextra -Wno-error -pedantic -std=c++20 -Wno-missing-field-initializers -Wno-unused-function -fsanitize=address)
    target_compile_options(Editor PUBLIC -g -Wall -Wextra -Wno-error -pedantic -std=c++20 -fsanitize=address)
    target_link_options(Engine PUBLIC -fsanitize=address)
    target_link_options(Editor PUBLIC -fsanitize=address)
endif()
target_compile_options(Engine PUBLIC "$<$<CONFIG:DEBUG>:-DENABLE_VALIDATION>")
target_compile_options(Engine PUBLIC "$<$<CONFIG:DEBUG>:-DSEELE_DEBUG>")

add_subdirectory(src/)
add_subdirectory(tests/)

if(WIN32)
    add_custom_target(dll_copy ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:Engine> $<TARGET_FILE_DIR:Editor>
        COMMAND_EXPAND_LISTS
        DEPENDS Editor)
elseif(APPLE)
    add_custom_target(dll_copy ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SLANG_ROOT}/libslang.dylib $<TARGET_FILE_DIR:Editor>
        COMMAND_EXPAND_LISTS
        DEPENDS Editor)
else()
    add_custom_target(dll_copy ALL
        COMMAND ${CMAKE_COMMAND} -E true)
endif()

add_custom_target(SeeleEngine ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:Engine>
    COMMAND_EXPAND_LISTS
	DEPENDS Editor dll_copy)

install(
    TARGETS
        AssetViewer
        Editor
        Engine
        crcpp
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
