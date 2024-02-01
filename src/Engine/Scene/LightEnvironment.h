#pragma once
#include "Graphics/Descriptor.h"
#include "Graphics/Buffer.h"
#include "Component/DirectionalLight.h"
#include "Component/PointLight.h"

namespace Seele
{
class LightEnvironment
{
public:
    LightEnvironment(Gfx::PGraphics graphics);
    ~LightEnvironment();
    void reset();
    void addDirectionalLight(Component::DirectionalLight dirLight);
    void addPointLight(Component::PointLight pointLight);
    void commit();
    const Gfx::PDescriptorLayout getDescriptorLayout() const;
    Gfx::PDescriptorSet getDescriptorSet();
private:
    #define INIT_DIRECTIONAL_LIGHTS 4
    #define INIT_POINT_LIGHTS 256
    struct LightEnv
    {
        uint32 numDirectionalLights;
        uint32 numPointLights;
    } lightEnv;
    Gfx::OShaderBuffer directionalLights;
    Gfx::OUniformBuffer lightEnvBuffer;
    Gfx::OShaderBuffer pointLights;
    Array<Component::DirectionalLight> dirs;
    Array<Component::PointLight> points;
    Gfx::ODescriptorLayout layout;
    Gfx::PDescriptorSet set;
    Gfx::PGraphics graphics;
};
DEFINE_REF(LightEnvironment)
}