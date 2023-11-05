import Common;

struct ComputeShaderInput
{
	uint3 groupID : SV_GroupID;
	uint3 groupThreadID : SV_GroupThreadID;
	uint3 dispatchThreadID : SV_DispatchThreadID;
	uint groupIndex : SV_GroupIndex;
};

struct DispatchParams
{
	uint3 numThreadGroups;
	uint pad0;
	uint3 numThreads;
	uint pad1;
	RWShaderBuffer<Frustum> frustums;
}
ParameterBuffer<DispatchParams> dispatchParams;


[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
[shader("compute")]
void computeFrustums(ComputeShaderInput in)
{
	const float3 eyePos = float3(0,0,0);

	float4 screenSpace[4];

	screenSpace[0] = float4(in.dispatchThreadID.xy * BLOCK_SIZE, -1.0f, 1.0f);
	screenSpace[1] = float4(float2(in.dispatchThreadID.x + 1, in.dispatchThreadID.y) * BLOCK_SIZE, -1.0f, 1.0f);
	screenSpace[2] = float4(float2(in.dispatchThreadID.x, in.dispatchThreadID.y + 1) * BLOCK_SIZE, -1.0f, 1.0f);
	screenSpace[3] = float4(float2(in.dispatchThreadID.x + 1, in.dispatchThreadID.y + 1) * BLOCK_SIZE, -1.0f, 1.0f);
	
	//Convert to viewSpace
	float3 viewSpace[4];
	
	for(int i = 0; i < 4; i++)
	{
		viewSpace[i] = screenToClip(screenSpace[i]).xyz;
	}

	//Compute frustum
	Frustum frustum;

	frustum.planes[0] = computePlane(eyePos, viewSpace[2], viewSpace[0]);
	frustum.planes[1] = computePlane(eyePos, viewSpace[1], viewSpace[3]);
	frustum.planes[2] = computePlane(eyePos, viewSpace[0], viewSpace[1]);
	frustum.planes[3] = computePlane(eyePos, viewSpace[3], viewSpace[2]);

	if(in.dispatchThreadID.x < dispatchParams.numThreads.x && in.dispatchThreadID.y < dispatchParams.numThreads.y)
	{
		uint index = in.dispatchThreadID.x + (in.dispatchThreadID.y * numThreads.x);
		dispatchParams.frustums[index] = frustum;
	}
}
