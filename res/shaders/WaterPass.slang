import Common;
import Bounding;
import Scene;

struct WaterPayload
{
    float3 offset;
    float extent;
    uint numMeshes;
};

struct WaterTile
{
    int2 location;
    float extent;
    float height;
}

struct WaterVertex
{
    float4 position_CS : SV_Position;
    float3 position_WS : POSITION0;
    float2 texCoord : TEXCOORD0;
    float depth : DEPTH;
    uint lod : BONEINDEX;
};

struct MaterialParams
{
    float3 sunDirection;
    float displacementDepthAttenuation;
    
    float foamSubtract0;
    float foamSubtract1;
    float foamSubtract2;
    float foamSubtract3;
    
    float tile1;
    float tile2;
    float tile3;
    float tile4;

    float normalStrength;
    float foamDepthAttenuation;
    float normalDepthAttenuation;
    float roughness;
    
    float3 sunIrradiance;
    float foamRoughnessModifier;

    float3 scatterColor;
    float environmentLightStrength;

    float3 bubbleColor;
    float heightModifier;

    float bubbleDensity;
    float wavePeakScatterStrength;
    float scatterStrength;
    float scatterShadowStrength;

    uint contributeDisplacement1;
    uint contributeDisplacement2;
    uint contributeDisplacement3;
    uint contributeDisplacement4;
    
    float3 foamColor;

    Texture2DArray displacementTextures;
    Texture2DArray<float2> slopeTextures;
    TextureCube environmentMap;
    SamplerState sampler;
    StructuredBuffer<WaterTile> tiles;
};
layout(set = 1)
ParameterBlock<MaterialParams> pWaterMaterial;

[numthreads(1, 1, 1)]
[shader("amplification")]
void taskMain(
    uint threadID: SV_GroupThreadID,
    uint3 groupID: SV_GroupID
    ) {
    WaterTile tile = pWaterMaterial.tiles[groupID.x];
    GroupMemoryBarrierWithGroupSync();
    AABB bounding;
    bounding.minCorner = float3(tile.location.x, tile.height, tile.location.y) * tile.extent;
    bounding.maxCorner = float3(tile.location.x + 1, tile.height, tile.location.y + 1) * tile.extent;
    float3 median = (bounding.minCorner + bounding.maxCorner) / 2;
    float distance = distance(median, pViewParams.cameraPos_WS.xyz);
    float tileDistance = distance / tile.extent;
    uint numMeshes = groupID.y + 1;
    
    if(numMeshes == 4 && tileDistance > 2)
    {
        return;
    }
    if(numMeshes == 3 && (tileDistance > 4 || tileDistance < 1))
    {
        return;
    }
    if(numMeshes == 2 && (tileDistance > 8 || tileDistance < 3))
    {
        return;
    }
    if(numMeshes == 1 && tileDistance < 7)
    {
        return;
    }
    WaterPayload payload;
    payload.offset = bounding.minCorner;
    payload.extent = tile.extent / numMeshes;
    payload.numMeshes = numMeshes;
    DispatchMesh(numMeshes, numMeshes, 1, payload);
}

float SchlickFresnel(float3 normal, float3 viewDir) {
    // 0.02f comes from the reflectivity bias of water kinda idk it's from a paper somewhere i'm not gonna link it tho lmaooo
    return 0.02f + (1 - 0.02f) * (pow(1 - clamp(dot(normal, viewDir), 0, 1), 5.0f));
}

float SmithMaskingBeckmann(float3 H, float3 S, float roughness) {
    float hdots = max(0.001f, clamp(dot(H, S), 0, 1));
    float a = hdots / (roughness * sqrt(1 - hdots * hdots));
    float a2 = a * a;

    return a < 1.6f ? (1.0f - 1.259f * a + 0.396f * a2) / (3.535f * a + 2.181 * a2) : 0.0f;
}

float Beckmann(float ndoth, float roughness) {
    float exp_arg = (ndoth * ndoth - 1) / (roughness * roughness * ndoth * ndoth);

    return exp(exp_arg) / (PI * roughness * roughness * ndoth * ndoth * ndoth * ndoth);
}


