target_sources(Engine
    PRIVATE
        Asset.h
        Asset.cpp
        AssetRegistry.h
        AssetRegistry.cpp
        FontAsset.h
        FontAsset.cpp
        MaterialAsset.h
        MaterialAsset.cpp
        MaterialInstanceAsset.h
        MaterialInstanceAsset.cpp
        MeshAsset.h
        MeshAsset.cpp
        TextureAsset.h
        TextureAsset.cpp)

target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            Asset.h
            AssetRegistry.h
            FontAsset.h
            MaterialAsset.h
            MaterialInstanceAsset.h
            MeshAsset.h
            TextureAsset.h)