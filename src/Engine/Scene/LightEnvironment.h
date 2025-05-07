#pragma once
#include "Component/DirectionalLight.h"
#include "Component/PointLight.h"
#include "Component/Transform.h"
#include "Graphics/Buffer.h"
#include "Graphics/Descriptor.h"

namespace Seele {
DECLARE_REF(EnvironmentMapAsset)
class LightEnvironment {
  public:
    struct ShaderDirectionalLight {
        Vector4 color;
        Vector4 direction;
        Matrix4 lightSpaceMatrix;
    };
    struct ShaderPointLight {
        Vector4 position_WS;
        Vector4 colorRange;
    };
    LightEnvironment(Gfx::PGraphics graphics);
    ~LightEnvironment();
    void reset();
    void addDirectionalLight(const Component::DirectionalLight& dirLight, const Component::Transform& transform);
    void addPointLight(const Component::PointLight& pointLight, const Component::Transform& transform);
    void commit();
    const Gfx::PDescriptorLayout getDescriptorLayout() const;
    const ShaderDirectionalLight& getDirectionalLight(uint32 lightIndex) const { return dirs[lightIndex]; }
    const Component::Transform& getDirectionalTransform(uint32 lightIndex) const { return directionalTransforms[lightIndex]; }
    Gfx::PDescriptorSet getDescriptorSet();
    PEnvironmentMapAsset getEnvironmentMap() { return environment; }
    uint64 getNumDirectionalLights() const { return dirs.size(); }
    const Array<Gfx::OTexture2D>& getShadowMaps() const { return shadowMaps; }

  private:
    Gfx::OShaderBuffer directionalLights;
    Array<Gfx::OTexture2D> shadowMaps;
    Array<Gfx::OSampler> shadowSamplers;
    Gfx::OShaderBuffer pointLights;
    Array<ShaderDirectionalLight> dirs;
    Array<Component::Transform> directionalTransforms;
    Array<ShaderPointLight> points;
    PEnvironmentMapAsset environment;
    Gfx::OSampler environmentSampler;
    Gfx::ODescriptorLayout layout;
    Gfx::PDescriptorSet set;
    Gfx::PGraphics graphics;
};
DEFINE_REF(LightEnvironment)
} // namespace Seele