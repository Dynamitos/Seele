import Common;

struct DispatchParams
{
	uint3 numThreadGroups;
	uint pad0;
	uint3 numThreads;
	uint pad1;
};

struct DispatchParamWrapper
{
    ParameterBlock<DispatchParams> p;
    RWStructuredBuffer<Frustum> frustums;
}
ParameterBlock<DispatchParamWrapper> pDispatchParams;