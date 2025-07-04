#include "Texture.h"


using namespace Seele;
using namespace Seele::Gfx;

TextureView::TextureView() {}

TextureView::~TextureView() {}

Texture::Texture(QueueFamilyMapping mapping) : QueueOwnedResource(mapping) {}

Texture::~Texture() {}

Texture2D::Texture2D(QueueFamilyMapping mapping) : Texture(mapping) {}

Texture2D::~Texture2D() {}

Texture3D::Texture3D(QueueFamilyMapping mapping) : Texture(mapping) {}

Texture3D::~Texture3D() {}

TextureCube::TextureCube(QueueFamilyMapping mapping) : Texture(mapping) {}

TextureCube::~TextureCube() {}
