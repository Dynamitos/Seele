import Common;

struct DispatchParams
{
	uint4 numThreadGroups;
    uint4 numThreads;
    RWStructuredBuffer<Frustum> frustums;
};

ParameterBlock<DispatchParams> pDispatchParams;