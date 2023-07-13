target_sources(Engine
    PRIVATE
        ArchiveBuffer.h
        ArchiveBuffer.cpp
        Serialization.h)

target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            ArchiveBuffer.h
            Serialization.h)