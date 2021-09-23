#include "InspectorView.h"
#include "Window.h"

using namespace Seele;

InspectorView::InspectorView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo &createInfo) 
    : View(graphics, window, createInfo)
{
    renderGraph = new RenderGraph();
    Gfx::PRenderTargetAttachment attachment = new Gfx::SwapchainAttachment(window->getGfxHandle());
    uiPass = new UIPass(renderGraph, graphics, viewport, attachment);
    renderGraph->addRenderPass(uiPass);
    renderGraph->setup();
}

InspectorView::~InspectorView() 
{
}

void InspectorView::beginFrame() 
{
    uiPass->beginFrame();
}

void InspectorView::render() 
{
    uiPass->render();
}

void InspectorView::endFrame() 
{
    uiPass->endFrame();
}

void InspectorView::keyCallback(KeyCode code, InputAction action, KeyModifier modifier) 
{
    
}

void InspectorView::mouseMoveCallback(double xPos, double yPos) 
{
    
}

void InspectorView::mouseButtonCallback(MouseButton button, InputAction action, KeyModifier modifier) 
{
    
}

void InspectorView::scrollCallback(double xOffset, double yOffset) 
{
    
}

void InspectorView::fileCallback(int count, const char** paths) 
{
    
}
