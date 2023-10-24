target_sources(Engine
    PRIVATE
        DebugVertex.h
        GraphicsResources.h
        GraphicsResources.cpp
        GraphicsInitializer.h
        GraphicsEnums.h
        GraphicsEnums.cpp
        Graphics.h
        Graphics.cpp
        Mesh.h
        Mesh.cpp
        ShaderCompiler.h
        ShaderCompiler.cpp
        StaticMeshVertexData.h
        StaticMeshVertexData.cpp
        VertexData.h
        VertexData.cpp)

target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            DebugVertex.h
            GraphicsResources.h
            GraphicsInitializer.h
            GraphicsEnums.h
            Graphics.h
            Mesh.h
            ShaderCompiler.h
            StaticMeshVertexData.h
            VertexData.h)

add_subdirectory(RenderPass/)
add_subdirectory(Vulkan/)