#
# Find nlohmann_json
#
# Try to find nlohmann_json library
# This module defines the following variables:
# - JSON_INCLUDE_DIRS
# - JSON_FOUND
#
# The following variables can be set as arguments for the module.
# - JSON_ROOT : Root library directory of json
#

# Additional modules
include(FindPackageHandleStandardArgs)

if(JSON_MultipleHeaders)
    find_path(
        JSON_INCLUDE_DIR
        NAMES nlohmann/json.hpp
        PATHS
        $ENV{PROGRAMFILES}/include
        ${JSON_ROOT}/single_include
        DOC "The directory where nlohmann/json.hpp resides")
else()
    find_path(
        JSON_INCLUDE_DIR
        NAMES nlohmann/json.hpp
        PATHS
        $ENV{PROGRAMFILES}/include
        ${JSON_ROOT}/include
        DOC "The directory where nlohmann/json.hpp resides")
endif()

find_package_handle_standard_args(JSON DEFAULT_MSG JSON_INCLUDE_DIR)

if(JSON_FOUND)
    set(JSON_INCLUDE_DIRS ${JSON_INCLUDE_DIR})
endif()

# Hide some variables
mark_as_advanced(JSON_INCLUDE_DIR)