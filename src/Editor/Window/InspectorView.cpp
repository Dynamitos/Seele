#include "InspectorView.h"
#include "Graphics/Graphics.h"
#include "Actor/Actor.h"
#include "Window/Window.h"
#include "Asset/AssetRegistry.h"
#include "Asset/FontLoader.h"
#include "UI/System.h"

using namespace Seele;

InspectorView::InspectorView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo &createInfo) 
    : View(graphics, window, createInfo, "InspectorView")
    , renderGraph(RenderGraphBuilder::build(
        UIPass(graphics),
        TextPass(graphics)
    ))
    , uiSystem(new UI::System())
{
    AssetRegistry::importFont(FontImportArgs{
        .filePath = "./fonts/Calibri.ttf",
    });
    renderGraph.updateViewport(viewport);
    uiSystem->updateViewport(viewport);
}

InspectorView::~InspectorView() 
{
}

void InspectorView::beginUpdate() 
{
    //co_return;
}

void InspectorView::update() 
{
    //co_return; 
}

void InspectorView::commitUpdate() 
{
}

void InspectorView::prepareRender() 
{
    renderGraph.updatePassData(
        uiSystem->getUIPassData(),
        uiSystem->getTextPassData()
    );
}

void InspectorView::render() 
{
    renderGraph.render(uiSystem->getVirtualCamera());
}

void InspectorView::keyCallback(KeyCode, InputAction, KeyModifier) 
{
    
}

void InspectorView::mouseMoveCallback(double, double) 
{
    
}

void InspectorView::mouseButtonCallback(MouseButton, InputAction, KeyModifier) 
{
    
}

void InspectorView::scrollCallback(double, double) 
{
    
}

void InspectorView::fileCallback(int, const char**) 
{
    
}
