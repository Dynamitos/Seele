target_sources(Engine
    PRIVATE
        Executor.h
        Executor.cpp
        SystemBase.h)

target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            Executor.h
            SystemBase.h)