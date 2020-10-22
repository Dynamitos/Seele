struct PrimitiveSceneData
{
    float4x4 localToWorld;
    float4x4 worldToLocal;
    float4 actorLocation;
};

layout(set = 3, binding = 0, std430)
ConstantBuffer<PrimitiveSceneData> gSceneData;
