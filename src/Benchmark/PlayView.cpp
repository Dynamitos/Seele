#include "PlayView.h"
#include "Window/Window.h"
#include <fmt/format.h>
#include <fstream>

using namespace Seele;

PlayView::PlayView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo& createInfo, std::string dllPath, bool useMeshCulling)
    : GameView(graphics, window, createInfo, dllPath) {
    getGlobals().useDepthCulling = useMeshCulling;
    queryThread = std::thread([&]() {
        PRenderGraphResources res = renderGraph.getResources();
        Gfx::PPipelineStatisticsQuery cachedQuery = res->requestQuery("CACHED_QUERY");
        Gfx::PPipelineStatisticsQuery depthQuery = res->requestQuery("DEPTH_QUERY");
        Gfx::PPipelineStatisticsQuery baseQuery = res->requestQuery("BASEPASS_QUERY");
        Gfx::PPipelineStatisticsQuery lightCullQuery = res->requestQuery("LIGHTCULL_QUERY");
        Gfx::PPipelineStatisticsQuery visibilityQuery = res->requestQuery("VISIBILITY_QUERY");
        Gfx::PTimestampQuery cachedTS = res->requestTimestampQuery("CACHED_TS");
        Gfx::PTimestampQuery depthTS = res->requestTimestampQuery("DEPTH_TS");
        Gfx::PTimestampQuery lightCullTS = res->requestTimestampQuery("LIGHTCULL_TS");
        Gfx::PTimestampQuery visibilityTS = res->requestTimestampQuery("VISIBILITY_TS");
        Gfx::PTimestampQuery baseTS = res->requestTimestampQuery("BASE_TS");
        std::ofstream stats(fmt::format("stats{}.csv", useMeshCulling ? "" : "NOCULL"));
        stats << "RelTime,"
              << "CACHED,MIPGEN,DEPTHCULL,VISIBILITY,LIGHTCULL,BASE,FrameTime,"
              << "CachedIAV,CachedIAP,CachedVS,CachedClipInv,CachedClipPrim,CachedFS,CachedCS,"
              << "DepthIAV,DepthIAP,DepthVS,DepthClipInv,DepthClipPrim,DepthFS,DepthCS,"
              << "BaseIAV,BaseIAP,BaseVS,BaseClipInv,BaseClipPrim,BaseFS,BaseCS,"
              << "LightCullIAV,LightCullIAP,LightCullVS,LightCullClipInv,LightCullClipPrim,LightCullFS,LightCullCS,"
              << "VisibilityIAV,VisibilityIAP,VisibilityVS,VisibilityClipInv,VisibilityClipPrim,VisibilityFS,VisibilityCS" << std::endl;
        uint64 start = 0;
        uint64 prevBegin = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        stats << std::fixed << std::setprecision(0);
        while (getGlobals().running) {
            auto cached = cachedTS->getResults();
            auto depth = depthTS->getResults();
            auto lightCull = lightCullTS->getResults();
            auto visibility = visibilityTS->getResults();
            auto base = baseTS->getResults();
            auto cachedResults = cachedQuery->getResults();
            auto depthResults = depthQuery->getResults();
            auto baseResults = baseQuery->getResults();
            auto lightCullResults = lightCullQuery->getResults();
            auto visiblityResults = visibilityQuery->getResults();
            if (start == 0) {
                start = cached[0].time;
            }
            int64 relTime = cached[0].time - start;
            int64 cachedTime = cached[1].time - cached[0].time;
            int64 mipTime = depth[1].time - depth[0].time;
            int64 depthTime = depth[2].time - depth[0].time;
            int64 baseTime = base[1].time - base[0].time;
            int64 lightCullTime = lightCull[1].time - lightCull[0].time;
            int64 visibilityTime = visibility[1].time - visibility[0].time;
            int64 frameTime = cachedTime + mipTime + depthTime + baseTime + lightCullTime + visibilityTime;

            stats << relTime << "," << cachedTime << "," << mipTime << "," << depthTime << ","
                  << visibilityTime
                  << "," << lightCullTime << "," << baseTime << ","
                  << frameTime << "," << cachedResults << depthResults << baseResults << lightCullResults << visiblityResults << std::endl;
            stats.flush();
            // std::cout << "Writing " << timestamps[0].time << std::endl;
        }
    });
}

PlayView::~PlayView() {}

void PlayView::beginUpdate() { GameView::beginUpdate(); }

void PlayView::update() { GameView::update(); }

void PlayView::commitUpdate() { GameView::commitUpdate(); }

void PlayView::prepareRender() { GameView::prepareRender(); }

void PlayView::render() { GameView::render(); }

void PlayView::keyCallback(KeyCode code, InputAction action, KeyModifier modifier) { GameView::keyCallback(code, action, modifier); }
