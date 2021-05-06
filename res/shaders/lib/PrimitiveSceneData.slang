struct PrimitiveSceneData
{
    float4x4 localToWorld;
    float4x4 worldToLocal;
    float4 actorLocation;
};

layout(set = INDEX_SCENE_DATA, binding = 0, std430)
ConstantBuffer<PrimitiveSceneData> gSceneData;
