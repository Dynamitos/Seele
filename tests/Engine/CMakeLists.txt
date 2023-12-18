target_sources(SeeleUnitTests
	PRIVATE
		EngineTest.h
		ThreadPool.cpp)

target_include_directories(SeeleUnitTests PUBLIC ./)
add_subdirectory(Containers/)
add_subdirectory(Graphics/)
add_subdirectory(Math/)
add_subdirectory(Serialization/)