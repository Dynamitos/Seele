target_sources(Engine
    PRIVATE
        View.h
        View.cpp
        Window.cpp
        Window.h
        WindowManager.h
        WindowManager.cpp)

target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            View.h
            Window.h
            WindowManager.h)
