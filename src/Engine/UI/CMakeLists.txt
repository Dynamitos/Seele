target_sources(Engine
    PRIVATE
        HorizontalLayout.h
        HorizontalLayout.cpp
        Layout.h
        Layout.cpp
        RenderHierarchy.h
        RenderHierarchy.cpp
        System.h
        System.cpp
        VerticalLayout.h
        VerticalLayout.cpp)

target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            HorizontalLayout.h
            Layout.h
            RenderHierarchy.h
            System.h
            VerticalLayout.h)


add_subdirectory(Elements/)