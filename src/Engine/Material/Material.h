#pragma once
#include "MinimalEngine.h"
#include "MaterialAsset.h"

namespace Seele
{
class VertexInputType;
class Material : public MaterialAsset
{
public:
    Material();
    Material(const std::string &directory, const std::string &name);
    Material(const std::filesystem::path& fullPath);
    ~Material();
    virtual void save() override;
    virtual void load() override;
    virtual PMaterial getRenderMaterial() { return this; }
    const std::string& getName() {return materialName;}

    const Gfx::ShaderCollection* getShaders(Gfx::RenderPassType renderPass, VertexInputType* vertexInput) const;
    Gfx::ShaderCollection& createShaders(Gfx::PGraphics graphics, Gfx::RenderPassType renderPass, VertexInputType* vertexInput);
    void compile();
private:
    static Gfx::ShaderMap shaderMap;
    static std::mutex shaderMapLock;

    std::string materialName;
    friend class MaterialLoader;
};
DEFINE_REF(Material);
} // namespace Seele