include (ExternalProject)

#------------ASSIMP---------------
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "")
set(ASSIMP_BUILD_SAMPLES OFF CACHE INTERNAL "")
set(ASSIMP_BUILD_OVERALLS OFF CACHE INTERNAL "")
set(BUILD_SHARED_LIBS ON CACHE INTERNAL "")

add_subdirectory(${ASSIMP_ROOT})
target_compile_definitions(assimp PRIVATE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
if(WIN32)
    target_compile_options(assimp PRIVATE /WX-)
else()
    target_compile_options(assimp PRIVATE -Wno-error -fPIC)
    target_compile_options(IrrXML PRIVATE -fPIC)
endif()
#-------------BOOST----------------
add_subdirectory(${BOOST_ROOT})

#-----------------KTX----------------------------
set(KTX_FEATURE_TESTS off)

add_subdirectory(${KTX_ROOT} ${KTX_ROOT})

#--------------------JSON------------------
set(JSON_MultipleHeaders ON CACHE INTERNAL "")
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")

add_subdirectory(${JSON_ROOT})

#--------------GLM------------------------------
add_subdirectory(${GLM_ROOT})

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

#--------------FreeType------------------------------
add_subdirectory(${FREETYPE_ROOT})

#--------------STB-----------------------------------
add_library(stb INTERFACE)

target_include_directories(stb INTERFACE ${STB_ROOT})

#--------------Aftermath------------------------------
add_library(nsam INTERFACE)

target_include_directories(nsam INTERFACE ${NSAM_ROOT}/include)
target_link_libraries(nsam INTERFACE ${NSAM_ROOT}/lib/${CMAKE_PLATFORM}/*.lib)
set_target_properties(nsam PROPERTIES NSAM_BINARY ${NSAM_ROOT}/lib/${CMAKE_PLATFORM}/GFSDK_Aftermath_Lib.${CMAKE_PLATFORM}.dll)
set_target_properties(nsam PROPERTIES LLVM_BINARY ${NSAM_ROOT}/lib/${CMAKE_PLATFORM}/llvm_7_0_1.dll)


#--------------SLang------------------------------
string(TOLOWER release_${CMAKE_PLATFORM} SLANG_CONFIG)
if(WIN32)
string(TOLOWER ${SLANG_ROOT}/bin/windows-${CMAKE_PLATFORM}/release SLANG_BINARY_DIR)
ExternalProject_Add(slang-build
    SOURCE_DIR ${SLANG_ROOT}
    BINARY_DIR ${SLANG_ROOT}
    CONFIGURE_COMMAND ${SLANG_ROOT}/premake.bat vs2019 --file=${CMAKE_SOURCE_DIR}/external/slang/premake5.lua gmake --arch=${CMAKE_PLATFORM} --deps=true
    BUILD_COMMAND msbuild slang.sln -p:Configuration=Release -p:Platform=${CMAKE_PLATFORM}
    INSTALL_COMMAND "")
elseif(UNIX)
ExternalProject_Add(slang-build
    SOURCE_DIR ${SLANG_ROOT}
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}
    CONFIGURE_COMMAND ${CMAKE_SOURCE_DIR}/premake5 --file=${CMAKE_SOURCE_DIR}/external/slang/premake5.lua gmake --arch=${CMAKE_PLATFORM} --deps=true --build-location=build/linux
    BUILD_COMMAND make -C ${CMAKE_SOURCE_DIR}/external/slang/build/linux config=${SLANG_CONFIG}
    INSTALL_COMMAND "")
endif()

add_library(slang INTERFACE)

target_include_directories(slang INTERFACE ${SLANG_ROOT})
target_link_libraries(slang INTERFACE ${SLANG_BINARY_DIR}/*.lib)
set_target_properties(slang PROPERTIES SLANG_BINARY ${SLANG_BINARY_DIR}/slang.dll)
set_target_properties(slang PROPERTIES GLSLANG_BINARY ${SLANG_BINARY_DIR}/slang-glslang.dll)
