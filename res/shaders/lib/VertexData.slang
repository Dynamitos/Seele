import Scene;

interface VertexAttributes
{
	float4 getPosition();
	float2 getTexCoord();
	float3 getNormal();
}

interface VertexData
{
	VertexAttributes getAttributes(uint index, InstanceData inst);
};