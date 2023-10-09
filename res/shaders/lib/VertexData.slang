import Scene;

interface VertexAttributes
{
	float3 getWorldPosition();
	float2 getTexCoords();
	float3 getNormal();
	float3 getTangent();
	float3 getBiTangent();
}

interface VertexData
{
	VertexAttributes getAttributes(uint index, InstanceData inst);
};