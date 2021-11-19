#
# Find Nsight Aftermath
#
# Try to find Nvidia Nsight aftermath library.
# This module defines the following variables:
# - NSAM_INCLUDE_DIRS
# - NSAM_LIBRARIES
# - NSAM_FOUND
#
# The following variables can be set as arguments for the module.
# - NSAM_ROOT : Root library directory of NSAM
# - NSAM_USE_STATIC_LIBS : Specifies to use static version of NSAM library (Windows only)
#
# References:
# - https://github.com/progschj/OpenGL-Examples/blob/master/cmake_modules/FindNSAM.cmake
# - https://bitbucket.org/Ident8/cegui-mk2-opengl3-renderer/src/befd47200265/cmake/FindNSAM.cmake
#

# Additional modules
include(FindPackageHandleStandardArgs)

if (WIN32)
	# Find include files
	find_path(
		NSAM_INCLUDE_DIR
		NAMES GFSDK_Aftermath.h
		PATHS
			$ENV{PROGRAMFILES}/include
			${NSAM_ROOT}/include
		DOC "The directory where GFSDK_Aftermath.h resides")


	# Find library files
	find_library(
		NSAM_LIBRARY
		NAMES GFSDK_Aftermath_Lib.${CMAKE_PLATFORM}.lib
		PATHS
			$ENV{PROGRAMFILES}/lib
			${NSAM_ROOT}/lib/${CMAKE_PLATFORM}
			${NSAM_ROOT}/lib
		PATH_SUFFIXES x86 x64)

	find_file(
		NSAM_BINARY
		NAMES GFSDK_Aftermath_Lib.${CMAKE_PLATFORM}.dll
		PATHS
			$ENV{PROGRAMFILES}/bin
			${NSAM_ROOT}/lib/${CMAKE_PLATFORM}
			${NSAM_ROOT}/lib
		PATH_SUFFIXES x86 x64)
    
    find_file(
        NSAM_LLVM
        NAMES llvm_7_0_1.dll
        PATHS
            $ENV{PROGRAMFILES}/bin
            ${NSAM_ROOT}/lib/${CMAKE_PLATFORM}
            ${NSAM_ROOT}/lib
        PATH_SUFFIXES x86 x64)
    
else()
	# Find include files
	find_path(
		NSAM_INCLUDE_DIR
		NAMES GFSDK_Aftermath.h
		PATHS
			/usr/include
			/usr/local/include
			/sw/include
			/opt/local/include
			${NSAM_ROOT}/include
		DOC "The directory where GL/nsam.h resides")

	find_file(
		NSAM_LIBRARY
		NAMES libGFSDK_Aftermath_Lib.${CMAKE_PLATFORM}.so
		PATHS	
			/usr/lib64
			/usr/lib
			/usr/local/lib64
			/usr/local/lib
			/sw/lib
			/opt/local/lib
			${NSAM_ROOT}/lib
		PATH_SUFFIXES x64 x86)
endif()

# Handle REQUIRD argument, define *_FOUND variable
find_package_handle_standard_args(NSAM DEFAULT_MSG NSAM_INCLUDE_DIR NSAM_LIBRARY)

# Define NSAM_LIBRARIES and NSAM_INCLUDE_DIRS
if (NSAM_FOUND)
	set(NSAM_LIBRARIES ${NSAM_LIBRARY})
	set(NSAM_INCLUDE_DIRS ${NSAM_INCLUDE_DIR})
    set(NSAM_BINARIES ${NSAM_BINARY})
endif()

# Hide some variables
mark_as_advanced(NSAM_INCLUDE_DIR NSAM_LIBRARY)