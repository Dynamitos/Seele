#
# Find FREETYPE
#
# Try to find FREETYPE library.
# This module defines the following variables:
# - FREETYPE_INCLUDE_DIRS
# - FREETYPE_LIBRARIES
# - FREETYPE_FOUND
#
# The following variables can be set as arguments for the module.
# - FREETYPE_ROOT : Root library directory of FREETYPE
# - FREETYPE_USE_STATIC_LIBS : Specifies to use static version of FREETYPE library (Windows only)
#
# References:
# - https://github.com/progschj/OpenGL-Examples/blob/master/cmake_modules/FindFREETYPE.cmake
# - https://bitbucket.org/Ident8/cegui-mk2-opengl3-renderer/src/befd47200265/cmake/FindFREETYPE.cmake
#

# Additional modules
include(FindPackageHandleStandardArgs)

if (WIN32)
	# Find include files
	find_path(
		FREETYPE_INCLUDE_DIR
		NAMES freetype/freetype.h
		PATHS
			$ENV{PROGRAMFILES}/include
			${FREETYPE_ROOT}/include
		DOC "The directory where FREETYPE/glfw.h resides")

	# Find library files
	find_library(
		FREETYPE_LIBRARY
		NAMES freetype${CMAKE_DEBUG_POSTFIX}.lib
		PATHS
			$ENV{PROGRAMFILES}/lib
			${FREETYPE_ROOT}/lib
			${FREETYPE_ROOT}
		PATH_SUFFIXES Debug Release)


	find_file(
		FREETYPE_BINARY
		NAMES freetype${CMAKE_DEBUG_POSTFIX}.dll
		PATHS
			$ENV{PROGRAMFILES}/bin
			${CMAKE_BINARY_DIR}
			${FREETYPE_ROOT}/src
			${FREETYPE_ROOT}
		PATH_SUFFIXES Debug Release)
else()
	# Find include files
	find_path(
		FREETYPE_INCLUDE_DIR
		NAMES FREETYPE/glfw3.h
		PATHS
			/usr/include
			/usr/local/include
			/sw/include
			/opt/local/include
		DOC "The directory where GL/glfw.h resides")

	# Find library files	
	find_file(
		FREETYPE_LIBRARY
		NAMES libglfw${CMAKE_DEBUG_POSTFIX}.so
		PATHS	
			/usr/lib64
			/usr/lib
			/usr/local/lib64
			/usr/local/lib
			/sw/lib
			/opt/local/lib
			${FREETYPE_ROOT}/src
	)
endif()

# Handle REQUIRD argument, define *_FOUND variable
find_package_handle_standard_args(FREETYPE DEFAULT_MSG FREETYPE_INCLUDE_DIR FREETYPE_LIBRARY FREETYPE_BINARY)

# Define FREETYPE_LIBRARIES and FREETYPE_INCLUDE_DIRS
if (FREETYPE_FOUND)
	set(FREETYPE_LIBRARIES ${FREETYPE_LIBRARY})
	set(FREETYPE_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIR})
endif()

# Hide some variables
mark_as_advanced(FREETYPE_INCLUDE_DIR FREETYPE_LIBRARY)