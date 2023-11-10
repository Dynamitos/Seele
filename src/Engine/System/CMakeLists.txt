target_sources(Engine
    PRIVATE
        CameraUpdater.h
        CameraUpdater.cpp
        ComponentSystem.h
        Executor.h
        Executor.cpp
        LightGather.h
        LightGather.cpp
        MeshUpdater.h
        MeshUpdater.cpp
        SystemBase.h
        SystemGraph.h
        SystemGraph.cpp)

target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            CameraUpdater.h
            ComponentSystem.h
            Executor.h
            LightGather.h
            MeshUpdater.h
            SystemBase.h
            SystemGraph.h)