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
    Material(const std::filesystem::path& fullPath);
    ~Material();
    virtual void save() override;
    virtual void load() override;
    virtual std::string getMaterialName() const { return materialName; }
    inline std::string getCode() const { return materialCode; }

    const Gfx::ShaderCollection* getShaders(Gfx::RenderPassType renderPass, PVertexShaderInput vertexInput) const;
    Gfx::ShaderCollection& createShaders(Gfx::PGraphics graphics, Gfx::RenderPassType renderPass, PVertexShaderInput vertexInput);
private:
    static Gfx::ShaderMap shaderMap;
    static std::mutex shaderMapLock;

    void compile();
    std::string materialName;
    std::string materialCode;
    friend class MaterialLoader;
};
DEFINE_REF(Material);
} // namespace Seele