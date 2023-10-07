import VertexData;
import Common;
import Scene;

struct StaticMeshVertexAttributes : VertexAttributes
{
	float4 position: SV_Position;
	float2 texCoords: TEXCOORD0;
	float3 normal: NORMAL0;
	float4 getPosition()
	{
		return position;
	}
	float2 getTexCoord()
	{
		return texCoords;
	}
	float3 getNormal()
	{
		return normal;
	}
}

struct StaticMeshVertexData : VertexData
{
	StaticMeshVertexAttributes getAttributes(uint index, InstanceData inst)
	{
		StaticMeshVertexAttributes attr;
		float4 worldPos = mul(inst.transformMatrix, positions[index]);
		float4 viewPos = mul(viewParams.viewMatrix, worldPos);
		float4 clipPos = mul(viewParams.projectionMatrix, viewPos);
		attr.position = clipPos;
		attr.texCoords = texCoords[index];
		attr.normal = normals[index];
		return attr;
	}
	StructuredBuffer<float4> positions;
	StructuredBuffer<float2> texCoords;
	StructuredBuffer<float3> normals;
}
ParameterBlock<StaticMeshVertexData> vertexData;