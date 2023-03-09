target_sources(Engine
    PRIVATE
        Actor.cpp
        Actor.h
        CameraActor.cpp
        CameraActor.h
        DirectionalLightActor.cpp
        DirectionalLightActor.h
        Entity.cpp
        Entity.h
        PointLightActor.cpp
        PointLightActor.h)

target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            Actor.h
            CameraActor.h
            DirectionalLightActor.h
            Entity.h
            PointLightActor.h)
