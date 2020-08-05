struct PrimitiveSceneData
{
    float4x4 localToWorld;
    float4x4 worldToLocal;
};


struct ViewParameter
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4 cameraPos_WS;
};
layout(set = 0)
ConstantBuffer<ViewParameter> gViewParams;

layout(set = 2)
ConstantBuffer<PrimitiveSceneData> gSceneData;

