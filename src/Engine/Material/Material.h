#pragma once
#include "MinimalEngine.h"
#include "Asset/FileAsset.h"
#include "Graphics/GraphicsResources.h"
#include <nlohmann/json_fwd.hpp>

namespace Seele
{
struct FloatExpression
{
    float data;
    std::string name;
    uint32 location;
};
struct VectorExpression
{
    Vector data;
    std::string name;
    uint32 location;
};
struct TextureExpression
{
    Gfx::PTexture texture;
    std::string name;
    uint32 location;
};
DECLARE_NAME_REF(Gfx, GraphicsPipeline);
DECLARE_NAME_REF(Gfx, PipelineLayout);
class Material : public FileAsset
{
public:
    Material();
    Material(const std::string &directory, const std::string &name);
    Material(const std::string &fullPath);
    ~Material();
    void compile();
    Gfx::PGraphicsPipeline getPipeline();

private:
    Gfx::PGraphicsPipeline pipeline;
    Gfx::PPipelineLayout pipelineLayout;
};
DEFINE_REF(Material);
} // namespace Seele