const static uint64_t TILE_VERTS = 144;
const static uint64_t TILE_PRIMS = 242;

const static float2 VERT[TILE_VERTS] = {float2(0.0f / 11, 0.0f / 11), float2(0.0f / 11, 1.0f / 11), float2(0.0f / 11, 2.0f / 11), float2(0.0f / 11, 3.0f / 11), float2(0.0f / 11, 4.0f / 11), float2(0.0f / 11, 5.0f / 11), float2(0.0f / 11, 
6.0f / 11), float2(0.0f / 11, 7.0f / 11), float2(0.0f / 11, 8.0f / 11), float2(0.0f / 11, 9.0f / 11), float2(0.0f / 11, 10.0f / 11), float2(0.0f / 11, 11.0f / 11), float2(1.0f / 11, 0.0f / 11), float2(1.0f / 11, 1.0f / 11), float2(1.0f / 11, 2.0f / 11), float2(1.0f / 11, 3.0f / 11), float2(1.0f / 
11, 4.0f / 11), float2(1.0f / 11, 5.0f / 11), float2(1.0f / 11, 6.0f / 11), float2(1.0f / 11, 7.0f / 11), float2(1.0f / 11, 8.0f / 11), float2(1.0f / 11, 9.0f / 11), float2(1.0f / 11, 10.0f / 11), float2(1.0f / 11, 11.0f / 11), float2(2.0f / 11, 0.0f / 11), float2(2.0f / 11, 1.0f / 11), float2(2.0f / 11, 2.0f / 11), float2(2.0f / 11, 3.0f / 11), float2(2.0f / 11, 4.0f / 11), float2(2.0f / 11, 5.0f / 11), float2(2.0f / 11, 6.0f / 11), float2(2.0f / 11, 7.0f / 11), float2(2.0f / 11, 8.0f / 11), float2(2.0f / 11, 9.0f / 11), float2(2.0f / 11, 10.0f / 11), float2(2.0f / 11, 11.0f / 11), float2(3.0f / 11, 0.0f / 11), float2(3.0f / 11, 1.0f / 11), float2(3.0f / 11, 2.0f / 11), float2(3.0f / 11, 3.0f / 11), float2(3.0f / 11, 4.0f / 11), float2(3.0f / 11, 5.0f / 11), float2(3.0f / 11, 6.0f / 11), float2(3.0f / 11, 7.0f / 11), float2(3.0f / 11, 8.0f / 11), float2(3.0f / 11, 9.0f / 11), float2(3.0f / 11, 10.0f / 11), float2(3.0f / 11, 11.0f / 11), float2(4.0f / 11, 0.0f / 11), float2(4.0f 
/ 11, 1.0f / 11), float2(4.0f / 11, 2.0f / 11), float2(4.0f / 11, 3.0f / 11), float2(4.0f / 11, 4.0f / 11), float2(4.0f / 11, 5.0f / 11), float2(4.0f / 11, 6.0f / 11), float2(4.0f / 11, 7.0f / 11), float2(4.0f / 11, 8.0f / 11), float2(4.0f / 11, 9.0f / 11), float2(4.0f / 11, 10.0f / 11), float2(4.0f / 11, 11.0f / 11), float2(5.0f / 11, 0.0f / 11), float2(5.0f / 11, 1.0f / 11), float2(5.0f / 11, 
2.0f / 11), float2(5.0f / 11, 3.0f / 11), float2(5.0f / 11, 4.0f / 11), float2(5.0f / 11, 5.0f / 11), float2(5.0f / 11, 6.0f / 11), float2(5.0f / 11, 7.0f / 11), float2(5.0f / 11, 8.0f / 11), float2(5.0f / 11, 9.0f / 11), float2(5.0f / 11, 10.0f / 11), float2(5.0f / 11, 11.0f / 11), float2(6.0f / 
11, 0.0f / 11), float2(6.0f / 11, 1.0f / 11), float2(6.0f / 11, 2.0f / 11), float2(6.0f / 11, 3.0f / 11), float2(6.0f / 11, 4.0f / 11), float2(6.0f / 11, 5.0f / 11), float2(6.0f / 11, 6.0f / 11), float2(6.0f / 11, 7.0f / 11), float2(6.0f / 11, 8.0f / 11), float2(6.0f / 11, 9.0f / 11), float2(6.0f 
/ 11, 10.0f / 11), float2(6.0f / 11, 11.0f / 11), float2(7.0f / 11, 0.0f / 11), float2(7.0f / 11, 1.0f / 11), float2(7.0f / 11, 2.0f / 11), float2(7.0f / 11, 3.0f / 11), float2(7.0f / 11, 4.0f / 11), float2(7.0f / 11, 5.0f / 11), float2(7.0f / 11, 6.0f / 11), float2(7.0f / 11, 7.0f / 11), float2(7.0f / 11, 8.0f / 11), float2(7.0f / 11, 9.0f / 11), float2(7.0f / 11, 10.0f / 11), float2(7.0f / 11, 11.0f / 11), float2(8.0f / 11, 0.0f / 11), float2(8.0f / 11, 1.0f / 11), float2(8.0f / 11, 2.0f / 11), float2(8.0f / 11, 3.0f / 11), float2(8.0f / 11, 4.0f / 11), float2(8.0f / 11, 5.0f / 11), float2(8.0f / 11, 6.0f / 11), float2(8.0f / 11, 7.0f / 11), float2(8.0f / 11, 8.0f / 11), float2(8.0f / 
11, 9.0f / 11), float2(8.0f / 11, 10.0f / 11), float2(8.0f / 11, 11.0f / 11), float2(9.0f / 11, 0.0f / 11), float2(9.0f / 11, 1.0f / 11), float2(9.0f / 11, 2.0f / 11), float2(9.0f / 11, 3.0f / 11), float2(9.0f / 11, 4.0f / 11), float2(9.0f / 11, 5.0f / 11), float2(9.0f / 11, 6.0f / 11), float2(9.0f / 11, 7.0f / 11), float2(9.0f / 11, 8.0f / 11), float2(9.0f / 11, 9.0f / 11), float2(9.0f / 11, 10.0f / 11), float2(9.0f / 11, 11.0f / 11), float2(10.0f / 11, 0.0f / 11), float2(10.0f / 11, 1.0f / 11), float2(10.0f / 11, 2.0f / 11), float2(10.0f / 11, 3.0f / 11), float2(10.0f / 11, 4.0f / 11), float2(10.0f / 11, 5.0f / 11), float2(10.0f / 11, 6.0f / 11), float2(10.0f / 11, 7.0f / 11), float2(10.0f / 11, 8.0f / 11), float2(10.0f / 11, 9.0f / 11), float2(10.0f / 11, 10.0f / 11), float2(10.0f / 11, 11.0f / 11), float2(11.0f / 11, 0.0f / 11), float2(11.0f / 11, 1.0f / 11), float2(11.0f / 11, 2.0f / 11), float2(11.0f / 11, 3.0f / 11), float2(11.0f / 11, 4.0f / 11), float2(11.0f / 11, 5.0f / 11), float2(11.0f / 11, 6.0f / 11), float2(11.0f / 11, 7.0f / 11), float2(11.0f / 11, 8.0f / 11), float2(11.0f / 11, 9.0f / 11), float2(11.0f / 11, 10.0f / 11), float2(11.0f / 11, 11.0f / 11)};

