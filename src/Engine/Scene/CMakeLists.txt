target_sources(Engine
    PRIVATE
        EventManager.h
        Util.h
        Scene.cpp
        Scene.h)

target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            EventManager.h
            Util.h
            Scene.h)