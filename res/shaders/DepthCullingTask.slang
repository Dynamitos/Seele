import Common;
import Scene;
import Bounding;

groupshared MeshPayload p;
groupshared uint head;
groupshared MeshData mesh;
groupshared InstanceData instance;
groupshared Frustum viewFrustum;
groupshared float4x4 modelViewProjection;
groupshared bool meshVisible;

struct DepthData
{
    Texture2D<float> texture;
    RWStructuredBuffer<float> buffer;
}

ParameterBlock<DepthData> pDepthAttachment;

bool isBoxVisible(AABB bounding)
{
    uint2 mipDimensions = uint2(uint(pViewParams.screenDimensions.x), uint(pViewParams.screenDimensions.y));
    // now we calculate what mip level we need to only sample up to 4 texels covering the entire meshlet
    uint2 screenCornerMin = mipDimensions;
    uint2 screenCornerMax = uint2(0, 0);
    // lower values are closer
    float maxDepth = bounding.projectScreenDepth(modelViewProjection, screenCornerMin, screenCornerMax);
    uint mipOffset = 0;
    // in theory this wouldnt work if no corner was in screen, as min would be greater that max, however we verified that with view culling
    while(screenCornerMax.x - screenCornerMin.x > 1 || screenCornerMax.y - screenCornerMin.y > 1)
    {
        mipOffset += mipDimensions.x * mipDimensions.y;
        mipDimensions = uint2(mipDimensions.x + 1, mipDimensions.y + 1) / 2;
        screenCornerMin = uint2(screenCornerMin.x + 1, screenCornerMin.y + 1) / 2;
        screenCornerMax = uint2(screenCornerMax.x, screenCornerMax.y) / 2;
    }

    // now we sample 4 texels from the depth at the calculated mip level, this should give us the screen extent of the meshlet
    float d1 = pDepthAttachment.buffer[mipOffset + (screenCornerMin.y * mipDimensions.x) + screenCornerMin.x];
    float d2 = pDepthAttachment.buffer[mipOffset + (screenCornerMin.y * mipDimensions.x) + screenCornerMax.x];
    float d3 = pDepthAttachment.buffer[mipOffset + (screenCornerMax.y * mipDimensions.x) + screenCornerMin.x];
    float d4 = pDepthAttachment.buffer[mipOffset + (screenCornerMax.y * mipDimensions.x) + screenCornerMax.x];

    // we want to check if the minimum depth (the value farthest away) is smaller than the maximum bounding box depth
    // otherwise, there is no way for the meshlet to be visible
    float d = min(min(d1, d2), min(d3, d4));
                
    if(d < maxDepth)
    {
        return true;
    }
    return false;
}

[numthreads(TASK_GROUP_SIZE, 1, 1)]
[shader("amplification")]
void taskMain(
    uint threadID: SV_GroupThreadID,
    uint groupID: SV_GroupID, )
{
    if (threadID == 0)
    {
        head = 0;
	    instance = pScene.instances[pOffsets.instanceOffset + groupID];
        mesh = pScene.meshData[pOffsets.instanceOffset + groupID];
        p.instanceId = pOffsets.instanceOffset + groupID;
        p.meshletOffset = mesh.meshletOffset;
        p.cullingOffset = pScene.cullingOffsets[p.instanceId];
        modelViewProjection = mul(mul(pViewParams.projectionMatrix, pViewParams.viewMatrix), instance.transformMatrix);
		float3 origin = viewToModel(instance.inverseTransformMatrix, float4(0, 0, 0, 1)).xyz;
		const float offset = 0.0f;
		float3 corners[4] = {
			screenToModel(instance.inverseTransformMatrix, float4(offset, offset, -1.0f, 1.0f)).xyz,
			screenToModel(instance.inverseTransformMatrix, float4(pViewParams.screenDimensions.x - offset, offset, -1.0f, 1.0f)).xyz,
			screenToModel(instance.inverseTransformMatrix, float4(offset, pViewParams.screenDimensions.y - offset, -1.0f, 1.0f)).xyz,
			screenToModel(instance.inverseTransformMatrix, float4(pViewParams.screenDimensions - float2(offset, offset), -1.0f, 1.0f)).xyz
		};
		viewFrustum.sides[0] = computePlane(origin, corners[2], corners[0]);
		viewFrustum.sides[1] = computePlane(origin, corners[1], corners[3]);
		viewFrustum.sides[2] = computePlane(origin, corners[0], corners[1]);
		viewFrustum.sides[3] = computePlane(origin, corners[3], corners[2]);
        //meshVisible = isBoxVisible(mesh.bounding);
        meshVisible = true;
    }
    GroupMemoryBarrierWithGroupSync();
    if(!meshVisible)
    {
        return;
    }
    for (uint i = threadID; i < mesh.numMeshlets; i += TASK_GROUP_SIZE)
    {
        uint m = p.meshletOffset + i;
        uint cull = p.cullingOffset + i;
        MeshletDescription meshlet = pScene.meshletInfos[m];
        MeshletCullingInfo culling = pScene.cullingInfos[cull];
        // if any triangle was visible last frame, it was drawn by the cached pass already
        if(!culling.wasVisible())
        {
            // if the meshlet is outside of the frustum, we skip it since we cant do depth culling anyways
		    if(meshlet.bounding.insideFrustum(viewFrustum))
            {
#ifdef DEPTH_CULLING
                // if the meshlet bounding box is behind the cached depth buffer, we skip
                //if(isBoxVisible(meshlet.bounding))
#endif
                {
                    uint index;
                    InterlockedAdd(head, 1, index);
                    p.culledMeshlets[index] = i;
                }

            }
        }
    }
    GroupMemoryBarrierWithGroupSync();
    DispatchMesh(head, 1, 1, p);
}
