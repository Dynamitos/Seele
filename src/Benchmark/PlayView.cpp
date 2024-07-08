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
        Gfx::PTimestampQuery timeQuery = res->requestTimestampQuery("TIMESTAMP");
        std::ofstream stats(fmt::format("stats{}.csv", useMeshCulling ? "" : "NOCULL"));
        stats << std::fixed << std::setprecision(0);
        stats << "RelTime,"
              << "CachedIAV,CachedIAP,CachedVS,CachedClipInv,CachedClipPrim,CachedFS,CachedCS,CachedTS,CachedMS,"
              << "DepthIAV,DepthIAP,DepthVS,DepthClipInv,DepthClipPrim,DepthFS,DepthCS,DepthTS,DepthMS,"
              << "BaseIAV,BaseIAP,BaseVS,BaseClipInv,BaseClipPrim,BaseFS,BaseCS,BaseTS,BaseMS,"
              << "LightCullIAV,LightCullIAP,LightCullVS,LightCullClipInv,LightCullClipPrim,LightCullFS,LightCullCS,LightCullTS,LightCullMS,"
              << "VisibilityIAV,VisibilityIAP,VisibilityVS,VisibilityClipInv,VisibilityClipPrim,VisibilityFS,VisibilityCS,VisibilityTS,"
                 "VisibilityMS,"
              << "CACHED,MIPGEN,DEPTHCULL,VISIBILITY,LIGHTCULL,BASE,FrameTime" << std::endl;
        float start = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        while (getGlobals().running) {
            auto timestamps = timeQuery->getResults();
            auto cachedResults = cachedQuery->getResults();
            auto depthResults = depthQuery->getResults();
            auto baseResults = baseQuery->getResults();
            auto lightCullResults = lightCullQuery->getResults();
            auto visiblityResults = visibilityQuery->getResults();
            if (start == 0) {
                start = timestamps[0].time;
            }
            uint64 t0 = timestamps[0].time;
            uint64 t1 = timestamps[1].time;
            uint64 t2 = timestamps[2].time;
            uint64 t3 = timestamps[3].time;
            uint64 t4 = timestamps[4].time;
            uint64 t5 = timestamps[5].time;
            uint64 t6 = timestamps[6].time;
            if (t1 < t0 || t2 < t1 || t3 < t2 || t4 < t3 || t5 < t4 || t6 < t5)
                continue;
            stats << t0 - start << "," << cachedResults << depthResults << baseResults << lightCullResults << visiblityResults << t1 - t0
                  << "," << t2 - t1 << "," << t3 - t2 << "," << t4 - t3 << "," << t5 - t4 << "," << t6 - t5 << "," << t6 - t0 << std::endl;
            stats.flush();
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
