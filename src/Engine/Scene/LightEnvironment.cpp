#include "LightEnvironment.h"
#include "Graphics/Graphics.h"

using namespace Seele;

LightEnvironment::LightEnvironment(Gfx::PGraphics graphics)
    : graphics(graphics)
{
    layout = graphics->createDescriptorLayout("LightEnvironment");
    layout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    layout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    layout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    layout->addDescriptorBinding(3, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
}

LightEnvironment::~LightEnvironment()
{
}

void LightEnvironment::reset()
{

}

void LightEnvironment::addDirectionalLight(Component::DirectionalLight dirLight)
{
}

void LightEnvironment::addPointLight(Component::PointLight pointLight)
{
}

void LightEnvironment::commit()
{
}
