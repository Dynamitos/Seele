#
# Find GLM
#
# Try to find GLM library.
# This module defines the following variables:
# - GLM_INCLUDE_DIRS
# - GLM_LIBRARIES
# - GLM_FOUND
#
# The following variables can be set as arguments for the module.
# - GLM_ROOT : Root library directory of GLM
# - GLM_USE_STATIC_LIBS : Specifies to use static version of GLM library (Windows only)
#
# References:
# - https://github.com/progschj/OpenGL-Examples/blob/master/cmake_modules/FindGLM.cmake
# - https://bitbucket.org/Ident8/cegui-mk2-opengl3-renderer/src/befd47200265/cmake/FindGLM.cmake
#

# Additional modules
include(FindPackageHandleStandardArgs)

if (WIN32)
	# Find include files
	find_path(
		GLM_INCLUDE_DIR
		NAMES GLM/glm.hpp
		PATHS
			$ENV{PROGRAMFILES}/include
			${GLM_ROOT}
		DOC "The directory where GLM/glm.h resides")
else()
	# Find include files
	find_path(
		GLM_INCLUDE_DIR
		NAMES glm/glm.hpp
		PATHS
			/usr/include
			/usr/local/include
			/sw/include
			/opt/local/include
			${GLM_ROOT}
		DOC "The directory where GLM/glm.h resides")
endif()

# Handle REQUIRD argument, define *_FOUND variable
find_package_handle_standard_args(GLM DEFAULT_MSG GLM_INCLUDE_DIR)

# Define GLM_LIBRARIES and GLM_INCLUDE_DIRS
if (GLM_FOUND)
	set(GLM_INCLUDE_DIRS ${GLM_INCLUDE_DIR})
endif()

# Hide some variables
mark_as_advanced(GLM_INCLUDE_DIR)