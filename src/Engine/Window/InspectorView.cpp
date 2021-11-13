#include "InspectorView.h"
#include "Window.h"

using namespace Seele;

InspectorView::InspectorView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo &createInfo) 
    : View(graphics, window, createInfo, "InspectorView")
    , uiPass(UIPass(graphics, viewport, new Gfx::SwapchainAttachment(window->getGfxHandle())))
{
}

InspectorView::~InspectorView() 
{
}

void InspectorView::beginUpdate() 
{
}

void InspectorView::update() 
{
    
}

void InspectorView::commitUpdate() 
{
    
}

void InspectorView::prepareRender() 
{
    
}

void InspectorView::render() 
{
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
