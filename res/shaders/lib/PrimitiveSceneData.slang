struct PrimitiveSceneData
{
    float4x4 modelToWorld;
    float4x4 worldToModel;
    float4 actorLocation;
};

layout(set = INDEX_SCENE_DATA, binding = 0, std430)
ShaderBuffer<PrimitiveSceneData> gSceneData;

[[vk::push_constant]]
ConstantBuffer<uint> gSceneDataIndex;

PrimitiveSceneData getSceneData()
{
    return gSceneData[gSceneDataIndex];
}