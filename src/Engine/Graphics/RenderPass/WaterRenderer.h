#pragma once
#include "RenderPass.h"

namespace Seele {
class WaterRenderer {
  public:
    WaterRenderer(Gfx::PGraphics graphics, PScene scene, Gfx::PDescriptorLayout viewParamsLayout);
    ~WaterRenderer();
    void beginFrame();
    Gfx::ORenderCommand render(Gfx::PDescriptorSet viewParamsSet);
    void setViewport(Gfx::PViewport viewport, Gfx::PRenderPass renderPass);

  private:
    float jonswapAlpha(float fetch, float windSpeed) const;
    float jonswapPeakFrequency(float fetch, float windSpeed) const;
    void updateFFTDescriptor();
    void updateMaterialDescriptor();
    Gfx::PGraphics graphics;
    PScene scene;
    Gfx::ODescriptorLayout computeLayout;
    Gfx::ODescriptorSet computeSet;
    Gfx::OPipelineLayout pipelineLayout;

    Gfx::OComputeShader initSpectrumCS;
    Gfx::PComputePipeline initSpectrum;
    Gfx::OComputeShader packSpectrumConjugateCS;
    Gfx::PComputePipeline packSpectrumConjugate;
    Gfx::OComputeShader updateSpectrumForFFTCS;
    Gfx::PComputePipeline updateSpectrumForFFT;
    Gfx::OComputeShader horizontalFFTCS;
    Gfx::PComputePipeline horizontalFFT;
    Gfx::OComputeShader verticalFFTCS;
    Gfx::PComputePipeline verticalFFT;
    Gfx::OComputeShader assembleMapsCS;
    Gfx::PComputePipeline assembleMaps;

    struct SpectrumParameters {
        float scale;
        float angle;
        float spreadBlend;
        float swell;
        float alpha;
        float peakOmega;
        float gamma;
        float shortWavesFade;
    };
    StaticArray<SpectrumParameters, 8> spectrums;
    struct DisplaySpectrumSettings {
        float scale = 1;
        float windSpeed = 1;
        float windDirection = 0;
        float fetch = 1;
        float spreadBlend = 1;
        float swell = 1;
        float peakEnhancement = 1;
        float shortWavesFade = 1;
    };
    StaticArray<DisplaySpectrumSettings, 8> displaySpectrums;
    struct ComputeParams {
        float frameTime = 0.0f;
        float deltaTime = 0.0f;
        float gravity = 9.81f;
        float repeatTime = 200.0f;
        float depth = 20.0f;
        float lowCutoff = 0.0001f;
        float highCutoff = 9000.0f;
        int seed = 1;
        Vector2 lambda = Vector2(1, 1);
        uint32 N = 1024;

        // Layer 1
        uint32 lengthScale1 = 94;

        // Layer 2
        uint32 lengthScale2 = 128;

        // Layer 3
        uint32 lengthScale3 = 64;

        // Layer 4
        uint32 lengthScale4 = 32;
        
        float foamBias = 0.85f;
        float foamDecayRate = 0.0175f;
        float foamAdd = 0.1f;
        float foamThreshold = 0.0f;
    } params;
    float speed = 1.0f;
    float displacementDepthFalloff = 1.0f;
    uint32 logN;
    uint32 threadGroupsX;
    uint32 threadGroupsY;
    Gfx::OUniformBuffer paramsBuffer;
    Gfx::OTexture2D spectrumTextures, initialSpectrumTextures, displacementTextures;
    Gfx::OTexture2D slopeTextures;
    Gfx::OTexture2D boyancyData;
    Gfx::OShaderBuffer spectrumBuffer;
    Gfx::OSampler linearRepeatSampler;

    struct MaterialParams {
        Vector sunDirection = Vector(-5, 0.6186, 5);
        float displacementDepthAttenuation = 1;

        float foamSubtract0 = 0.04f;
        float foamSubtract1 = -0.04f;
        float foamSubtract2 = -0.46f;
        float foamSubtract3 = -0.38f;

        float tile1 = 0.01f;
        float tile2 = 3.0f;
        float tile3 = 3.0f;
        float tile4 = 0.13f;

        float normalStrength = 1;
        float foamDepthAttenuation = 1;
        float normalDepthAttenuation = 1;
        float roughness = 0.075f;

        Vector sunIrradiance = Vector(0.9921, 0.9058, 0.5450);
        float foamRoughnessModifier = 0;

        Vector scatterColor = Vector(0.016f, 0.0735f, 0.16f);
        float environmentLightStrength = 0.5f;

        Vector bubbleColor = Vector(0, 0.02f, 0.016f);
        float heightModifier = 1;

        float bubbleDensity = 1;
        float wavePeakScatterStrength = 1;
        float scatterStrength = 1;
        float scatterShadowStrength = 0.5f;

        uint32 contributeDisplacement1 = true;
        uint32 contributeDisplacement2 = false;
        uint32 contributeDisplacement3 = false;
        uint32 contributeDisplacement4 = false;

        Vector foamColor = Vector(0.6f, 0.5568f, 0.492f);
    } materialParams;
    // Water
    Gfx::OUniformBuffer materialUniforms;
    Gfx::PTextureCube skyBox;
    Gfx::OShaderBuffer waterTiles;
    Gfx::OTaskShader waterTask;
    Gfx::OMeshShader waterMesh;
    Gfx::OFragmentShader waterFragment;
    Gfx::ODescriptorLayout materialLayout;
    Gfx::ODescriptorSet materialSet;
    Gfx::OPipelineLayout waterLayout;
    Gfx::PGraphicsPipeline waterPipeline;

    Gfx::PViewport viewport;
};
DEFINE_REF(WaterRenderer)
}