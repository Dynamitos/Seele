import VertexData;

struct MaterialParameter
{
	float3 worldPosition;
	float2 texCoords;
	float3 normal;
	float3 tangent;
	float3 biTangent;

	static MaterialParameter create(VertexAttributes attrib)
	{
		MaterialParameter result;
		result.worldPosition = attrib.getWorldPosition().xyz;
		result.texCoords = attrib.getTexCoords();
		result.normal = attrib.getNormal();
		result.tangent = attrib.getTangent();
		result.biTangent = attrib.getBiTangent();
		return result;
	}
}

