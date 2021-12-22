target_sources(Seele_unit_tests
	PRIVATE
		../../src/Engine/MinimalEngine.cpp
		../../src/Engine/ThreadPool.cpp
		EngineTest.h
		EngineTest.cpp
		ThreadPool.cpp)
target_include_directories(Seele_unit_tests PUBLIC ./)
add_subdirectory(Containers/)
add_subdirectory(Graphics/)
add_subdirectory(Math/)