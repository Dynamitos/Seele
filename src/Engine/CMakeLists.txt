target_sources(Engine
    PRIVATE
        Concepts.h
        EngineTypes.h
        Game.h
        MinimalEngine.h
        MinimalEngine.cpp)

target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            Concepts.h
            Game.h
            EngineTypes.h
            MinimalEngine.h)

add_subdirectory(Actor/)
add_subdirectory(Asset/)
add_subdirectory(Component/)
add_subdirectory(Containers/)
add_subdirectory(Graphics/)
add_subdirectory(Material/)
add_subdirectory(Math/)
add_subdirectory(Physics/)
add_subdirectory(Platform/)
add_subdirectory(Serialization/)
add_subdirectory(Scene/)
add_subdirectory(System/)
add_subdirectory(UI/)
add_subdirectory(Window/)
