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
	if(in.dispatchThreadID.x < pDispatchParams.numThreads.x && in.dispatchThreadID.y < pDispatchParams.numThreads.y)
	{
		uint index = in.dispatchThreadID.x + (in.dispatchThreadID.y * pDispatchParams.numThreads.x);
		//pDispatchParams.frustums[index] = frustum;
	}
}
