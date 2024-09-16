import Common;
import DispatchParams;

struct ComputeShaderInput
{
	uint3 groupID : SV_GroupID;
	uint3 groupThreadID : SV_GroupThreadID;
	uint3 dispatchThreadID : SV_DispatchThreadID;
	uint groupIndex : SV_GroupIndex;
};

[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
[shader("compute")]
void computeFrustums(ComputeShaderInput in)
{
	float3 origin = float3(0, 0, 0);
	float3 corners[4] = {
		screenToWorld(float4(float2(in.dispatchThreadID.x + 0, in.dispatchThreadID.y + 0) * BLOCK_SIZE, -1.0f, 1.0f)).xyz,
		screenToWorld(float4(float2(in.dispatchThreadID.x + 1, in.dispatchThreadID.y + 0) * BLOCK_SIZE, -1.0f, 1.0f)).xyz,
		screenToWorld(float4(float2(in.dispatchThreadID.x + 0, in.dispatchThreadID.y + 1) * BLOCK_SIZE, -1.0f, 1.0f)).xyz,
		screenToWorld(float4(float2(in.dispatchThreadID.x + 1, in.dispatchThreadID.y + 1) * BLOCK_SIZE, -1.0f, 1.0f)).xyz
	};
	Frustum frustum;
	frustum.sides[0] = computePlane(origin, corners[2], corners[0]);
	frustum.sides[1] = computePlane(origin, corners[1], corners[3]);
	frustum.sides[2] = computePlane(origin, corners[0], corners[1]);
	frustum.sides[3] = computePlane(origin, corners[3], corners[2]);
	if(in.dispatchThreadID.x < pDispatchParams.p.numThreads.x && in.dispatchThreadID.y < pDispatchParams.p.numThreads.y)
	{
		uint index = in.dispatchThreadID.x + (in.dispatchThreadID.y * pDispatchParams.p.numThreads.x);
		pDispatchParams.frustums[index] = frustum;
	}
}
