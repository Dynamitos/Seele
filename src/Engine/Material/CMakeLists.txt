target_sources(Engine
    PRIVATE
        BRDF.h
        BRDF.cpp
        MaterialAsset.h
        MaterialAsset.cpp
        ShaderExpression.h
        ShaderExpression.cpp)
        
target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            BRDF.h
            MaterialAsset.h
            ShaderExpression.h)

