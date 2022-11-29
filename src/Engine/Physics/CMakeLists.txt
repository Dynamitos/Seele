target_sources(Engine
    PRIVATE
        BVH.h
        BVH.cpp
        CollisionSystem.h
        CollisionSystem.cpp
        PhysicsSystem.h
        PhysicsSystem.cpp)

target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            BVH.h
            CollisionSystem.h
            PhysicsSystem.h)