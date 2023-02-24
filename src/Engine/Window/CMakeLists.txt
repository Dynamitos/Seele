target_sources(Engine
    PRIVATE
        GameView.h
        GameView.cpp
        View.h
        View.cpp
        Window.cpp
        Window.h
        WindowManager.h
        WindowManager.cpp)

target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            GameView.h
            View.h
            Window.h
            WindowManager.h)
