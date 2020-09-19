#pragma once
#include "GraphicsResources.h"

namespace Seele
{
namespace Gfx
{
class MaterialShaderMap;//TODO implement
class MaterialRenderContext;
struct UniformExpressionCache
{
    Gfx::PUniformBuffer uniformBuffer;
    uint8 bUpToDate;

    const MaterialShaderMap* cachedUniformExpressionShaderMap;
};
class Material;
class RenderMaterial
{
public:
    mutable UniformExpressionCache uniformExpressionCache;
    RenderMaterial();
    virtual ~RenderMaterial();

    void evaluateUniformExpressions(UniformExpressionCache& outUniforExpressionCache, const MaterialRenderContext& context);

    void cacheUniformExpressions(bool bRecreatueUniformBuffer);

    void invalidateUniformExpressionCache(bool bRecreateUniformBuffer);

    void updateUniformExpressionCacheIfNeeded() const;

    const Material* getMaterial() const
    {
        const RenderMaterial* unused = nullptr;
        return &getMaterialWithFallback(unused);
    }

    virtual const Material& getMaterialWithFallback(const RenderMaterial*& outFallbackRenderMaterial) const = 0;

    virtual Material* getMaterialNoFallback() const { return nullptr; }
};
} // namespace Gfx
} // namespace Seele
