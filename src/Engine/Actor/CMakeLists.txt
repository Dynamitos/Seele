target_sources(Engine
    PRIVATE
        Actor.cpp
        Actor.h
        CameraActor.cpp
        CameraActor.h
        Entity.cpp
        Entity.h)

target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            Actor.h
            CameraActor.h
            Entity.h)
