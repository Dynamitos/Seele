import Common;
import VertexData;
import MaterialParameter;

struct StaticMeshVertexData
{
    float uint16ToFloat(uint16_t value)
    {
        return value / 65535.0f;
    }

    float3x3 decodeQTangentUI32(uint v){
        float4 q = float4(((float3(int3(uint3((uint3(v) >> uint3(0u, 10u, 20u)) & uint2(0x3ffu, 0x1ffu).xxy)) - int2(512, 256).xxy)) / float2(511.0, 255.0).xxy) * 0.7071067811865475, 0.0);
        q.w = sqrt(1.0 - clamp(dot(q.xyz, q.xyz), 0.0, 1.0)); 
        q = normalize(float4[4](q.wxyz, q.xwyz, q.xywz, q.xyzw)[uint((v >> 30u) & 0x3u)]);
        float3 t2 = q.xyz * 2.0, tx = q.xxx * t2.xyz, ty = q.yyy * t2.xyz, tz = q.www * t2.xyz;
        float3 tangent = float3(1.0 - (ty.y + (q.z * t2.z)), tx.y + tz.z, tx.z - tz.y);
        float3 normal = float3(tx.z + tz.y, ty.z - tz.x, 1.0 - (tx.x + ty.y));
        return float3x3(tangent, cross(tangent, normal) * (((v & (1u << 29u)) != 0u) ? -1.0 : 1.0), normal);
    }

    // Decodes the UI32 encoded qtangent into a unpacked qtangent for further processing like vertex interpolation and so on
    float4 decodeQTangentUI32Raw(uint v){
        float4 q = float4(((float3(int3(uint3((uint3(v) >> uint3(0u, 10u, 20u)) & uint2(0x3ffu, 0x1ffu).xxy)) - int2(512, 256).xxy)) / float2(511.0, 255.0).xxy) * 0.7071067811865475, 0.0);
        q.w = sqrt(1.0 - clamp(dot(q.xyz, q.xyz), 0.0, 1.0)); 
        return normalize(float4[4](q.wxyz, q.xwyz, q.xywz, q.xyzw)[uint((v >> 30u) & 0x3u)]) * (((v & (1u << 29u)) != 0u) ? -1.0 : 1.0);
    }

	VertexAttributes getAttributes(uint index)
	{
		VertexAttributes attributes;
		attributes.position_MS = float3(positions[index * 3 + 0], positions[index * 3 + 1], positions[index * 3 + 2]);
#ifndef POS_ONLY
        attributes.qTangent = decodeQTangentUI32Raw(qtangents[index]);
		for(uint i = 0; i < MAX_TEXCOORDS; ++i)
		{
			attributes.texCoords[i] = float2(uint16ToFloat(texCoords[i][index * 2 + 0]), uint16ToFloat(texCoords[i][index * 2 + 1]));
		}
		attributes.vertexColor = float3(uint16ToFloat(color[index * 3 + 0]), uint16ToFloat(color[index * 3 + 1]) , uint16ToFloat(color[index * 3 + 2]));
#endif
		return attributes;
	}
	StructuredBuffer<float> positions;
	StructuredBuffer<uint> qtangents;
	StructuredBuffer<uint16_t> color;
	StructuredBuffer<uint16_t> texCoords[MAX_TEXCOORDS];
};
layout(set=1)
ParameterBlock<StaticMeshVertexData> pVertexData;
