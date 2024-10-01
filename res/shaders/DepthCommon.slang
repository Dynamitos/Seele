struct DepthDebugData
{
    uint mipLevel;
    uint mipOffset;
    int2 mipDimensions;
    int2 screenCornerMin;
    int2 screenCornerMax;
    int2 origScreenMin;
    int2 origScreenMax;
};

struct DepthData
{
    //uint bufferLength;
    Texture2D<float> texture;
    RWStructuredBuffer<float> buffer;
    //globallycoherent RWStructuredBuffer<uint> debugHead;
    //globallycoherent RWStructuredBuffer<DepthDebugData> debugData;
};
ParameterBlock<DepthData> pDepthAttachment;
