include (ExternalProject)


#--------------SLang------------------------------
add_library(slang SHARED IMPORTED)
if(WIN32)
    add_library(slang-glslang SHARED IMPORTED)
    set(SLANG_ROOT ${PROJECT_SOURCE_DIR}/../slang/build/Release/)
    set_target_properties(slang-glslang PROPERTIES IMPORTED_LOCATION ${SLANG_ROOT}bin/slang-glslang.dll)
    set_target_properties(slang-glslang PROPERTIES IMPORTED_IMPLIB ${SLANG_ROOT}lib/slang.lib)
    set_target_properties(slang PROPERTIES IMPORTED_LOCATION ${SLANG_ROOT}bin/slang.dll)
    set_target_properties(slang PROPERTIES IMPORTED_IMPLIB ${SLANG_ROOT}lib/slang.lib)
    target_link_libraries(slang INTERFACE slang-glslang)
    install(FILES 
        ${SLANG_ROOT}bin/slang-glslang.dll
        ${SLANG_ROOT}bin/slang.dll
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
    install(FILES 
        ${SLANG_ROOT}lib/slang.lib
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
elseif(APPLE)
    set(BINARY_ROOT ${PROJECT_SOURCE_DIR}/../slang/build/Debug)
    set_target_properties(slang PROPERTIES IMPORTED_LOCATION ${BINARY_ROOT}/lib/libslang.dylib)
    install(FILES
        ${BINARY_ROOT}/lib/libslang.dylib
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
endif()
target_include_directories(slang INTERFACE 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/../slang/include>
    $<INSTALL_INTERFACE:include>
)


#--------------CRC++------------------------------
add_library(crcpp INTERFACE)

target_include_directories(crcpp INTERFACE 
    $<BUILD_INTERFACE:${CRCPP_ROOT}/inc>
    $<INSTALL_INTERFACE:include/Seele/>
)
    
install(FILES
    ${CRCPP_ROOT}/inc/CRC.h
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Seele
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
