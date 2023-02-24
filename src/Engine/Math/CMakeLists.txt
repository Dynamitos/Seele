target_sources(Engine
	PRIVATE
		Math.h
		Matrix.h
		Transform.h
		Transform.cpp
		Vector.h
		Vector.cpp)

target_sources(Engine
	PUBLIC FILE_SET HEADERS
		FILES
			Math.h
			Matrix.h
			Transform.h
			Vector.h)
