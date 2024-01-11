import Common;
import VertexData;
import MaterialParameter;

struct StaticMeshVertexData : IVertexData
{
	VertexAttributes getAttributes(uint index)
	{
		VertexAttributes attributes;
		attributes.position_MS = float3(positions[3 * index + 0], positions[3 * index + 1], positions[3 * index + 2]);
		attributes.vertexColor = float3(color[3 * index + 0], color[3 * index + 1], color[3 * index + 2]);
		return attributes;
	}
	StructuredBuffer<float> positions;
	StructuredBuffer<float> color;
};
