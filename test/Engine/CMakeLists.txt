target_sources(Seele_unit_tests
	PRIVATE
		../../src/Engine/MinimalEngine.cpp
		EngineTest.h
		EngineTest.cpp)
target_include_directories(Seele_unit_tests PUBLIC ./)
add_subdirectory(Containers/)
add_subdirectory(Graphics/)
add_subdirectory(Math/)