#include "System.h"
#include "Elements/Panel.h"
#include "Graphics/Graphics.h"
#include "Asset/AssetRegistry.h"
#include "Asset/TextureAsset.h"

using namespace Seele;
using namespace Seele::UI;

System::System() 
{
    
}

System::~System() 
{
    
}

void System::update()
{
    
}

UIPassData System::getUIPassData()
{
    UIPassData uiPassData;
    RenderElementStyle& style = uiPassData.renderElements.add();
    style.position = Math::Vector(0, 0, -0.1);
    style.dimensions = Math::Vector2(0.4, 0.4);
    style.backgroundColor = Math::Vector(0.2, 0.3, 0.1);
    style.backgroundImageIndex = 0;
    uiPassData.usedTextures.add(AssetRegistry::findTexture("")->getTexture());
    return uiPassData;
}

TextPassData System::getTextPassData()
{
    TextPassData textPassData;
    TextRender& render = textPassData.texts.add();
    render.font = AssetRegistry::findFont("Calibri");
    render.text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vivamus quis magna ex. Morbi ullamcorper fringilla risus eget vehicula. Praesent vel quam vel ante molestie gravida vitae ac enim. Donec vitae eleifend orci. Phasellus at sodales lorem, ac eleifend turpis. Vivamus vitae condimentum lacus, a bibendum neque. Ut et est ut felis varius vehicula. Etiam lorem magna, dapibus vitae felis in, vulputate suscipit neque. Aenean facilisis ac risus et scelerisque. Ut tincidunt eros quis posuere iaculis. Curabitur justo lacus, molestie id varius vel, sodales efficitur diam. Integer orci velit, condimentum sit amet turpis sit amet, congue blandit nisl. Donec pretium ligula id mauris pretium commodo. Mauris quis lectus mi. In blandit, dolor non accumsan venenatis, ipsum erat congue neque, quis elementum orci nunc vel justo. ";
    //render.text = "Seele Engine";
    render.position = Math::Vector2(0.f, 300.f);
    render.scale = 0.1f;
    render.textColor = Math::Vector4(1, 0, 0, 1);
    return textPassData;
}
