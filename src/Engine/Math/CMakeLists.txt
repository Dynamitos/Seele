target_sources(Engine
	PRIVATE
		AABB.h
		Math.h
		Matrix.h
		Transform.h
		Transform.cpp
		Vector.h
		Vector.cpp)

target_sources(Engine
	PUBLIC FILE_SET HEADERS
		FILES
			AABB.h
			Math.h
			Matrix.h
			Transform.h
			Vector.h)
