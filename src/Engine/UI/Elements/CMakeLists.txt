target_sources(Engine
    PRIVATE
        Button.h
        Button.cpp
        Element.h
        Element.cpp
        Label.h
        Label.cpp
        Panel.h
        Panel.cpp)

target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            Button.h
            Element.h
            Label.h
            Panel.h)
