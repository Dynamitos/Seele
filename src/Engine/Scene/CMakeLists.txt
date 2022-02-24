target_sources(Engine
	PRIVATE
        Util.h
        Scene.cpp
        Scene.h)

add_subdirectory(Actor/)
add_subdirectory(Components/)