target_sources(Engine
    PRIVATE
        Material.h
        Material.cpp
        MaterialInstance.h
        MaterialInstance.cpp
        MaterialInterface.h
        MaterialInterface.cpp
        ShaderExpression.h
        ShaderExpression.cpp)

target_sources(Engine
    PUBLIC FILE_SET HEADERS
        FILES
            Material.h
            MaterialInstance.h
            MaterialInterface.h
            ShaderExpression.h)

