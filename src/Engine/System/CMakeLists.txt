target_sources(Engine
    PRIVATE
        ComponentSystem.h
        Executor.h
        Executor.cpp
        MeshUpdater.h
        MeshUpdater.cpp
        SystemBase.h
        SystemGraph.h
        SystemGraph.cpp)

target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            ComponentSystem.h
            MeshUpdater.h
            Executor.h
            SystemBase.h
            SystemGraph.h)