#
# Find Assimp
#
# Try to find Assimp : Open Asset Import Library.
# This module defines the following variables:
# - ASSIMP_INCLUDE_DIRS
# - ASSIMP_LIBRARIES
# - ASSIMP_FOUND
#
# The following variables can be set as arguments for the module.
# - ASSIMP_ROOT : Root library directory of Assimp
#

# Additional modules
include(FindPackageHandleStandardArgs)
if (WIN32)
	# Find include files
	find_path(
		ASSIMP_INCLUDE_DIR
		NAMES assimp/scene.h
		PATHS
			$ENV{PROGRAMFILES}/include
			${ASSIMP_ROOT}/include
		DOC "The directory where assimp/scene.h resides")

	# Find library files
	find_library(
		ASSIMP_LIBRARY
		NAMES assimp-vc${MSVC_TOOLSET_VERSION}-mt${CMAKE_DEBUG_POSTFIX}.lib
		PATHS
			$ENV{PROGRAMFILES}/lib
			${ASSIMP_ROOT}/lib
			${ASSIMP_ROOT}/lib/${CMAKE_BUILD_TYPE})

	find_file(
		ASSIMP_BINARY
		NAMES assimp-vc${MSVC_TOOLSET_VERSION}-mt${CMAKE_DEBUG_POSTFIX}.dll
		PATHS
			$ENV{PROGRAMFILES}/bin
			${ASSIMP_ROOT}/bin)
else()
	# Find include files
	find_path(
		ASSIMP_INCLUDE_DIR
		NAMES assimp/scene.h
		PATHS
			/usr/include
			/usr/local/include
			/sw/include
			/opt/local/include
			${ASSIMP_ROOT}/include
		DOC "The directory where assimp/scene.h resides")

	# Find library files
	find_library(
		ASSIMP_LIBRARY
		NAMES libassimp${CMAKE_DEBUG_POSTFIX}.so
		PATHS
			/usr/lib64
			/usr/lib
			/usr/local/lib64
			/usr/local/lib
			/sw/lib
			/opt/local/lib
			${ASSIMP_ROOT}/bin
		DOC "The Assimp library")
endif()

# Handle REQUIRD argument, define *_FOUND variable
find_package_handle_standard_args(assimp DEFAULT_MSG ASSIMP_INCLUDE_DIR ASSIMP_LIBRARY)

# Define GLFW_LIBRARIES and GLFW_INCLUDE_DIRS
if (ASSIMP_FOUND)
	set(ASSIMP_LIBRARIES ${ASSIMP_LIBRARY})
	set(ASSIMP_INCLUDE_DIRS ${ASSIMP_INCLUDE_DIR})
	set(ASSIMP_BINARIES ${ASSIMP_BINARY})
endif()

# Hide some variables
mark_as_advanced(ASSIMP_INCLUDE_DIR ASSIMP_LIBRARY, ASSIMP_BINARY)