const static uint3 IND[TILE_PRIMS] = {uint3(0, 1, 12), uint3(12, 1, 13), uint3(1, 2, 13), uint3(13, 2, 14), uint3(2, 3, 14), uint3(14, 3, 15), uint3(3, 4, 15), uint3(15, 4, 16), uint3(4, 5, 16), uint3(16, 5, 17), uint3(5, 6, 17), uint3(17, 6, 18), uint3(6, 7, 18), uint3(18, 7, 19), uint3(7, 8, 19), uint3(19, 8, 20), uint3(8, 9, 20), uint3(20, 9, 21), uint3(9, 10, 21), uint3(21, 10, 22), uint3(10, 11, 22), uint3(22, 11, 23), uint3(12, 13, 24), uint3(24, 13, 25), uint3(13, 14, 25), uint3(25, 14, 26), uint3(14, 
15, 26), uint3(26, 15, 27), uint3(15, 16, 27), uint3(27, 16, 28), uint3(16, 17, 28), uint3(28, 17, 29), uint3(17, 18, 29), uint3(29, 18, 30), uint3(18, 19, 30), uint3(30, 19, 31), uint3(19, 20, 31), uint3(31, 20, 32), uint3(20, 21, 32), uint3(32, 21, 33), uint3(21, 22, 33), uint3(33, 22, 34), uint3(22, 23, 34), uint3(34, 23, 35), uint3(24, 25, 36), uint3(36, 25, 37), uint3(25, 26, 
37), uint3(37, 26, 38), uint3(26, 27, 38), uint3(38, 27, 39), uint3(27, 28, 39), uint3(39, 28, 40), uint3(28, 29, 40), uint3(40, 29, 41), uint3(29, 30, 41), uint3(41, 30, 42), uint3(30, 31, 42), uint3(42, 31, 43), uint3(31, 32, 43), uint3(43, 32, 44), uint3(32, 33, 44), uint3(44, 33, 45), uint3(33, 34, 45), uint3(45, 34, 46), uint3(34, 35, 46), uint3(46, 35, 47), uint3(36, 37, 48), uint3(48, 37, 49), uint3(37, 38, 49), uint3(49, 38, 50), uint3(38, 39, 50), uint3(50, 39, 51), uint3(39, 40, 51), uint3(51, 40, 52), uint3(40, 41, 52), uint3(52, 41, 53), uint3(41, 42, 53), 
uint3(53, 42, 54), uint3(42, 43, 54), uint3(54, 43, 55), uint3(43, 44, 55), uint3(55, 44, 56), uint3(44, 45, 56), uint3(56, 45, 57), uint3(45, 46, 57), uint3(57, 46, 58), uint3(46, 47, 58), uint3(58, 47, 59), uint3(48, 49, 60), uint3(60, 49, 61), uint3(49, 50, 61), uint3(61, 50, 62), uint3(50, 51, 62), uint3(62, 51, 63), uint3(51, 52, 63), uint3(63, 52, 64), uint3(52, 53, 64), uint3(64, 53, 65), uint3(53, 54, 65), uint3(65, 54, 66), uint3(54, 55, 66), uint3(66, 55, 67), uint3(55, 56, 67), uint3(67, 56, 68), uint3(56, 57, 68), uint3(68, 57, 69), uint3(57, 58, 69), uint3(69, 58, 70), uint3(58, 59, 70), uint3(70, 59, 71), uint3(60, 61, 72), uint3(72, 61, 73), uint3(61, 62, 73), uint3(73, 62, 74), uint3(62, 63, 74), uint3(74, 63, 75), uint3(63, 64, 75), uint3(75, 64, 76), uint3(64, 65, 76), uint3(76, 65, 77), uint3(65, 66, 77), uint3(77, 66, 78), uint3(66, 67, 78), uint3(78, 67, 79), uint3(67, 68, 79), uint3(79, 68, 80), uint3(68, 69, 80), uint3(80, 69, 81), uint3(69, 70, 81), uint3(81, 70, 82), uint3(70, 71, 82), uint3(82, 71, 83), uint3(72, 
73, 84), uint3(84, 73, 85), uint3(73, 74, 85), uint3(85, 74, 86), uint3(74, 75, 86), uint3(86, 75, 87), uint3(75, 76, 87), uint3(87, 76, 88), uint3(76, 77, 88), uint3(88, 77, 89), uint3(77, 78, 89), uint3(89, 78, 90), uint3(78, 79, 90), uint3(90, 79, 91), uint3(79, 80, 91), uint3(91, 80, 92), uint3(80, 81, 92), uint3(92, 81, 93), uint3(81, 82, 93), uint3(93, 82, 94), uint3(82, 83, 
94), uint3(94, 83, 95), uint3(84, 85, 96), uint3(96, 85, 97), uint3(85, 86, 97), uint3(97, 86, 98), uint3(86, 87, 98), uint3(98, 87, 99), uint3(87, 88, 99), uint3(99, 88, 100), uint3(88, 89, 100), uint3(100, 89, 101), uint3(89, 90, 101), uint3(101, 90, 102), uint3(90, 91, 102), uint3(102, 91, 103), uint3(91, 92, 103), uint3(103, 92, 104), uint3(92, 93, 104), uint3(104, 93, 105), uint3(93, 94, 105), uint3(105, 94, 106), uint3(94, 95, 106), uint3(106, 95, 107), uint3(96, 97, 108), uint3(108, 97, 109), uint3(97, 98, 109), uint3(109, 98, 110), uint3(98, 99, 110), uint3(110, 99, 111), uint3(99, 100, 111), uint3(111, 100, 112), uint3(100, 101, 112), uint3(112, 101, 113), uint3(101, 102, 113), uint3(113, 102, 114), uint3(102, 103, 114), uint3(114, 103, 115), uint3(103, 104, 115), uint3(115, 104, 116), uint3(104, 105, 116), uint3(116, 105, 117), uint3(105, 106, 117), uint3(117, 106, 118), uint3(106, 107, 118), uint3(118, 107, 119), uint3(108, 109, 120), uint3(120, 109, 121), uint3(109, 110, 121), uint3(121, 110, 122), uint3(110, 111, 122), uint3(122, 111, 
123), uint3(111, 112, 123), uint3(123, 112, 124), uint3(112, 113, 124), uint3(124, 113, 125), uint3(113, 114, 125), uint3(125, 114, 126), uint3(114, 115, 126), uint3(126, 115, 127), uint3(115, 116, 127), uint3(127, 116, 128), uint3(116, 117, 128), uint3(128, 117, 129), uint3(117, 118, 129), uint3(129, 118, 130), uint3(118, 119, 130), uint3(130, 119, 131), uint3(120, 121, 132), uint3(132, 121, 133), uint3(121, 122, 133), uint3(133, 122, 134), uint3(122, 123, 134), uint3(134, 123, 135), uint3(123, 124, 135), uint3(135, 124, 136), uint3(124, 125, 136), uint3(136, 125, 137), uint3(125, 126, 137), uint3(137, 126, 138), uint3(126, 127, 138), uint3(138, 127, 139), uint3(127, 128, 139), uint3(139, 128, 140), uint3(128, 129, 140), uint3(140, 129, 141), uint3(129, 130, 141), uint3(141, 130, 142), uint3(130, 131, 142), uint3(142, 131, 143)};


