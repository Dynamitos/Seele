target_sources(SeeleEngine
    PRIVATE
        MinimalEngine.h
        MinimalEngine.cpp
        main.cpp)

add_subdirectory(Graphics/)
add_subdirectory(Math/)
add_subdirectory(Containers/)