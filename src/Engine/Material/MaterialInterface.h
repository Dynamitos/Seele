#pragma once
#include "ShaderExpression.h"
#include "Graphics/GraphicsResources.h"

namespace Seele
{
DECLARE_NAME_REF(Gfx, DescriptorLayout)
DECLARE_NAME_REF(Gfx, UniformBuffer)
class MaterialInterface
{
public:
    MaterialInterface() {}
    MaterialInterface(Gfx::PGraphics graphics, Array<PShaderParameter> parameter, uint32 uniformDataSize, uint32 uniformBinding);
    virtual ~MaterialInterface();
    PShaderParameter getParameter(const std::string& name);
    virtual Gfx::PDescriptorSet createDescriptorSet() = 0;
    virtual Gfx::PDescriptorLayout getDescriptorLayout() const = 0;

    virtual void save(ArchiveBuffer& buffer) const;
    virtual void load(ArchiveBuffer& buffer);

    // The name of the generated material shader, opposed to the name of the .asset file
    virtual const std::string& getName() = 0;
    virtual const Gfx::ShaderCollection* getShaders(Gfx::RenderPassType renderPass, VertexInputType* vertexInput) const = 0;
    virtual Gfx::ShaderCollection& createShaders(Gfx::PGraphics graphics, Gfx::RenderPassType renderPass, VertexInputType* vertexInput) = 0;
protected:
    Gfx::PGraphics graphics;
    std::string brdfName;
    Array<PShaderParameter> parameters;
    Gfx::PUniformBuffer uniformBuffer;
    uint32 uniformDataSize;
    Array<uint8> uniformData;
    int32 uniformBinding;
};
DEFINE_REF(MaterialInterface)
} // namespace Seele