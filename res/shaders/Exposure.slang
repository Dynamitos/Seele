
struct HistogramParameters
{
    float minLogLum;
    float inverseLogLumRange;
    float timeCoeff;
    uint numPixels;
    Texture2D hdrImage;
    globallycoherent RWStructuredBuffer<uint> histogram;
    RWStructuredBuffer<float> averageLuminance;
};
ParameterBlock<HistogramParameters> pHistogramParams;

static const float3 RGB_TO_LUMINANCE = float3(0.2125, 0.7154, 0.0721);
static const float eps = 0.005;

uint colorToBin(float3 hdrColor) {
    float lum = dot(hdrColor, RGB_TO_LUMINANCE);

    if(lum < eps) {
        return 0;
    }

    float logLum = clamp((log2(lum) - pHistogramParams.minLogLum) * pHistogramParams.inverseLogLumRange, 0, 1);

    return uint(logLum * 254.0 + 1.0);
}

groupshared uint histogramShared[256];

[shader("compute")]
[numthreads(16, 16, 1)]
void histogram(uint threadID : SV_GroupIndex, uint2 globalThreadID : SV_DispatchThreadID)
{
    histogramShared[threadID] = 0;
    GroupMemoryBarrierWithGroupSync();

    uint2 dim;
    pHistogramParams.hdrImage.GetDimensions(dim.x, dim.y);
    if(globalThreadID.x < dim.x && globalThreadID.y < dim.y) {
        float3 hdrColor = pHistogramParams.hdrImage.Load(int3(globalThreadID, 0)).xyz;
        uint binIndex = colorToBin(hdrColor);

        InterlockedAdd(histogramShared[binIndex], 1);
    }

    GroupMemoryBarrierWithGroupSync();

    InterlockedAdd(pHistogramParams.histogram[threadID], histogramShared[threadID]);
}

[shader("compute")]
[numthreads(256, 1, 1)]
void exposure(uint threadID : SV_GroupThreadID)
{
    uint countForThisBin = pHistogramParams.histogram[threadID];
    histogramShared[threadID] = countForThisBin * threadID;

    GroupMemoryBarrierWithGroupSync();

    pHistogramParams.histogram[threadID] = 0;

    for(uint cutoff = (256 >> 1); cutoff > 0; cutoff >>= 1) {
        if(uint(threadID) < cutoff) {
            histogramShared[threadID] += histogramShared[threadID + cutoff];
        }
        GroupMemoryBarrierWithGroupSync();
    }
    if(threadID == 0) {
        float weightedLogAverage = (histogramShared[0] / max(pHistogramParams.numPixels - float(countForThisBin), 1.0f)) - 1.0f;

        float weightedAvgLum = exp2(((weightedLogAverage / 254.0f) * pHistogramParams.inverseLogLumRange) + pHistogramParams.minLogLum);

        float lumLastFrame = pHistogramParams.averageLuminance[0];
        float adaptedLum = lumLastFrame + (weightedAvgLum - lumLastFrame) * pHistogramParams.timeCoeff;
        pHistogramParams.averageLuminance[0] = adaptedLum;
    }
}