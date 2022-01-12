target_sources(SeeleEngine
	PRIVATE
        Util.h
        Scene.cpp
        Scene.h)

add_subdirectory(Actor/)
add_subdirectory(Components/)