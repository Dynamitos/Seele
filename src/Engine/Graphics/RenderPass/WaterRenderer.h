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
    Gfx::PDescriptorSet computeSet;
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
        float frameTime;
        float deltaTime;
        float gravity = 9.81f;
        float repeatTime = 200.0f;
        float depth = 20.0f;
        float lowCutoff = 0.0001f;
        float highCutoff = 9000.0f;
        int seed = 0;
        Vector2 lambda = Vector2(1, 1);
        uint32 N = 1024;
        uint32 lengthScale0 = 256;
        uint32 lengthScale1 = 256;
        uint32 lengthScale2 = 256;
        uint32 lengthScale3 = 256;
        float foamBias = -0.5f;
        float foamDecayRate = 0.05f;
        float foamAdd = 0.5f;
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
        Vector sunDirection = Vector(0, -1, 0);
        float displacementDepthAttenuation = 1;

        float foamSubtract0 = 0;
        float foamSubtract1 = 0;
        float foamSubtract2 = 0;
        float foamSubtract3 = 0;

        float normalStrength = 1;
        float foamDepthAttenuation = 1;
        float normalDepthAttenuation = 1;
        float roughness = 0.1f;

        Vector sunIrradiance = Vector(1, 1, 1);
        float foamRoughnessModifier = 1;

        Vector scatterColor = Vector(1, 1, 1);
        float environmentLightStrength = 1;

        Vector bubbleColor = Vector(1, 1, 1);
        float heightModifier = 1;

        float bubbleDensity = 1;
        float wavePeakScatterStrength = 1;
        float scatterStrength = 1;
        float scatterShadowStrength = 1;

        Vector foamColor = Vector(1, 1, 1);
    } materialParams;
    // Water
    Gfx::OUniformBuffer materialUniforms;
    Gfx::PTextureCube skyBox;
    Gfx::OShaderBuffer waterTiles;
    Gfx::OTaskShader waterTask;
    Gfx::OMeshShader waterMesh;
    Gfx::OFragmentShader waterFragment;
    Gfx::ODescriptorLayout materialLayout;
    Gfx::PDescriptorSet materialSet;
    Gfx::OPipelineLayout waterLayout;
    Gfx::PGraphicsPipeline waterPipeline;

    Gfx::PViewport viewport;
};
DEFINE_REF(WaterRenderer)
}