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
        MeshBatch.h
        MeshBatch.cpp
        ShaderCompiler.h
        ShaderCompiler.cpp
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
            MeshBatch.h
            ShaderCompiler.h)

add_subdirectory(RenderPass/)
add_subdirectory(Vulkan/)