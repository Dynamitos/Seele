#include "PlayView.h"
#include "Component/Mesh.h"
#include "Window/Window.h"

using namespace Seele;
using namespace Seele::Editor;

PlayView::PlayView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo& createInfo, std::filesystem::path dllPath)
    : GameView(graphics, window, createInfo, dllPath) {}

PlayView::~PlayView() {}

void PlayView::beginUpdate() { GameView::beginUpdate(); }

void PlayView::update() { GameView::update(); }

void PlayView::commitUpdate() { GameView::commitUpdate(); }

void PlayView::prepareRender() { GameView::prepareRender(); }

void PlayView::render() { GameView::render(); }

void PlayView::keyCallback(KeyCode code, InputAction action, KeyModifier modifier) {
    GameView::keyCallback(code, action, modifier);
    if (code == KeyCode::KEY_P && action == InputAction::RELEASE) {
        getGlobals().usePositionOnly = !getGlobals().usePositionOnly;
        std::cout << "Use Pos only " << getGlobals().usePositionOnly << std::endl;
    }
    if (code == KeyCode::KEY_O && action == InputAction::RELEASE) {
        getGlobals().useDepthCulling = !getGlobals().useDepthCulling;
        std::cout << "Use Depth Culling " << getGlobals().useDepthCulling << std::endl;
    }
    if (code == KeyCode::KEY_L && action == InputAction::RELEASE) {
        getGlobals().useLightCulling = !getGlobals().useLightCulling;
        std::cout << "Use Light Culling " << getGlobals().useLightCulling << std::endl;
    }
    if (code == KeyCode::KEY_G && action == InputAction::RELEASE) {
        Component::Camera cam;
        Component::Transform tra;
        scene->view<Component::Camera, Component::Transform>([&cam, &tra](Component::Camera& c, Component::Transform& t) {
            if (c.mainCamera) {
                tra = t;
                cam = c;
            }
        });
        std::cout << tra.getPosition() << std::endl;
        std::cout << tra.getRotation() << std::endl;
    }
    if (code == KeyCode::KEY_R && action == InputAction::RELEASE) {
        getGlobals().useRayTracing = !getGlobals().useRayTracing;
    }
    if (code == KeyCode::KEY_H && action == InputAction::RELEASE) {
        MemoryManager::printAllocations();
    }
}
