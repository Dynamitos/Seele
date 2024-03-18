include (ExternalProject)


#--------------SLang------------------------------
add_library(shader-slang-glslang SHARED IMPORTED)
add_library(shader-slang SHARED IMPORTED)
if(WIN32)
    set(SLANG_ROOT ${EXTERNAL_ROOT}/slang/bin/windows-x64/release)
    set_target_properties(shader-slang-glslang PROPERTIES IMPORTED_LOCATION ${SLANG_ROOT}/slang-glslang.dll)
    set_target_properties(shader-slang-glslang PROPERTIES IMPORTED_IMPLIB ${SLANG_ROOT}/slang.lib)
    set_target_properties(shader-slang PROPERTIES IMPORTED_LOCATION ${SLANG_ROOT}/slang.dll)
    set_target_properties(shader-slang PROPERTIES IMPORTED_IMPLIB ${SLANG_ROOT}/slang.lib)
    target_link_libraries(shader-slang INTERFACE shader-slang-glslang)
    install(FILES 
        ${SLANG_ROOT}/slang-glslang.dll
        ${SLANG_ROOT}/slang.dll
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
    install(FILES 
        ${SLANG_ROOT}/slang.lib
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
elseif(APPLE)
    set(SLANG_ROOT ${EXTERNAL_ROOT}/slang/bin/macosx-aarch64/release)
    set_target_properties(shader-slang-glslang PROPERTIES IMPORTED_LOCATION ${SLANG_ROOT}/libslang-glslang.dylib)
    set_target_properties(shader-slang PROPERTIES IMPORTED_LOCATION ${SLANG_ROOT}/libslang.dylib)
    target_link_libraries(shader-slang INTERFACE shader-slang-glslang)
    install(FILES
        ${SLANG_ROOT}/libslang.dylib
        ${SLANG_ROOT}/libslang-glslang.dylib
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endif()
target_include_directories(shader-slang INTERFACE 
    $<BUILD_INTERFACE:${EXTERNAL_ROOT}/slang/>
    $<INSTALL_INTERFACE:include>
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

add_library(metal INTERFACE)
target_include_directories(metal INTERFACE
    $<BUILD_INTERFACE:${METAL_ROOT}/>
    $<INSTALL_INTERFACE:include>
)
install(DIRECTORY
    ${METAL_ROOT}/
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include
)
target_link_libraries(metal INTERFACE
    "-framework Metal"
    "-framework MetalKit"
    "-framework AppKit"
    "-framework Foundation"
    "-framework QuartzCore"
)
