#
# Find TTFPARSER
#
# Try to find TTFPARSER library.
# This module defines the following variables:
# - TTFPARSER_INCLUDE_DIRS
# - TTFPARSER_LIBRARIES
# - TTFPARSER_FOUND
#
# The following variables can be set as arguments for the module.
# - TTFPARSER_ROOT : Root library directory of TTFPARSER
# - TTFPARSER_USE_STATIC_LIBS : Specifies to use static version of TTFPARSER library (Windows only)
#
# References:
# - https://github.com/progschj/OpenGL-Examples/blob/master/cmake_modules/FindTTFPARSER.cmake
# - https://bitbucket.org/Ident8/cegui-mk2-opengl3-renderer/src/befd47200265/cmake/FindTTFPARSER.cmake
#

# Additional modules
include(FindPackageHandleStandardArgs)

if (WIN32)
	# Find include files
	find_path(
		TTFPARSER_INCLUDE_DIR
		NAMES ttfParser.h
		PATHS
			$ENV{PROGRAMFILES}/include
			${TTFPARSER_ROOT}/src
		DOC "The directory where TTFPARSER/glfw.h resides")

else()
	# Find include files
	find_path(
		TTFPARSER_INCLUDE_DIR
		NAMES TTFPARSER/glfw3.h
		PATHS
			/usr/include
			/usr/local/include
			/sw/include
			/opt/local/include
		DOC "The directory where GL/glfw.h resides")

	# Find library files	
	find_file(
		TTFPARSER_LIBRARY
		NAMES libglfw${CMAKE_DEBUG_POSTFIX}.so
		PATHS	
			/usr/lib64
			/usr/lib
			/usr/local/lib64
			/usr/local/lib
			/sw/lib
			/opt/local/lib
			${TTFPARSER_ROOT}/src
	)
endif()

# Handle REQUIRD argument, define *_FOUND variable
find_package_handle_standard_args(TTFPARSER DEFAULT_MSG TTFPARSER_INCLUDE_DIR)

# Define TTFPARSER_LIBRARIES and TTFPARSER_INCLUDE_DIRS
if (TTFPARSER_FOUND)
	set(TTFPARSER_INCLUDE_DIRS ${TTFPARSER_INCLUDE_DIR})
endif()

# Hide some variables
mark_as_advanced(TTFPARSER_INCLUDE_DIR)