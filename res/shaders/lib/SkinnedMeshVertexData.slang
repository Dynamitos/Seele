import VertexData;
import Common;
import Scene;
import MaterialParameter;

struct SkinnedMeshVertexData
{
	VertexInput getAttributes(uint index, float4x4 transform)
	{
		StaticMeshVertexAttributes attr;
		float4x4 boneTransform = float4x4(1.0f);
		for(int i = 0; i < 4; ++i)
		{
			boneTransform += bones[boneIndices[i]] * boneWeights[i];
		}
		float4 localPos = mul(boneTransform, float4(positions[index], 1));
		float4 worldPos = mul(transform, localPos);
		float4 viewPos = mul(viewParams.viewMatrix, worldPos);
		float4 clipPos = mul(viewParams.projectionMatrix, viewPos);
		attr.clipPosition = clipPos;
		attr.worldPosition = worldPos.xyz;
		attr.texCoords = texCoords[index];
		attr.normal = normals[index];
		attr.tangent = tangents[index];
		attr.biTangent = biTangents[index];
		return attr;
	}
	StructuredBuffer<float3> positions;
	StructuredBuffer<float2> texCoords;
	StructuredBuffer<float3> normals;
	StructuredBuffer<float3> tangents;
	StructuredBuffer<float3> biTangents;
	StructuredBuffer<int4> boneIndices;
	StructuredBuffer<float4> boneWeights;
	StructuredBuffer<float4x4> bones;
}
