#include "PlayView.h"
#include "Window/Window.h"
#include <fmt/format.h>
#include <fstream>

using namespace Seele;

PlayView::PlayView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo& createInfo, std::string dllPath, bool useMeshCulling)
    : GameView(graphics, window, createInfo, dllPath) {
    getGlobals().useDepthCulling = useMeshCulling;
    renderTimestamp = graphics->createTimestampQuery(2, "RenderTimestamp");
    queryThread = std::thread([&]() {
        PRenderGraphResources res = renderGraph.getResources();
        Gfx::PPipelineStatisticsQuery cachedQuery = res->requestQuery("CACHED_QUERY");
        Gfx::PPipelineStatisticsQuery depthQuery = res->requestQuery("DEPTH_QUERY");
        Gfx::PPipelineStatisticsQuery baseQuery = res->requestQuery("BASEPASS_QUERY");
        Gfx::PPipelineStatisticsQuery lightCullQuery = res->requestQuery("LIGHTCULL_QUERY");
        Gfx::PPipelineStatisticsQuery visibilityQuery = res->requestQuery("VISIBILITY_QUERY");
        Gfx::PTimestampQuery timestamps = res->requestTimestampQuery("TIMESTAMPS");
        std::ofstream stats(fmt::format("stats{}.csv", useMeshCulling ? "" : "NOCULL"));
        stats << "RelTime,"
              << "CACHED,MIPGEN,DEPTHCULL,VISIBILITY,LIGHTCULL,BASE,FrameTime,"
              << "CachedIAV,CachedIAP,CachedVS,CachedClipInv,CachedClipPrim,CachedFS,CachedCS,CachedTS,CachedMS,"
              << "DepthIAV,DepthIAP,DepthVS,DepthClipInv,DepthClipPrim,DepthFS,DepthCS,DepthTS,DepthMS,"
              << "BaseIAV,BaseIAP,BaseVS,BaseClipInv,BaseClipPrim,BaseFS,BaseCS,BaseTS,BaseMS,"
              << "LightCullIAV,LightCullIAP,LightCullVS,LightCullClipInv,LightCullClipPrim,LightCullFS,LightCullCS,LightCullTS,LightCullMS,"
              << "VisibilityIAV,VisibilityIAP,VisibilityVS,VisibilityClipInv,VisibilityClipPrim,VisibilityFS,VisibilityCS,VisibilityMS,"
                 "VisibilityMS,"
              << std::endl;
        uint64 start = 0;
        uint64 prevBegin = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        stats << std::fixed << std::setprecision(0);
        while (getGlobals().running) {
            auto cachedResults = cachedQuery->getResults();
            auto depthResults = depthQuery->getResults();
            auto baseResults = baseQuery->getResults();
            auto lightCullResults = lightCullQuery->getResults();
            auto visiblityResults = visibilityQuery->getResults();
            int64 renderBegin = renderTimestamp->getResult().time;
            int64 renderEnd = renderTimestamp->getResult().time;
            Map<std::string, uint64> results;
            for (uint32 i = 0; i < 11; ++i) {
                auto ts = timestamps->getResult();
                results[ts.name] = ts.time;
            }
            if (start == 0) {
                start = results.at("CachedBegin");
            }
            int64 relTime = results.at("CachedBegin") - start;
            int64 cachedTime = results.at("CachedEnd") - results.at("CachedBegin");
            int64 mipTime = results.at("CullingBegin") - results.at("MipBegin");
            int64 depthTime = results.at("CullingEnd") - results.at("CullingBegin");
            int64 baseTime = results.at("BaseEnd") - results.at("BaseBegin");
            int64 lightCullTime = results.at("LightCullEnd") - results.at("LightCullBegin");
            int64 visibilityTime = results.at("VisibilityEnd") - results.at("VisibilityBegin");
            int64 frameTime = cachedTime + mipTime + depthTime + baseTime + lightCullTime + visibilityTime;

            stats << relTime << "," << cachedTime << "," << mipTime << "," << depthTime << "," << visibilityTime << "," << lightCullTime
                  << "," << baseTime << "," << frameTime << "," << cachedResults << depthResults << baseResults << lightCullResults
                  << visiblityResults << std::endl;
            stats.flush();
        }
    });
}

PlayView::~PlayView() {}

void PlayView::beginUpdate() { GameView::beginUpdate(); }

void PlayView::update() { GameView::update(); }

void PlayView::commitUpdate() { GameView::commitUpdate(); }

void PlayView::prepareRender() { GameView::prepareRender(); }

void PlayView::render() {
    renderTimestamp->write(Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT, "RenderBegin");
    GameView::render();
    renderTimestamp->write(Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT, "RenderEnd");
}

void PlayView::keyCallback(KeyCode code, InputAction action, KeyModifier modifier) { GameView::keyCallback(code, action, modifier); }
