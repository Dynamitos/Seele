#include "InspectorView.h"
#include "Graphics/Graphics.h"
#include "Scene/Actor/Actor.h"
#include "Window.h"

using namespace Seele;

InspectorView::InspectorView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo &createInfo) 
    : View(graphics, window, createInfo, "InspectorView")
    , uiPass(UIPass(graphics, viewport, new Gfx::SwapchainAttachment(window->getGfxHandle())))
{
    PRenderGraphResources resources = new RenderGraphResources();
    uiPass.setResources(resources);
    uiPass.publishOutputs();
    uiPass.createRenderPass();
}

InspectorView::~InspectorView() 
{
}

Job InspectorView::beginUpdate() 
{
    co_return;
}

Job InspectorView::update() 
{
    co_return; 
}

void InspectorView::commitUpdate() 
{
    
}

void InspectorView::prepareRender() 
{
    
}

MainJob InspectorView::render() 
{
    co_await uiPass.beginFrame();
    co_await uiPass.render();
    co_await uiPass.endFrame();
    renderFinishedEvent.raise();
    co_return;
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