[numthreads(MESH_GROUP_SIZE, 1, 1)]
[outputtopology("triangle")]
[shader("mesh")]
void meshMain(
    in uint threadID: SV_GroupThreadID,
    in uint3 groupID: SV_GroupID,
    in payload WaterPayload params,
    out vertices WaterVertex vertices[TILE_VERTS],
    out indices uint3 indices[TILE_PRIMS],
) {
    float4x4 vp = mul(pViewParams.projectionMatrix, pViewParams.viewMatrix);
	SetMeshOutputCounts(TILE_VERTS, TILE_PRIMS);
	
	for(uint i = threadID; i < TILE_PRIMS; i += MESH_GROUP_SIZE)
	{
        indices[i] = IND[i];
	}
	for(uint i = threadID; i < TILE_VERTS; i += MESH_GROUP_SIZE)
	{
        float2 base = VERT[i];
        float3 objectPos = float3(base.x, 0, base.y);
        float3 worldPos = params.offset + objectPos * params.extent + float3(groupID.x * params.extent, 0, groupID.y * params.extent);

        float lodDisplacement = 0;
        float3 camPos = pViewParams.cameraPos_WS.xyz;
        float cameraDistance = distance(worldPos, camPos);
        float threshold = 0;
        if(params.numMeshes == 3)
        {
            threshold = 10;
        }
        if(params.numMeshes == 2)
        {
            threshold = 30;
        }
        if(params.numMeshes == 1)
        {
            threshold = 70;
        }

        if(params.numMeshes < 4)
        {
            if(cameraDistance > threshold)
            {
                lodDisplacement = clamp(exp(-(1 / (cameraDistance + threshold))), 0, 1) - 1;
            }
            else
            {
                lodDisplacement = -1;
            }
        }
    

        float3 displacement1 = pWaterMaterial.displacementTextures.Sample(pWaterMaterial.sampler, float3(worldPos.xz * pWaterMaterial.tile1, 0)).xyz * pWaterMaterial.contributeDisplacement1;
        float3 displacement2 = pWaterMaterial.displacementTextures.Sample(pWaterMaterial.sampler, float3(worldPos.xz * pWaterMaterial.tile2, 1)).xyz * pWaterMaterial.contributeDisplacement2;
        float3 displacement3 = pWaterMaterial.displacementTextures.Sample(pWaterMaterial.sampler, float3(worldPos.xz * pWaterMaterial.tile3, 2)).xyz * pWaterMaterial.contributeDisplacement3;
        float3 displacement4 = pWaterMaterial.displacementTextures.Sample(pWaterMaterial.sampler, float3(worldPos.xz * pWaterMaterial.tile4, 3)).xyz * pWaterMaterial.contributeDisplacement4;
        float3 displacement = displacement1 + displacement2 + displacement3 + displacement4;

        float4 clipPos = mul(vp, float4(worldPos, 1));
        float ndcDepth = clipPos.z / clipPos.w;

        float depth = 1 - (-ndcDepth + 1.0f) / 2.0f;

        displacement = lerp(0.0f, displacement, pow(saturate(depth), pWaterMaterial.displacementDepthAttenuation));

        worldPos += displacement;
        worldPos.y += lodDisplacement;

        WaterVertex v;
        v.position_WS = worldPos;
        v.position_CS = mul(vp, float4(worldPos, 1));
        v.texCoord = worldPos.xz;
        v.depth = depth;
        v.lod = params.numMeshes;
        vertices[i] = v;
	}
}

