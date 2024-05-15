#include "Texture.h"
#include "Texture.h"

using namespace Seele;
using namespace Seele::Gfx;


Texture::Texture(QueueFamilyMapping mapping, Gfx::QueueType startQueueType)
	: QueueOwnedResource(mapping, startQueueType)
{
}

Texture::~Texture()
{
}

Texture2D::Texture2D(QueueFamilyMapping mapping, Gfx::QueueType startQueueType)
	: Texture(mapping, startQueueType)
{
}

Texture2D::~Texture2D()
{
}

Texture3D::Texture3D(QueueFamilyMapping mapping, Gfx::QueueType startQueueType)
	: Texture(mapping, startQueueType)
{
}

Texture3D::~Texture3D()
{
}

TextureCube::TextureCube(QueueFamilyMapping mapping, Gfx::QueueType startQueueType)
	: Texture(mapping, startQueueType)
{
}

TextureCube::~TextureCube()
{
}
