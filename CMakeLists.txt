cmake_minimum_required(VERSION 3.7...3.23)
if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()
set(CMAKE_CXX_STANDARD 23)
# Handle superbuild first
option (USE_SUPERBUILD "Whether or not a superbuild should be invoked" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR OFF)
set(CMAKE_INSTALL_MESSAGE LAZY)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(VCPKG_INSTALL_OPTIONS "--allow-unsupported")
set(VCPKG_BUILD_TYPE release)

set(ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src/Engine)
set(EXTERNAL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/external)

set(CMAKE_TOOLCHAIN_FILE ${EXTERNAL_ROOT}/vcpkg/scripts/buildsystems/vcpkg.cmake)

set(CRCPP_ROOT ${EXTERNAL_ROOT}/CRCpp)
set(METAL_ROOT ${EXTERNAL_ROOT}/metal-cpp)
if(WIN32)
    set(VCPKG_BASE_FOLDER ${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/)
elseif(APPLE)
    set(VCPKG_BASE_FOLDER ${CMAKE_BINARY_DIR}/vcpkg_installed/arm64-osx/)
else()    
    set(VCPKG_BASE_FOLDER ${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/)
endif()

set(Boost_NO_WARN_NEW_VERSIONS 1)

project (Seele)

include(cmake/SuperBuild.cmake)

find_package(Vulkan REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(FreeType CONFIG REQUIRED)
find_package(harfbuzz CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Ktx CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(lunasvg CONFIG REQUIRED)
find_package(metis CONFIG REQUIRED)
find_package(meshoptimizer CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

if(UNIX)
    find_package(Threads REQUIRED)
endif()

if(WIN32)
    add_compile_definitions(USE_EXTENSIONS)
endif()

add_library(Engine SHARED "")

target_include_directories(Engine PRIVATE src/Engine)
target_include_directories(Engine PRIVATE ${VCPKG_BASE_FOLDER}/include/)
target_link_libraries(Engine PUBLIC Vulkan::Vulkan)
target_link_libraries(Engine PUBLIC EnTT::EnTT)
target_link_libraries(Engine PUBLIC glfw)
target_link_libraries(Engine PUBLIC glm::glm)
target_link_libraries(Engine PUBLIC freetype)
target_link_libraries(Engine PUBLIC harfbuzz::harfbuzz)
target_link_libraries(Engine PUBLIC harfbuzz::harfbuzz-subset)
target_link_libraries(Engine PUBLIC assimp::assimp)
target_link_libraries(Engine PUBLIC KTX::ktx)
target_link_libraries(Engine PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(Engine PUBLIC crcpp)
target_link_libraries(Engine PUBLIC fmt::fmt)
target_link_libraries(Engine PUBLIC lunasvg::lunasvg)
target_link_libraries(Engine PUBLIC GPUOpen::VulkanMemoryAllocator)
target_link_libraries(Engine PUBLIC metis)
target_link_libraries(Engine PUBLIC meshoptimizer::meshoptimizer)
if(WIN32)
    target_link_libraries(Engine PUBLIC ${VCPKG_BASE_FOLDER}/lib/slang.lib)
elseif(APPLE)
    target_link_libraries(Engine PUBLIC ${VCPKG_BASE_FOLDER}/debug/lib/libslang.dylib)
else()
    target_link_libraries(Engine PUBLIC ${VCPKG_BASE_FOLDER}/lib/libslang.so)
    target_link_libraries(Engine PUBLIC ${VCPKG_BASE_FOLDER}/lib/libslang-glslang.so)
endif()

if(APPLE)
    target_link_libraries(Engine PUBLIC metal)
    SET(CMAKE_OSX_DEPLOYMENT_TARGET 15.0)
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

add_executable(Benchmark "")
target_link_libraries(Benchmark PRIVATE Engine)
target_include_directories(Benchmark PRIVATE src/Benchmark)

if(MSVC)
    target_compile_definitions(Engine PUBLIC _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(Engine PUBLIC WIN32)
    target_compile_options(Engine PUBLIC /Zi /MP14 /W4 /wd4505)
    target_compile_options(Editor PUBLIC /Zi /MP14 /W4)
    target_sources(Engine INTERFACE   
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Seele.natvis>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/Seele/Seele.natvis>
    )
    install(FILES
        Seele.natvis
        DESTINATION Seele/)
else()
    target_compile_options(Engine PUBLIC -g -Wall -Wextra -Wno-error -pedantic -Wno-missing-field-initializers -Wno-unused-function)
    target_compile_options(Editor PUBLIC -g -Wall -Wextra -Wno-error -pedantic)
endif()
if(APPLE)
  #Metal lib throws that internally for some reason
  target_compile_options(Engine PUBLIC -Wno-gnu-anonymous-struct -Wno-nested-anon-types)
  target_compile_options(Editor PUBLIC -Wno-gnu-anonymous-struct -Wno-nested-anon-types)
endif()
target_compile_options(Engine PUBLIC "$<$<CONFIG:DEBUG>:-DENABLE_VALIDATION>")
target_compile_options(Engine PUBLIC "$<$<CONFIG:DEBUG>:-DSEELE_DEBUG>")

add_subdirectory(src/)
#add_subdirectory(tests/)

if(WIN32)
    add_custom_target(dll_copy ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:Engine> $<TARGET_FILE_DIR:Editor>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${VCPKG_BASE_FOLDER}/bin/slang.dll $<TARGET_FILE_DIR:Editor>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${VCPKG_BASE_FOLDER}/bin/slang-glslang.dll $<TARGET_FILE_DIR:Editor>
        COMMAND_EXPAND_LISTS
        DEPENDS Editor)
elseif(APPLE)
    add_custom_target(dll_copy ALL
        COMMAND ${CMAKE_COMMAND} -E true
        COMMAND_EXPAND_LISTS
        DEPENDS Editor)
else()
    add_custom_target(dll_copy ALL
        COMMAND ${CMAKE_COMMAND} -E true)
endif()

add_custom_target(SeeleEngine ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res $<TARGET_FILE_DIR:Engine>
    COMMAND_EXPAND_LISTS
	DEPENDS Editor dll_copy)

install(
    TARGETS
        AssetViewer
        Editor
        Engine
        crcpp
        metal
    EXPORT
        EngineTargets
    FILE_SET HEADERS
    DESTINATION include/Seele)

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
        lib/cmake/Seele
)

install(
    FILES
        ${CMAKE_BINARY_DIR}/SeeleConfig.cmake
    DESTINATION
        lib/cmake/Seele
)

install(
    DIRECTORY
        ${CMAKE_SOURCE_DIR}/res/cmake
        ${CMAKE_SOURCE_DIR}/res/fonts
        ${CMAKE_SOURCE_DIR}/res/shaders
        ${CMAKE_SOURCE_DIR}/res/textures
    DESTINATION
        bin
)
