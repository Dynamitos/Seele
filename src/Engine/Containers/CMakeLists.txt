target_sources(Engine
	PRIVATE
		Array.h
		List.h
		Map.h
		MemoryResource.h
		MemoryResource.cpp
		Pair.h
		Set.h
		Tree.h)

target_sources(Engine
	PUBLIC FILE_SET HEADERS
		FILES	
			Array.h
			List.h
			Map.h
			MemoryResource.h
			Pair.h
			Set.h
			Tree.h)