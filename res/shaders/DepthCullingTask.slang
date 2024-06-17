import Common;
import Scene;

groupshared MeshPayload p;
groupshared uint head;
groupshared MeshData mesh;
groupshared InstanceData instance;
groupshared Frustum viewFrustum;
groupshared float4x4 modelViewProjection;

struct DepthData
{
    Texture2D<float> texture;
    RWStructuredBuffer<float> buffer;
}

ParameterBlock<DepthData> pDepthAttachment;

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
    }
    GroupMemoryBarrierWithGroupSync();
    for (uint i = threadID; i < mesh.numMeshlets; i += TASK_GROUP_SIZE)
    {
        uint m = p.meshletOffset + i;
        uint cull = p.cullingOffset + i;
        MeshletDescription meshlet = pScene.meshletInfos[m];
        MeshletCullingInfo culling = pScene.cullingInfos[cull];
        // if any triangle was visible last frame, it was drawn by the cached pass already
        if(!culling.anyVisible())
        {
            // if the meshlet is outside of the frustum, we skip it since we cant do depth culling anyways
		    if(meshlet.bounding.insideFrustum(viewFrustum))
            {
                uint2 mipDimensions = uint2((uint(pViewParams.screenDimensions.x) + BLOCK_SIZE - 1) / BLOCK_SIZE, (uint(pViewParams.screenDimensions.y) + BLOCK_SIZE - 1) / BLOCK_SIZE);
                // now we calculate what mip level we need to only sample up to 4 texels covering the entire meshlet
                uint2 screenCornerMin = mipDimensions;
                uint2 screenCornerMax = uint2(0, 0);
                // we use reverse depth, so higher values are closer
                float maxDepth = 0;
                {
                    float4 corners[8];
                    corners[0] = float4(meshlet.bounding.min.x, meshlet.bounding.min.y, meshlet.bounding.min.z, 1.0f);
                    corners[1] = float4(meshlet.bounding.min.x, meshlet.bounding.min.y, meshlet.bounding.max.z, 1.0f);
                    corners[2] = float4(meshlet.bounding.min.x, meshlet.bounding.max.y, meshlet.bounding.min.z, 1.0f);
                    corners[3] = float4(meshlet.bounding.min.x, meshlet.bounding.max.y, meshlet.bounding.max.z, 1.0f);
                    corners[4] = float4(meshlet.bounding.max.x, meshlet.bounding.min.y, meshlet.bounding.min.z, 1.0f);
                    corners[5] = float4(meshlet.bounding.max.x, meshlet.bounding.min.y, meshlet.bounding.max.z, 1.0f);
                    corners[6] = float4(meshlet.bounding.max.x, meshlet.bounding.max.y, meshlet.bounding.min.z, 1.0f);
                    corners[7] = float4(meshlet.bounding.max.x, meshlet.bounding.max.y, meshlet.bounding.max.z, 1.0f);
                    for(uint i = 0; i < 8; ++i)
                    {
                        float4 clipCorner = mul(modelViewProjection, corners[i]);
                        float4 screenCorner = clipToScreen(clipCorner) / BLOCK_SIZE;
                        screenCornerMin = uint2(min(screenCornerMin.x, uint(screenCorner.x)), min(screenCornerMin.y, uint(screenCorner.y)));
                        screenCornerMax = uint2(max(screenCornerMax.x, uint(screenCorner.x)), max(screenCornerMax.y, uint(screenCorner.y)));
                        maxDepth = max(maxDepth, screenCorner.z);
                    }
                }
                uint mipOffset = 0;
                // in theory this wouldnt work if no corner was in screen, as min would be greater that max, however we verified that with view culling
                while(screenCornerMax.x - screenCornerMin.x > 1 || screenCornerMax.y - screenCornerMin.y > 1)
                {
                    mipOffset += mipDimensions.x * mipDimensions.y;
                    mipDimensions = uint2(mipDimensions.x + 1, mipDimensions.y + 1) / 2;
                    screenCornerMin /= 2;
                    screenCornerMax /= 2;
                }

                // now we sample 4 texels from the depth at the calculated mip level, this should give us the screen extent of the meshlet
                float d1 = pDepthAttachment.buffer[mipOffset + (screenCornerMin.y * mipDimensions.x) + screenCornerMin.x];
                float d2 = pDepthAttachment.buffer[mipOffset + (screenCornerMin.y * mipDimensions.x) + screenCornerMax.x];
                float d3 = pDepthAttachment.buffer[mipOffset + (screenCornerMax.y * mipDimensions.x) + screenCornerMin.x];
                float d4 = pDepthAttachment.buffer[mipOffset + (screenCornerMax.y * mipDimensions.x) + screenCornerMax.x];

                // we want to check if the minimum depth (the value farthest away) is smaller than the maximum bounding box depth
                // otherwise, there is no way for the meshlet to be visible
                float d = min(min(d1, d2), min(d3, d4));
                
                // this is technically not correct, as the mipmap is generated with a linear filter, but we actually would need a min filter, but whatever
                if(d < maxDepth)
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
