#
# Find KTX
#
# Try to find KTX library.
# This module defines the following variables:
# - KTX_INCLUDE_DIRS
# - KTX_LIBRARIES
# - KTX_FOUND
#
# The following variables can be set as arguments for the module.
# - KTX_ROOT : Root library directory of KTX
# - KTX_USE_STATIC_LIBS : Specifies to use static version of KTX library (Windows only)
#
#

# Additional modules
include(FindPackageHandleStandardArgs)

if (WIN32)
	# Find include files
	find_path(
		KTX_INCLUDE_DIR
		NAMES ktx.h
		PATHS
			$ENV{PROGRAMFILES}/include
			${KTX_ROOT}/include
		DOC "The directory where ktx.h resides")

	# Find library files
	find_library(
		KTX_LIBRARY
		NAMES ktx${CMAKE_DEBUG_POSTFIX}.lib
		PATHS
			$ENV{PROGRAMFILES}/lib
			${KTX_ROOT}
			${KTX_ROOT}/lib
		PATH_SUFFIXES Debug Release)
else()
	# Find include files
	find_path(
		KTX_INCLUDE_DIR
		NAMES ktx.h
		PATHS
			/usr/include
			/usr/local/include
			/sw/include
			/opt/local/include
			${KTX_ROOT}
		DOC "The directory where KTX/ktx.h resides")

	find_library(
		KTX_LIBRARY
		NAMES libktx${CMAKE_DEBUG_POSTFIX}.so
		PATHS
			/usr/include
			/usr/local/include
			/sw/include
			/opt/local/include
			${KTX_ROOT}
		DOC "The directory where KTX/libktx.so resides")
endif()

# Handle REQUIRD argument, define *_FOUND variable
find_package_handle_standard_args(KTX DEFAULT_MSG KTX_INCLUDE_DIR KTX_LIBRARY)

# Define KTX_LIBRARIES and KTX_INCLUDE_DIRS
if (KTX_FOUND)
	set(KTX_INCLUDE_DIRS ${KTX_INCLUDE_DIR})
	set(KTX_LIBRARIES ${KTX_LIBRARY})
endif()

# Hide some variables
mark_as_advanced(KTX_INCLUDE_DIR)
mark_as_advanced(KTX_LIBRARY)