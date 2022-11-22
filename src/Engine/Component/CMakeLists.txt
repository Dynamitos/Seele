target_sources(Engine
    PRIVATE
        Camera.h
        Camera.cpp
        Component.h
        StaticMesh.h
		Transform.h
		Transform.cpp)

target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            Camera.h
            Component.h
            StaticMesh.h
            Transform.h)