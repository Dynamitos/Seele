#
# Find GLFW
#
# Try to find GLFW library.
# This module defines the following variables:
# - GLFW_INCLUDE_DIRS
# - GLFW_LIBRARIES
# - GLFW_FOUND
#
# The following variables can be set as arguments for the module.
# - GLFW_ROOT : Root library directory of GLFW
# - GLFW_USE_STATIC_LIBS : Specifies to use static version of GLFW library (Windows only)
#
# References:
# - https://github.com/progschj/OpenGL-Examples/blob/master/cmake_modules/FindGLFW.cmake
# - https://bitbucket.org/Ident8/cegui-mk2-opengl3-renderer/src/befd47200265/cmake/FindGLFW.cmake
#

# Additional modules
include(FindPackageHandleStandardArgs)

if (WIN32)
	# Find include files
	find_path(
		GLFW_INCLUDE_DIR
		NAMES GLFW/glfw3.h
		PATHS
			$ENV{PROGRAMFILES}/include
			${GLFW_ROOT}/include
		DOC "The directory where GLFW/glfw.h resides")

	# Use glfw3(d).lib for static library
	if (GLFW_USE_STATIC_LIBS)
		set(GLFW_LIBRARY_NAME glfw3${CMAKE_DEBUG_POSTFIX})
	else()
		set(GLFW_LIBRARY_NAME glfw3${CMAKE_DEBUG_POSTFIX}dll)
	endif()

	# Find library files
	find_library(
		GLFW_LIBRARY
		NAMES ${GLFW_LIBRARY_NAME}.lib
		PATHS
			$ENV{PROGRAMFILES}/lib
			${GLFW_ROOT}/lib
			${GLFW_ROOT}/src
		PATH_SUFFIXES Debug Release)

	unset(GLFW_LIBRARY_NAME)

	#find_file(
	#	GLFW_BINARY
	#	NAMES glfw3${CMAKE_DEBUG_POSTFIX}.dll
	#	PATHS
	#		$ENV{PROGRAMFILES}/bin
	#		${CMAKE_BINARY_DIR}
	#		${GLFW_ROOT}/src
	#		${GLFW_ROOT}/bin
	#	PATH_SUFFIXES Debug Release)
else()
	# Find include files
	find_path(
		GLFW_INCLUDE_DIR
		NAMES GLFW/glfw3.h
		PATHS
			/usr/include
			/usr/local/include
			/sw/include
			/opt/local/include
		DOC "The directory where GL/glfw.h resides")

	# Find library files	
	find_file(
		GLFW_LIBRARY
		NAMES libglfw${CMAKE_DEBUG_POSTFIX}.so
		PATHS	
			/usr/lib64
			/usr/lib
			/usr/local/lib64
			/usr/local/lib
			/sw/lib
			/opt/local/lib
			${GLFW_ROOT}/src
	)
endif()

# Handle REQUIRD argument, define *_FOUND variable
find_package_handle_standard_args(GLFW DEFAULT_MSG GLFW_INCLUDE_DIR GLFW_LIBRARY)

# Define GLFW_LIBRARIES and GLFW_INCLUDE_DIRS
if (GLFW_FOUND)
	set(GLFW_LIBRARIES ${GLFW_LIBRARY})
	set(GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIR})
endif()

# Hide some variables
mark_as_advanced(GLFW_INCLUDE_DIR GLFW_LIBRARY)