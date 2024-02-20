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
	const float3 topLeft = float3(-1, -1, -1);
	const float3 topRight = float3(1, -1, -1);
	const float3 bottomLeft = float3(-1, 1, -1);
	const float3 origin = float3(0, 0, 0);
	float3 xStep = (topRight - topLeft) / pDispatchParams.numThreads.x;
	float3 yStep = (bottomLeft - topLeft) / pDispatchParams.numThreads.y;
	float3 corners[4] = {
		topLeft + (in.dispatchThreadID.x + 0) * xStep + (in.dispatchThreadID.y + 0) * yStep,
		topLeft + (in.dispatchThreadID.x + 1) * xStep + (in.dispatchThreadID.y + 0) * yStep,
		topLeft + (in.dispatchThreadID.x + 0) * xStep + (in.dispatchThreadID.y + 1) * yStep,
		topLeft + (in.dispatchThreadID.x + 1) * xStep + (in.dispatchThreadID.y + 1) * yStep
	};
	Frustum frustum;
	frustum.sides[0] = computePlane(origin, corners[0], corners[2]);
	frustum.sides[1] = computePlane(origin, corners[3], corners[1]);
	frustum.sides[2] = computePlane(origin, corners[1], corners[0]);
	frustum.sides[3] = computePlane(origin, corners[2], corners[3]);
	if(in.dispatchThreadID.x < pDispatchParams.numThreads.x && in.dispatchThreadID.y < pDispatchParams.numThreads.y)
	{
		uint index = in.dispatchThreadID.x + (in.dispatchThreadID.y * pDispatchParams.numThreads.x);
		pDispatchParams.frustums[index] = frustum;
	}
}
