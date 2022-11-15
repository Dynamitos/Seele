target_sources(Engine
	PUBLIC
        Util.h
        Scene.cpp
        Scene.h)

add_subdirectory(Actor/)
add_subdirectory(Component/)
add_subdirectory(System/)