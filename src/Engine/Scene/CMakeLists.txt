target_sources(Engine
    PRIVATE
        EventManager.h
        LightEnvironment.h
        LightEnvironment.cpp
        Util.h
        Scene.h
        Scene.cpp)

target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            EventManager.h
            LightEnvironment.h
            Util.h
            Scene.h)