target_sources(SeeleEngine
    PRIVATE
        Graphics.h
        Graphics.cpp
        RenderPath.h
        RenderPath.cpp
        View.h
        View.cpp
        WindowManager.h
        WindowManager.cpp
        Window.h
        Window.cpp
        GraphicsResources.h)

add_subdirectory(Vulkan/)