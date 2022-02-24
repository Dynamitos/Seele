target_sources(Engine
    PRIVATE
        EngineTypes.h
        MinimalEngine.h
        MinimalEngine.cpp
        ThreadPool.h
        ThreadPool.cpp
        main.cpp)

add_subdirectory(Asset/)
add_subdirectory(Containers/)
add_subdirectory(Graphics/)
add_subdirectory(Material/)
add_subdirectory(Math/)
add_subdirectory(Scene/)
add_subdirectory(UI/)
add_subdirectory(Window/)
