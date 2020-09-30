#
# Find SPIR-V Cross
#
# Try to find SPIR-V Cross library.
# This module defines the following variables:
# - SPIRV_INCLUDE_DIRS
# - SPIRV_LIBRARIES
# - SPIRV_FOUND
#
# The following variables can be set as arguments for the module.
# - SPIRV_ROOT : Root library directory of SPIR-V Cross
#
# References:
# - https://github.com/progschj/OpenGL-Examples/blob/master/cmake_modules/FindGLFW.cmake
# - https://bitbucket.org/Ident8/cegui-mk2-opengl3-renderer/src/befd47200265/cmake/FindGLFW.cmake
#

# Additional modules
include(FindPackageHandleStandardArgs)
find_path(SPIRV_INCLUDE_DIR
  NAMES spirv.h
  PATHS
    ${SPIRV_ROOT})

find_library(SPIRV_CORE_LIBRARY
NAMES spirv-cross-core${CMAKE_DEBUG_POSTFIX}
PATHS
  ${Seele_BINARY_DIR}/external/SPIRV-Cross)


find_library(SPIRV_GLSL_LIBRARY
NAMES spirv-cross-glsl${CMAKE_DEBUG_POSTFIX}
PATHS
  ${Seele_BINARY_DIR}/external/SPIRV-Cross)

find_library(SPIRV_REFLECT_LIBRARY
  NAMES spirv-cross-reflect${CMAKE_DEBUG_POSTFIX}
  PATHS
    ${Seele_BINARY_DIR}/external/SPIRV-Cross)


find_package_handle_standard_args(SPIRV DEFAULT_MSG SPIRV_CORE_LIBRARY SPIRV_GLSL_LIBRARY SPIRV_REFLECT_LIBRARY SPIRV_INCLUDE_DIR)

if(SPIRV_FOUND)  
  set(SPIRV_LIBRARIES ${SPIRV_CORE_LIBRARY} ${SPIRV_GLSL_LIBRARY} ${SPIRV_REFLECT_LIBRARY})
  set(SPIRV_INCLUDE_DIRS ${SPIRV_INCLUDE_DIR})
endif()

mark_as_advanced(SPIRV_INCLUDE_DIR SPIRV_LIBRARY)

