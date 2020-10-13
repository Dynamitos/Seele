#
# Find STB
#
# Try to find STB library.
# This module defines the following variables:
# - STB_INCLUDE_DIRS
# - STB_LIBRARIES
# - STB_FOUND
#
# The following variables can be set as arguments for the module.
# - STB_ROOT : Root library directory of STB
# - STB_USE_STATIC_LIBS : Specifies to use static version of STB library (Windows only)
#
# References:
# - https://github.com/progschj/OpenGL-Examples/blob/master/cmake_modules/FindSTB.cmake
# - https://bitbucket.org/Ident8/cegui-mk2-opengl3-renderer/src/befd47200265/cmake/FindSTB.cmake
#

# Additional modules
include(FindPackageHandleStandardArgs)

if (WIN32)
	# Find include files
	find_path(
		STB_INCLUDE_DIR
		NAMES stb.h
		PATHS
			$ENV{PROGRAMFILES}/include
			${STB_ROOT}
		DOC "The directory where STB/stb.h resides")
else()
	# Find include files
	find_path(
		STB_INCLUDE_DIR
		NAMES stb.h
		PATHS
			/usr/include
			/usr/local/include
			/sw/include
			/opt/local/include
		DOC "The directory where STB/stb.h resides")
endif()

# Handle REQUIRD argument, define *_FOUND variable
find_package_handle_standard_args(STB DEFAULT_MSG STB_INCLUDE_DIR)

# Define STB_LIBRARIES and STB_INCLUDE_DIRS
if (STB_FOUND)
	set(STB_INCLUDE_DIRS ${STB_INCLUDE_DIR})
endif()

# Hide some variables
mark_as_advanced(STB_INCLUDE_DIR)