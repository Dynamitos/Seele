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
    render.text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vivamus quis magna ex. Morbi ullamcorper fringilla risus eget vehicula. Praesent vel quam vel ante molestie gravida vitae ac enim. Donec vitae eleifend orci. Phasellus at sodales lorem, ac eleifend turpis. Vivamus vitae condimentum lacus, a bibendum neque. Ut et est ut felis varius vehicula. Etiam lorem magna, dapibus vitae felis in, vulputate suscipit neque. Aenean facilisis ac risus et scelerisque. Ut tincidunt eros quis posuere iaculis. Curabitur justo lacus, molestie id varius vel, sodales efficitur diam. Integer orci velit, condimentum sit amet turpis sit amet, congue blandit nisl. Donec pretium ligula id mauris pretium commodo. Mauris quis lectus mi. In blandit, dolor non accumsan venenatis, ipsum erat congue neque, quis elementum orci nunc vel justo. ";
    //render.text = "Seele Engine";
    render.position = Vector2(0.f, 300.f);
    render.scale = 0.1f;
    render.textColor = Vector4(1, 0, 0, 1);
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
