import Common;

struct DispatchParams
{
	uint3 numThreadGroups;
	uint pad0;
	uint3 numThreads;
	uint pad1;
    RWStructuredBuffer<Frustum> frustums;
};

ParameterBlock<DispatchParams> pDispatchParams;