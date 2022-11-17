#pragma once
#include "RenderPass.h"

namespace Seele
{
template<typename... RenderPasses>
class RenderGraph;

template<>
class RenderGraph<>
{
public:
    RenderGraph(PRenderGraphResources) {}
    void updatePassData() {}
protected:
    void setViewport(Gfx::PViewport) {}
    void createRenderPass() {}
    void beginFrame(const Component::Camera&) {}
    void render() {}
    void endFrame() {}
};

template<typename This, typename... Rest>
class RenderGraph<This, Rest...> : private RenderGraph<Rest...>
{
public:
    RenderGraph(PRenderGraphResources resources, This&& pass, Rest&&... rest)
        : RenderGraph<Rest...>(resources, std::move(rest)...)
        , resources(resources)
        , rp(std::move(pass))
    {
        rp.setResources(resources);
    }
    template<typename PassData, typename... OtherData>
    void updatePassData(PassData passData, OtherData... otherData)
    {
        rp.updateViewFrame(std::move(passData));
        RenderGraph<Rest...>::updatePassData(otherData...);
    }
    void updateViewport(Gfx::PViewport viewport)
    {
        setViewport(viewport);
        createRenderPass();
    }
    void render(const Component::Camera& cam)
    {
        beginFrame(cam);
        render();
        endFrame();
    }
protected:
    void setViewport(Gfx::PViewport viewport)
    {
        rp.setViewport(viewport);
        RenderGraph<Rest...>::setViewport(viewport);
    }
    void createRenderPass()
    {
        rp.createRenderPass();
        RenderGraph<Rest...>::createRenderPass();
    }
    void beginFrame(const Component::Camera& cam)
    {
        rp.beginFrame(cam);
        RenderGraph<Rest...>::beginFrame(cam);
    }
    void render()
    {
        rp.render();
        RenderGraph<Rest...>::render();
    }
    void endFrame()
    {
        rp.endFrame();
        RenderGraph<Rest...>::endFrame();
    }
private:
    PRenderGraphResources resources;
    This rp;
};

class RenderGraphBuilder
{
public:
    template<typename... RenderPasses>
    static RenderGraph<RenderPasses...> build(RenderPasses&&... renderPasses)
    {
        PRenderGraphResources resources = new RenderGraphResources();
        return RenderGraph<RenderPasses...>(resources, std::move(renderPasses)...);
    }
private:
    RenderGraphBuilder() = delete;
};

} // namespace Seele
