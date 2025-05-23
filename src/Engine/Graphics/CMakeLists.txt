#add_subdirectory(CBT/)

target_sources(Engine
    PRIVATE
        Buffer.h
        Buffer.cpp
        Command.h
        Command.cpp
        DebugVertex.h
        Descriptor.h
        Descriptor.cpp
        Enums.h
        Enums.cpp
        Graphics.h
        Graphics.cpp
        Initializer.h
        Mesh.h
        Mesh.cpp
        MeshData.h
        Meshlet.h
        Meshlet.cpp
        Pipeline.h
        Pipeline.cpp
        Query.h
        Query.cpp
        RayTracing.h
        RayTracing.cpp
        RenderTarget.h
        RenderTarget.cpp
        Resources.h
        Resources.cpp
        Shader.h
        Shader.cpp
        slang-compile.h
        slang-compile.cpp
        StaticMeshVertexData.h
        StaticMeshVertexData.cpp
        Texture.h
        Texture.cpp
        VertexData.h
        VertexData.cpp
        Window.h
        Window.cpp)

target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            Buffer.h
            Command.h
            DebugVertex.h
            Descriptor.h
            Enums.h
            Graphics.h
            Initializer.h
            Mesh.h
            Meshlet.h
            MeshData.h
            Pipeline.h
            Query.h
            RayTracing.h
            RenderTarget.h
            Resources.h
            Shader.h
            StaticMeshVertexData.h
            Texture.h
            VertexData.h
            Window.h)

add_subdirectory(RenderPass/)
if(APPLE)
    add_subdirectory(Metal/)
else()
    add_subdirectory(Vulkan/)
endif()