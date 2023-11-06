target_sources(Engine
    PRIVATE
        GameInterface.h
        GameInterface.cpp)


target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            GameInterface.h)