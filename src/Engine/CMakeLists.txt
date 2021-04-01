target_sources(SeeleEngine
    PRIVATE
        MinimalEngine.h
        MinimalEngine.cpp
        main.cpp)

add_subdirectory(Asset/)
add_subdirectory(Containers/)
#add_subdirectory(Fibers/)
add_subdirectory(Graphics/)
add_subdirectory(Material/)
add_subdirectory(Math/)
add_subdirectory(Scene/)
add_subdirectory(UI/)
add_subdirectory(Window/)
