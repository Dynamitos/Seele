#pragma once
#include "MinimalEngine.h"
#include "MaterialAsset.h"

namespace Seele
{

class Material : public MaterialAsset
{
public:
    Material();
    Material(const std::string &directory, const std::string &name);
    Material(const std::string &fullPath);
    ~Material();
    inline std::string getMaterialName() const {return materialName;}
private:
    void compile();
    std::string materialName;
    friend class MaterialLoader;
};
DEFINE_REF(Material);
} // namespace Seele