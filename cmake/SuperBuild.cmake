include (ExternalProject)

#--------------ZLIB------------------------------
add_subdirectory(${ZLIB_ROOT})

#--------------FreeType------------------------------
add_subdirectory(${FREETYPE_ROOT})

#------------ASSIMP---------------
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "")
set(ASSIMP_BUILD_SAMPLES OFF CACHE INTERNAL "")
set(ASSIMP_BUILD_OVERALLS OFF CACHE INTERNAL "")
add_subdirectory(${ASSIMP_ROOT})
target_compile_definitions(assimp PRIVATE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
if(WIN32)
    target_compile_options(assimp PRIVATE /WX-)
else()
    target_compile_options(assimp PRIVATE -Wno-error -fPIC)
    target_compile_options(IrrXML PRIVATE -fPIC)
endif()

#-----------------KTX----------------------------
set(KTX_FEATURE_TESTS off)

add_subdirectory(${KTX_ROOT} ${KTX_ROOT})

#--------------------JSON------------------
set(JSON_MultipleHeaders ON CACHE INTERNAL "")
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install ON CACHE INTERNAL "")

add_subdirectory(${JSON_ROOT})


#--------------GLFW------------------------------
set(ENKITS_BUILD_EXAMPLES OFF CACHE BOOL "Build basic example applications" )
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL  "GLFW lib only" )
set(GLFW_BUILD_TESTS OFF CACHE BOOL  "GLFW lib only" )
set(GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
set(GLFW_BUILD_INSTALL OFF CACHE BOOL  "GLFW lib only" )
set(GLFW_VULKAN_STATIC OFF CACHE BOOL "GLFW vulkan static")

add_subdirectory(${GLFW_ROOT})

#--------------EnTT------------------------------
add_subdirectory(${ENTT_ROOT})

#--------------thread-pool------------------------------
add_subdirectory(${THREADPOOL_ROOT})

target_compile_options(ThreadPool INTERFACE "/WX-")

#--------------SLang------------------------------
string(TOLOWER release_${CMAKE_PLATFORM} SLANG_CONFIG)
if(WIN32)
string(TOLOWER ${SLANG_ROOT}/bin/windows-${CMAKE_PLATFORM}/release SLANG_BINARY_DIR)
ExternalProject_Add(slang-build
    SOURCE_DIR ${SLANG_ROOT}
    BINARY_DIR ${SLANG_ROOT}
    CONFIGURE_COMMAND ${SLANG_ROOT}/premake.bat vs2019 --file=${SLANG_ROOT}/premake5.lua gmake --arch=${CMAKE_PLATFORM} --deps=true
    BUILD_COMMAND msbuild slang.sln -p:Configuration=Release -p:Platform=${CMAKE_PLATFORM}
    INSTALL_COMMAND ""
)
elseif(UNIX)
ExternalProject_Add(slang-build
    SOURCE_DIR ${SLANG_ROOT}
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}
    CONFIGURE_COMMAND ${CMAKE_SOURCE_DIR}/premake5 --file=${CMAKE_SOURCE_DIR}/external/slang/premake5.lua gmake --arch=${CMAKE_PLATFORM} --deps=true --build-location=build/linux
    BUILD_COMMAND make -C ${CMAKE_SOURCE_DIR}/external/slang/build/linux config=${SLANG_CONFIG}
    INSTALL_COMMAND ""
)
endif()

add_library(slang INTERFACE)

target_include_directories(slang INTERFACE 
    $<BUILD_INTERFACE:${SLANG_ROOT}/>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(slang INTERFACE ${SLANG_BINARY_DIR}/*.lib)
set_target_properties(slang PROPERTIES SLANG_BINARY ${SLANG_BINARY_DIR}/slang.dll)
set_target_properties(slang PROPERTIES GLSLANG_BINARY ${SLANG_BINARY_DIR}/slang-glslang.dll)

install(DIRECTORY
    ${SLANG_ROOT}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include
    FILES_MATCHING PATTERN "*.h"
)

#--------------CRC++------------------------------
add_library(crcpp INTERFACE)

target_include_directories(crcpp INTERFACE 
    $<BUILD_INTERFACE:${CRCPP_ROOT}/inc>
    $<INSTALL_INTERFACE:include>
)
    
install(FILES
    ${CRCPP_ROOT}/inc/CRC.h
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include
)

#--------------GLM------------------------------
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE 
    $<BUILD_INTERFACE:${GLM_ROOT}>
    $<INSTALL_INTERFACE:include/glm>
)

install(DIRECTORY
    ${GLM_ROOT}/glm
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include
    FILES_MATCHING PATTERN "*.h*"
)

install(DIRECTORY
    ${GLM_ROOT}/glm
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include
    FILES_MATCHING PATTERN "*.inl"
)

#--------------STB-----------------------------------
add_library(stb INTERFACE)

target_include_directories(stb INTERFACE 
    $<BUILD_INTERFACE:${STB_ROOT}>
    $<INSTALL_INTERFACE:include>
)

install(DIRECTORY
    ${STB_HEADERS}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include
    FILES_MATCHING PATTERN "*.h"
)

