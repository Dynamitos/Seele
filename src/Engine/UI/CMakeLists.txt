target_sources(Engine
    PRIVATE
        Attributes.h
        Element.h
        Element.cpp
        
        )

target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
        Attributes.h
        Element.h
)

add_subdirectory(Style/)
add_subdirectory(Element/)