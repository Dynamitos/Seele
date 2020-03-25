cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD_REQUIRED 17)

# Handle superbuild first
option (USE_SUPERBUILD "Whether or not a superbuild should be invoked" ON)

set(ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src/Engine)
set(EXTERNAL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(SLANG_ROOT ${EXTERNAL_ROOT}/slang)
set(BOOST_ROOT ${EXTERNAL_ROOT}/boost)
set(GLM_ROOT ${EXTERNAL_ROOT}/glm)
set(GLFW_ROOT ${EXTERNAL_ROOT}/glfw)
set(CMAKE_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})

# Configuration types
SET(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)
IF(DEFINED CMAKE_BUILD_TYPE AND CMAKE_VERSION VERSION_GREATER "2.8")
  SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS  ${CMAKE_CONFIGURATION_TYPES})
ENDIF()
SET(CMAKE_ARCHITECTURES "x86;x64" CACHE STRING "Architectures" FORCE)
IF(DEFINED CMAKE_GENERATOR_PLATFORM AND CMAKE_VERSION VERSION_GREATER "2.8")
  SET_PROPERTY(CACHE CMAKE_GENERATOR_PLATFORM PROPERTY STRINGS ${CMAKE_ARCHITECTURES})
ENDIF()

set(Boost_USE_STATIC_LIBS ON)
set(Boost_LIB_PREFIX lib)

set(ENKITS_BUILD_EXAMPLES OFF CACHE BOOL "Build basic example applications" )
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL  "GLFW lib only" )
set(GLFW_BUILD_TESTS OFF CACHE BOOL  "GLFW lib only" )
set(GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
set(GLFW_BUILD_INSTALL OFF CACHE BOOL  "GLFW lib only" )

if (USE_SUPERBUILD)
  project (SUPERBUILD NONE)
  # execute the superbuild (this script will be invoked again without the
  # USE_SUPERBUILD option this time)
  include (cmake/SuperBuild.cmake)
  return()                      # stop processing this file further
else()
  project (Seele)
endif()

add_subdirectory(${GLFW_ROOT})
find_package(Vulkan REQUIRED)
find_package(Boost REQUIRED COMPONENTS serialization)

include_directories(${GLFW_ROOT}/include)
include_directories(${GLM_INCLUDE_DIRS})
include_directories(${Vulkan_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${SLANG_INCLUDE_DIRS})
include_directories(src/Engine)
add_definitions(${GLM_DEFINITIONS})
add_definitions(${Vulkan_DEFINITIONS})
add_definitions(${Boost_DEFINITIONS})
add_compile_definitions(GLFW_WINDOWS)
if(WIN32)
	add_compile_definitions(USE_EXTENSIONS)
endif()
add_executable(SeeleEngine "")
add_subdirectory(src/)
target_link_libraries(SeeleEngine ${Boost_LIBRARIES})
target_link_libraries(SeeleEngine ${Vulkan_LIBRARY})
target_link_libraries(SeeleEngine glfw ${GLFW_LIBRARIES})
target_link_libraries(SeeleEngine ${SLANG_LIBRARY})

add_subdirectory(test/)

add_custom_command(TARGET SeeleEngine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DEPENDENT_BINARIES} $<TARGET_FILE_DIR:SeeleEngine>)

