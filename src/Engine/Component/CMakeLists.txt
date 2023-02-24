target_sources(Engine
    PRIVATE
        AABB.h
        BoxCollider.h
        Camera.h
        Camera.cpp
        Collider.h
        Collider.cpp
        Component.h
        KeyboardInput.h
        MeshCollider.h
        RigidBody.h
        ShapeBase.h
        ShapeBase.cpp
        Skybox.h
        SphereCollider.h
        StaticMesh.h
		Transform.h
		Transform.cpp)


target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            AABB.h
            BoxCollider.h
            Camera.h
            Collider.h
            Component.h
            KeyboardInput.h
            MeshCollider.h
            RigidBody.h
            ShapeBase.h
            Skybox.h
            SphereCollider.h
            StaticMesh.h
            Transform.h)