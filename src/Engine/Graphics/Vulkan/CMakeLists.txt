target_sources(Engine
	PRIVATE
		Allocator.h
		Allocator.cpp
		Buffer.h
		Buffer.cpp
		Command.h
		Command.cpp
		Debug.h
		Debug.cpp
		Descriptor.h
		Descriptor.cpp
		Enums.h
		Enums.cpp
		Framebuffer.h
		Framebuffer.cpp
		Graphics.h
		Graphics.cpp
		Pipeline.h
		Pipeline.cpp
		PipelineCache.h
		PipelineCache.cpp
		Query.h
		Query.cpp
		Queue.h
		Queue.cpp
        RayTracing.h
        RayTracing.cpp
		RenderPass.h
		RenderPass.cpp
		Resources.h
		Resources.cpp
		Shader.h
		Shader.cpp
		Texture.h
		Texture.cpp
		Window.h
		Window.cpp)

target_sources(Engine
	PUBLIC FILE_SET HEADERS
		FILES
			Allocator.h
			Buffer.h
			Command.h
			Debug.h
			Descriptor.h
			Enums.h
			Framebuffer.h
			Graphics.h
			Pipeline.h
			PipelineCache.h
			Query.h
			Queue.h
            RayTracing.h
			RenderPass.h
			Resources.h
			Shader.h
			Texture.h
			Window.h)
	