[shader("pixel")]
float4 fragmentMain(WaterVertex vert) : SV_TARGET {
    float3 lightDir = -normalize(pWaterMaterial.sunDirection);
    float3 viewDir = normalize(pViewParams.c.cameraPos_WS.xyz - vert.position_WS);
    float3 halfwayDir = normalize(lightDir + viewDir);
    float depth = vert.depth;
    float LdotH = clamp(dot(lightDir, halfwayDir), 0, 1);
    float VdotH = clamp(dot(viewDir, halfwayDir), 0, 1);
    
    
    float4 displacementFoam1 = pWaterMaterial.displacementTextures.Sample(pWaterMaterial.sampler, float3(vert.position_WS.xz * pWaterMaterial.tile1, 0));
    displacementFoam1.a += pWaterMaterial.foamSubtract0;
    float4 displacementFoam2 = pWaterMaterial.displacementTextures.Sample(pWaterMaterial.sampler, float3(vert.position_WS.xz * pWaterMaterial.tile2, 0));
    displacementFoam2.a += pWaterMaterial.foamSubtract1;
    float4 displacementFoam3 = pWaterMaterial.displacementTextures.Sample(pWaterMaterial.sampler, float3(vert.position_WS.xz * pWaterMaterial.tile3, 0));
    displacementFoam3.a += pWaterMaterial.foamSubtract2;
    float4 displacementFoam4 = pWaterMaterial.displacementTextures.Sample(pWaterMaterial.sampler, float3(vert.position_WS.xz * pWaterMaterial.tile4, 0));
    displacementFoam4.a += pWaterMaterial.foamSubtract3;
    float4 displacementFoam = displacementFoam1 + displacementFoam2 + displacementFoam3 + displacementFoam4;

    float2 slopes1 = pWaterMaterial.slopeTextures.Sample(pWaterMaterial.sampler, float3(vert.position_WS.xz * pWaterMaterial.tile1, 0));
    float2 slopes2 = pWaterMaterial.slopeTextures.Sample(pWaterMaterial.sampler, float3(vert.position_WS.xz * pWaterMaterial.tile2, 1));
    float2 slopes3 = pWaterMaterial.slopeTextures.Sample(pWaterMaterial.sampler, float3(vert.position_WS.xz * pWaterMaterial.tile3, 2));
    float2 slopes4 = pWaterMaterial.slopeTextures.Sample(pWaterMaterial.sampler, float3(vert.position_WS.xz * pWaterMaterial.tile4, 3));
    float2 slopes = slopes1 + slopes2 + slopes3 + slopes4;
    
    slopes *= pWaterMaterial.normalStrength;
    float foam = lerp(0.0f, saturate(displacementFoam.a), pow(depth, pWaterMaterial.foamDepthAttenuation));

    float3 macroNormal = float3(0, 1, 0);
    float3 mesoNormal = normalize(float3(-slopes.x, 1.0f, -slopes.y));
    mesoNormal = normalize(lerp(macroNormal, mesoNormal, pow(saturate(depth), pWaterMaterial.normalDepthAttenuation)));
    mesoNormal = normalize(mesoNormal);

    float NdotL = clamp(dot(mesoNormal, lightDir), 0, 1);

    
    float a = pWaterMaterial.roughness + foam * pWaterMaterial.foamRoughnessModifier;
    float ndoth = max(0.0001f, dot(mesoNormal, halfwayDir));

    float viewMask = SmithMaskingBeckmann(halfwayDir, viewDir, a);
    float lightMask = SmithMaskingBeckmann(halfwayDir, lightDir, a);
    
    float G = rcp(1 + viewMask + lightMask);

    float eta = 1.33f;
    float R = ((eta - 1) * (eta - 1)) / ((eta + 1) * (eta + 1));
    float thetaV = acos(viewDir.y);

    float numerator = pow(1 - dot(mesoNormal, viewDir), 5 * exp(-2.69 * a));
    float F = R + (1 - R) * numerator / (1.0f + 22.7f * pow(a, 1.5f));
    F = saturate(F);
    
    float3 specular = pWaterMaterial.sunIrradiance * F * G * Beckmann(ndoth, a);
    specular /= 4.0f * max(0.001f, clamp(dot(macroNormal, lightDir), 0, 1));
    specular *= clamp(dot(mesoNormal, lightDir), 0, 1);

    float3 envReflection = pWaterMaterial.environmentMap.Sample(pWaterMaterial.sampler, reflect(-viewDir, mesoNormal)).rgb;
    envReflection *= pWaterMaterial.environmentLightStrength;

    float H = max(0.0f, displacementFoam.y) * pWaterMaterial.heightModifier;
    float3 scatterColor = pWaterMaterial.scatterColor;
    float3 bubbleColor = pWaterMaterial.bubbleColor;
    float bubbleDensity = pWaterMaterial.bubbleDensity;

    
    float k1 = pWaterMaterial.wavePeakScatterStrength * H * pow(clamp(dot(lightDir, -viewDir), 0, 1), 4.0f) * pow(0.5f - 0.5f * dot(lightDir, mesoNormal), 3.0f);
    float k2 = pWaterMaterial.scatterStrength * pow(clamp(dot(viewDir, mesoNormal), 0, 1), 2.0f);
    float k3 = pWaterMaterial.scatterShadowStrength * NdotL;
    float k4 = bubbleDensity;

    float3 scatter = (k1 + k2) * scatterColor * pWaterMaterial.sunIrradiance * rcp(1 + lightMask);
    scatter += k3 * scatterColor * pWaterMaterial.sunIrradiance + k4 * bubbleColor * pWaterMaterial.sunIrradiance;

    
    float3 output = (1 - F) * scatter + specular + F * envReflection;
    output = max(0.0f, output);
    output = lerp(output, pWaterMaterial.foamColor, saturate(foam));
    return float4(output, 1);

    //if(vert.lod == 1)
    //{
    //    return float4(1, 0, 0, 1.0f);
    //}
    //if(vert.lod == 2)
    //{
    //    return float4(0, 1, 0, 1.0f);
    //}
    //if(vert.lod == 3)
    //{
    //    return float4(0, 0, 1, 1.0f);
    //}
    //if(vert.lod == 4)
    //{
    //    return float4(1, 1, 1, 1.0f);
    //}
    //return float4(0, 0, 0, 1);
}
