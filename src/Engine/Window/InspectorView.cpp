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
    render.position = Vector2(200.f, 300.f);
    render.scale = 1.0f;
    render.textColor = Vector4(1, 1, 1, 1);
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
    textPass.updateViewFrame(textPassData);
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
