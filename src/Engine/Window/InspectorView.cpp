#include "InspectorView.h"
#include "Graphics/Graphics.h"
#include "Scene/Actor/Actor.h"
#include "Window.h"
#include "Asset/AssetRegistry.h"
#include "UI/System.h"

using namespace Seele;

InspectorView::InspectorView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo &createInfo) 
    : View(graphics, window, createInfo, "InspectorView")
    , uiPass(UIPass(graphics, viewport, new Gfx::SwapchainAttachment(window->getGfxHandle())))
    , textPass(TextPass(graphics, viewport, new Gfx::SwapchainAttachment(window->getGfxHandle())))
    , uiSystem(new UI::System())
{
    AssetRegistry::importFile("./fonts/Calibri.ttf");
    PRenderGraphResources resources = new RenderGraphResources();
    uiPass.setResources(resources);
    textPass.setResources(resources);
    uiPass.publishOutputs();
    textPass.publishOutputs();
    uiPass.createRenderPass();
    textPass.createRenderPass();
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
    uiPass.updateViewFrame(uiSystem->getUIPassData());
    textPass.updateViewFrame(uiSystem->getTextPassData());
}

void InspectorView::render() 
{
    uiPass.beginFrame();
    textPass.beginFrame();
    uiPass.render();
    textPass.render();
    uiPass.endFrame();
    textPass.endFrame();
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
