include (ExternalProject)

set_property(DIRECTORY PROPERTY EP_BASE external)

set(DEPENDENCIES)
set(EXTRA_CMAKE_ARGS)
set(DEPENDENT_BINARIES "")
execute_process(COMMAND git submodule update --init --recursive -- ${CMAKE_SOURCE_DIR})

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
	BUILD_COMMAND ./b2 link=static
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


#--------------GLFW------------------------------
#list(APPEND DEPENDENCIES glfw)
#set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
#set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
#set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
#
#ExternalProject_Add(glfw
#	SOURCE_DIR ${GLFW_ROOT}
#	BINARY_DIR ${CMAKE_BINARY_DIR}/lib
#	INSTALL_COMMAND "")
#
#list(APPEND EXTRA_CMAKE_ARGS
#	-DGLFW_INCLUDE_DIRS=${GLFW_ROOT}/include
#	-DGLFW_LIBRARY=${CMAKE_BINARY_DIR}/lib/src/glfw3.lib
#	)
#
#--------------SLang------------------------------
list(APPEND DEPENDENCIES slang)
if(WIN32)
ExternalProject_Add(slang
	SOURCE_DIR ${SLANG_ROOT}
	BINARY_DIR ${CMAKE_BINARY_DIR}/lib
	CONFIGURE_COMMAND ""
	BUILD_COMMAND devenv /upgrade ${SLANG_ROOT}/source/slang/slang.vcxproj
		COMMAND msbuild -m /p:WindowsTargetPlatformVersion=10.0 ${SLANG_ROOT}/source/slang/slang.vcxproj
	INSTALL_COMMAND "")
elseif(UNIX)
ExternalProject_Add(slang
	SOURCE_DIR ${SLANG_ROOT}
	BINARY_DIR ${CMAKE_BINARY_DIR}/lib
	CONFIGURE_COMMAND ""
	BUILD_COMMAND premake5 config=${CMAKE_BUILD_TYPE}_${CMAKE_GENERATOR_PLATFORM}
	INSTALL_COMMAND "")
endif()
if(WIN32)
	string(TOLOWER bin/windows-${CMAKE_GENERATOR_PLATFORM}/${CMAKE_BUILD_TYPE}/slang.dll SLANG_BINARY)
	string(TOLOWER bin/windows-${CMAKE_GENERATOR_PLATFORM}/${CMAKE_BUILD_TYPE}/slang.lib SLANG_LIB_PATH)
	set(SLANG_LIB_PATH ${SLANG_ROOT}/${SLANG_LIB_PATH})
elseif(UNIX)
	string(TOLOWER bin/linux-${CMAKE_GENERATOR_PLATFORM}/${CMAKE_BUILD_TYPE}/libslang.so SLANG_BINARY)
	set(SLANG_LIB_PATH)
endif()

list(APPEND EXTRA_CMAKE_ARGS
	-DSLANG_INCLUDE_DIRS=${EXTERNAL_ROOT}
	-DSLANG_LIBRARY=${SLANG_LIB_PATH})
	
list(APPEND DEPENDENT_BINARIES ${SLANG_ROOT}/${SLANG_BINARY})

list(APPEND EXTRA_CMAKE_ARGS
	-DDEPENDENT_BINARIES=${DEPENDENT_BINARIES})
#-----------------SeeleEngine--------------------
ExternalProject_Add(SeeleEngine
	DEPENDS ${DEPENDENCIES}
	SOURCE_DIR ${PROJECT_SOURCE_DIR}
	CMAKE_ARGS -DUSE_SUPERBUILD=OFF ${EXTRA_CMAKE_ARGS}
	INSTALL_COMMAND ""
	BINARY_DIR ${CMAKE_BINARY_DIR})
