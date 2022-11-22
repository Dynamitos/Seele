target_sources(Engine
    PRIVATE
        EngineTypes.h
        MinimalEngine.h
        MinimalEngine.cpp)

target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            EngineTypes.h
            MinimalEngine.h)

add_subdirectory(Actor/)
add_subdirectory(Asset/)
add_subdirectory(Component/)
add_subdirectory(Containers/)
add_subdirectory(Graphics/)
add_subdirectory(Material/)
add_subdirectory(Math/)
add_subdirectory(Scene/)
add_subdirectory(System/)
add_subdirectory(UI/)
add_subdirectory(Window/)
