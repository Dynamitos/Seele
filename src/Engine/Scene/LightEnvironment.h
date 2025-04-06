#pragma once
#include "Component/DirectionalLight.h"
#include "Component/PointLight.h"
#include "Graphics/Buffer.h"
#include "Graphics/Descriptor.h"

namespace Seele {
DECLARE_REF(EnvironmentMapAsset)
class LightEnvironment {
  public:
    LightEnvironment(Gfx::PGraphics graphics);
    ~LightEnvironment();
    void reset();
    void addDirectionalLight(Component::DirectionalLight dirLight);
    void addPointLight(Component::PointLight pointLight);
    void commit();
    const Gfx::PDescriptorLayout getDescriptorLayout() const;
    Gfx::PDescriptorSet getDescriptorSet();
    PEnvironmentMapAsset getEnvironmentMap() { return environment; }

  private:
    Gfx::OShaderBuffer directionalLights;
    Gfx::OShaderBuffer pointLights;
    Array<Component::DirectionalLight> dirs;
    Array<Component::PointLight> points;
    PEnvironmentMapAsset environment;
    Gfx::OSampler environmentSampler;
    Gfx::ODescriptorLayout layout;
    Gfx::PDescriptorSet set;
    Gfx::PGraphics graphics;
};
DEFINE_REF(LightEnvironment)
} // namespace Seele