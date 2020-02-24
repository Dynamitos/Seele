target_sources(Seele_unit_tests
	PRIVATE
		EngineTest.h
		EngineTest.cpp)
target_include_directories(Seele_unit_tests PUBLIC ./)
add_subdirectory(Containers/)