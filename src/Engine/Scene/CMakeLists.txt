target_sources(SeeleEngine
	PRIVATE
        Scene.cpp
        Scene.h
        SceneUpdater.h
        SceneUpdater.cpp)

add_subdirectory(Actor/)
add_subdirectory(Components/)