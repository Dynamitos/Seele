#include "InspectorView.h"
#include "Graphics/Graphics.h"
#include "Scene/Actor/Actor.h"
#include "Window.h"
#include "Asset/AssetRegistry.h"

using namespace Seele;

InspectorView::InspectorView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo &createInfo) 
    : View(graphics, window, createInfo, "InspectorView")
    , uiPass(UIPass(graphics, viewport, new Gfx::SwapchainAttachment(window->getGfxHandle())))
    , textPass(TextPass(graphics, viewport, new Gfx::SwapchainAttachment(window->getGfxHandle())))
{
    AssetRegistry::importFile("./fonts/Calibri.ttf");
    PRenderGraphResources resources = new RenderGraphResources();
    uiPass.setResources(resources);
    textPass.setResources(resources);
    uiPass.publishOutputs();
    textPass.publishOutputs();
    uiPass.createRenderPass();
    textPass.createRenderPass();
    TextRender& render = textPassData.texts.add();
    render.font = AssetRegistry::findFont("Calibri");
    render.text = "Seele Engine";
    render.position = Vector2(0.5f, 0.5f);
    render.scale = 1.0f;
    render.textColor = Vector4(1, 1, 1, 1);
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
    textPass.updateViewFrame(textPassData);
}

MainJob InspectorView::render() 
{
    return uiPass.beginFrame()
        .then(textPass.beginFrame())
        .then(uiPass.render())
        .then(textPass.render())
        .then(uiPass.endFrame())
        .then(textPass.endFrame());
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
