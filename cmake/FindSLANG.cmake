#
# Find SLANG
#
# Try to find SLANG library.
# This module defines the following variables:
# - SLANG_INCLUDE_DIRS
# - SLANG_LIBRARIES
# - SLANG_FOUND
#
# The following variables can be set as arguments for the module.
# - SLANG_ROOT : Root library directory of SLANG
#
# References:
# - https://github.com/progschj/OpenGL-Examples/blob/master/cmake_modules/FindSLANG.cmake
# - https://bitbucket.org/Ident8/cegui-mk2-opengl3-renderer/src/befd47200265/cmake/FindSLANG.cmake
#

# Additional modules
include(FindPackageHandleStandardArgs)

if (WIN32)
    set(SLANG_BINARY_PATH ${SLANG_ROOT}/bin/windows-${CMAKE_PLATFORM}/release/)
	# Find include files
	find_path(
		SLANG_INCLUDE_DIR
		NAMES slang.h
		PATHS
			$ENV{PROGRAMFILES}/include
			${SLANG_ROOT}
		DOC "The directory where SLANG/slang.h resides")

	# Find library files
	find_library(
		SLANG_LIBRARY
		NAMES slang.lib
		PATHS
			$ENV{PROGRAMFILES}/lib
			${SLANG_BINARY_PATH})

	find_file(
		SLANG_BINARY
		NAMES slang.dll
		PATHS
            ${SLANG_BINARY_PATH})

	find_file(
		SLANG_GLSLANG
		NAMES slang-glslang.dll
		PATHS
            ${SLANG_BINARY_PATH})
else()
	set(SLANG_BINARY_PATH ${SLANG_ROOT}/bin/linux-${CMAKE_PLATFORM}/release/)
	# Find include files
	find_path(
		SLANG_INCLUDE_DIR
		NAMES slang.h
		PATHS
			/usr/include
			/usr/local/include
			/sw/include
			/opt/local/include
			${SLANG_ROOT}
		DOC "The directory where GL/glfw.h resides")

	# Find library files
	# Try to use static libraries
	set(SLANG_LIBRARY "")
	find_library(
		SLANG_BINARY
		NAMES libslang.so
		PATHS
			/usr/lib64
			/usr/lib
			/usr/local/lib64
			/usr/local/lib
			/sw/lib
			/opt/local/lib
			${SLANG_BINARY_PATH}
		DOC "The SLANG library")
	
	find_file(
		SLANG_GLSLANG
		NAMES libslang-glslang.so
		PATHS	
			/usr/lib64
			/usr/lib
			/usr/local/lib64
			/usr/local/lib
			/sw/lib
			/opt/local/lib
			${SLANG_BINARY_PATH}
	)
endif()

# Handle REQUIRD argument, define *_FOUND variable
find_package_handle_standard_args(SLANG DEFAULT_MSG SLANG_INCLUDE_DIR SLANG_BINARY SLANG_GLSLANG)

# Define SLANG_LIBRARIES and SLANG_INCLUDE_DIRS
if (SLANG_FOUND)
    set(SLANG_BINARIES ${SLANG_BINARY} ${SLANG_GLSLANG})
	set(SLANG_LIBRARIES ${SLANG_LIBRARY})
	set(SLANG_INCLUDE_DIRS ${SLANG_INCLUDE_DIR})
endif()

# Hide some variables
mark_as_advanced(SLANG_INCLUDE_DIR SLANG_LIBRARY SLANG_GLSLANG SLANG_BINARY)