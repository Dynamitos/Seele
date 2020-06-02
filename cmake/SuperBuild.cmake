include (ExternalProject)

set(DEPENDENCIES)
set(EXTRA_CMAKE_ARGS)
execute_process(COMMAND git submodule update --init --recursive -- ${CMAKE_SOURCE_DIR})

#------------ASSIMP---------------
list(APPEND DEPENDENCIES assimp)
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "")
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE INTERNAL "")
set(ASSIMP_INSTALL OFF CACHE INTERNAL "")
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE INTERNAL "")

add_subdirectory(${ASSIMP_ROOT} ${ASSIMP_ROOT})
target_compile_definitions(assimp PRIVATE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)

#-------------BOOST----------------
list(APPEND DEPENDENCIES boost)
if(WIN32)
	set(BOOTSTRAP_EXTENSION bat)
else()
	set(BOOTSTRAP_EXTENSION sh)
endif()
ExternalProject_Add(boost
	SOURCE_DIR ${BOOST_ROOT}
	UPDATE_COMMAND ""
	CONFIGURE_COMMAND ./bootstrap.${BOOTSTRAP_EXTENSION} --with-libraries=serialization,test
	BUILD_COMMAND ./b2 link=static -d0
	BUILD_IN_SOURCE 1
	INSTALL_COMMAND "")

list (APPEND EXTRA_CMAKE_ARGS
	-DBoost_NO_SYSTEM_PATHS=ON)

#----------------GLM-----------------------
list(APPEND DEPENDENCIES glm)
ExternalProject_Add(glm
	SOURCE_DIR ${GLM_ROOT}
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND "")

list(APPEND EXTRA_CMAKE_ARGS
	-DGLM_INCLUDE_DIRS=${GLM_ROOT}
	)

#--------------------JSON------------------
list(APPEND DEPENDENCIES nlohmann_json)
set(JSON_MultipleHeaders ON CACHE INTERNAL "")
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")

add_subdirectory(${JSON_ROOT})

#--------------GLFW------------------------------
list(APPEND DEPENDENCIES glfw)
set(ENKITS_BUILD_EXAMPLES OFF CACHE BOOL "Build basic example applications" )
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL  "GLFW lib only" )
set(GLFW_BUILD_TESTS OFF CACHE BOOL  "GLFW lib only" )
set(GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
set(GLFW_BUILD_INSTALL OFF CACHE BOOL  "GLFW lib only" )

add_subdirectory(${GLFW_ROOT} ${GLFW_ROOT})

#---------------STB_IMAGE------------------------
list(APPEND EXTRA_CMAKE_ARGS
	-DSTB_INCLUDE_DIRS=${STB_ROOT})

#--------------SLang------------------------------
list(APPEND DEPENDENCIES slang)
string(TOLOWER release_${CMAKE_PLATFORM} SLANG_CONFIG)
if(WIN32)
ExternalProject_Add(slang
	SOURCE_DIR ${SLANG_ROOT}
	BINARY_DIR ${CMAKE_BINARY_DIR}/lib
	CONFIGURE_COMMAND devenv /upgrade ${SLANG_ROOT}/source/slang/slang.vcxproj
	BUILD_COMMAND msbuild -p:Configuration=Release -p:Platform=${CMAKE_PLATFORM} -p:WindowsTargetPlatformVersion=10.0 ${SLANG_ROOT}/source/slang/slang.vcxproj
	INSTALL_COMMAND "")

	string(TOLOWER bin/windows-${CMAKE_PLATFORM}/Release/slang.dll SLANG_BINARY)
	string(TOLOWER bin/windows-${CMAKE_PLATFORM}/Release/slang-glslang.dll SLANG_GLSLANG)
	string(TOLOWER bin/windows-${CMAKE_PLATFORM}/Release/slang.lib SLANG_LIB_PATH)
	set(SLANG_LIB_PATH ${SLANG_ROOT}/${SLANG_LIB_PATH})
elseif(UNIX)
ExternalProject_Add(slang
	SOURCE_DIR ${SLANG_ROOT}
	BINARY_DIR ${CMAKE_BINARY_DIR}/lib
	CONFIGURE_COMMAND ${CMAKE_SOURCE_DIR}/premake5 --file=${CMAKE_SOURCE_DIR}/external/slang/premake5.lua gmake2 --build-location=build.linux
	BUILD_COMMAND make -C ${CMAKE_SOURCE_DIR}/external/slang/build.linux config=${SLANG_CONFIG}
	INSTALL_COMMAND "")
	
	string(TOLOWER bin/linux-${CMAKE_PLATFORM}/Release/libslang.so SLANG_BINARY)
	string(TOLOWER bin/linux-${CMAKE_PLATFORM}/Release/libslang-glslang.so SLANG_GLSLANG)
	set(SLANG_LIB_PATH)
endif()

list(APPEND EXTRA_CMAKE_ARGS
	-DSLANG_INCLUDE_DIRS=${EXTERNAL_ROOT}/slang
	-DSLANG_LIBRARY=${SLANG_LIB_PATH}
	-DSLANG_BINARY=${SLANG_BINARY}
	-DSLANG_GLSLANG=${SLANG_GLSLANG})

#-----------------SeeleEngine--------------------
ExternalProject_Add(SeeleEngine
	DEPENDS ${DEPENDENCIES}
	SOURCE_DIR ${PROJECT_SOURCE_DIR}
	PREFIX ${CMAKE_BINARY_DIR}
	BINARY_DIR ${CMAKE_BINARY_DIR}
	CMAKE_ARGS -DUSE_SUPERBUILD=OFF ${EXTRA_CMAKE_ARGS}
	INSTALL_COMMAND "")
