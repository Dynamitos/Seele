include (ExternalProject)

set_property(DIRECTORY PROPERTY EP_BASE external)

set(DEPENDENCIES)
set(EXTRA_CMAKE_ARGS)

#-------------BOOST----------------
list(APPEND DEPENDENCIES boost)
ExternalProject_Add(boost
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/boost
	CONFIGURE_COMMAND ./bootstrap.sh --with-libraries=serialization
	BUILD_COMMAND ./b2 link=static
	BUILD_IN_SOURCE 1
	INSTALL_COMMAND "")

list (APPEND EXTRA_CMAKE_ARGS
	-DBOOST_ROOT=${CMAKE_CURRENT_SOURCE_DIR}/external/boost
	-DBoost_NO_SYSTEM_PATHS=ON)

#----------------GLM-----------------------
list(APPEND DEPENDENCIES glm)
ExternalProject_Add(glm
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/glm
	BINARY_DIR ${CMAKE_BINARY_DIR}/lib
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND "")

list(APPEND EXTRA_CMAKE_ARGS
	-DGLM_INCLUDE_DIRS=${CMAKE_CURRENT_SOURCE_DIR}/external/glm
	)

#--------------GLFW------------------------------
list(APPEND DEPENDENCIES glfw)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

ExternalProject_Add(glfw
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw
	BINARY_DIR ${CMAKE_BINARY_DIR}/lib/glfw
	INSTALL_COMMAND "")

list(APPEND EXTRA_CMAKE_ARGS
	-DGLFW_INCLUDE_DIRS=${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include
	-DGLFW_LIBRARY=${CMAKE_BINARY_DIR}/lib/glfw/src/glfw3.lib
	)

#--------------SLang------------------------------
list(APPEND DEPENDENCIES slang)
ExternalProject_Add(slang
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/slang
	BINARY_DIR ${CMAKE_BINARY_DIR}/lib
	CONFIGURE_COMMAND ""
	
	BUILD_COMMAND msbuild -m /p:WindowsTargetPlatformVersion=10.0 ${CMAKE_SOURCE_DIR}/external/slang/slang.sln

	INSTALL_COMMAND "")


#-----------------SeeleEngine--------------------
ExternalProject_Add(SeeleEngine
	DEPENDS ${DEPENDENCIES}
	SOURCE_DIR ${PROJECT_SOURCE_DIR}
	CMAKE_ARGS -DUSE_SUPERBUILD=OFF ${EXTRA_CMAKE_ARGS}
	INSTALL_COMMAND ""
	BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)