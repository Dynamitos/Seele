#pragma once
#include "RenderGraphResources.h"
#include "RenderPass.h"

namespace Seele {
class RenderGraph {
  public:
    RenderGraph() { res = new RenderGraphResources(); }
    void addPass(ORenderPass pass) {
        pass->setResources(res);
        passes.add(std::move(pass));
    }
    void setViewport(Gfx::PViewport viewport) {
        for (auto& pass : passes) {
            pass->setViewport(viewport);
        }
    }
    void createRenderPass() {
        for (auto& pass : passes) {
            pass->publishOutputs();
        }
        for (auto& pass : passes) {
            pass->createRenderPass();
        }
    }
    void render(const Component::Camera& cam, const Component::Transform& transform) {
        for (auto& pass : passes) {
            pass->beginFrame(cam, transform);
        }
        for (auto& pass : passes) {
            pass->render();
        }
        for (auto& pass : passes) {
            pass->endFrame();
        }
    }
    PRenderGraphResources getResources() {
        return res;
    }
  private:
    PRenderGraphResources res;
    List<ORenderPass> passes;
};

} // namespace Seele
