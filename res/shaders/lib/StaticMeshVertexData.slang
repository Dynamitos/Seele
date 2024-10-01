import Common;
import VertexData;
import MaterialParameter;

struct StaticMeshVertexData
{
    float uint16ToFloat(uint16_t value)
    {
        return value / 65535.0f;
    }
    float3 xAxis( float4 qQuat )
    {
        float fTy  = 2.0 * qQuat.y;
        float fTz  = 2.0 * qQuat.z;
        float fTwy = fTy * qQuat.w;
        float fTwz = fTz * qQuat.w;
        float fTxy = fTy * qQuat.x;
        float fTxz = fTz * qQuat.x;
        float fTyy = fTy * qQuat.y;
        float fTzz = fTz * qQuat.z;
    
        return float3( 1.0-(fTyy+fTzz), fTxy+fTwz, fTxz-fTwy );
    }
    
    float3 yAxis( float4 qQuat )
    {
        float fTx  = 2.0 * qQuat.x;
        float fTy  = 2.0 * qQuat.y;
        float fTz  = 2.0 * qQuat.z;
        float fTwx = fTx * qQuat.w;
        float fTwz = fTz * qQuat.w;
        float fTxx = fTx * qQuat.x;
        float fTxy = fTy * qQuat.x;
        float fTyz = fTz * qQuat.y;
        float fTzz = fTz * qQuat.z;
    
        return float3( fTxy-fTwz, 1.0-(fTxx+fTzz), fTyz+fTwx );
    }

	VertexAttributes getAttributes(uint index)
	{
		VertexAttributes attributes;
		attributes.position_MS = float3(positions[index * 3 + 0], positions[index * 3 + 1], positions[index * 3 + 2]);
#ifndef POS_ONLY
        float4 qtangent = normalize(qtangents[index]);
        attributes.normal_MS = xAxis(qtangent);
        attributes.tangent_MS = yAxis(qtangent);
        float biNormalReflection = sign(qtangents[index].w);
        attributes.biTangent_MS = cross(attributes.normal_MS, attributes.tangent_MS) * biNormalReflection;
		for(uint i = 0; i < MAX_TEXCOORDS; ++i)
		{
			attributes.texCoords[i] = float2(uint16ToFloat(texCoords[i][index * 2 + 0]), uint16ToFloat(texCoords[i][index * 2 + 1]));
		}
		attributes.vertexColor = float3(uint16ToFloat(color[index * 3 + 0]), uint16ToFloat(color[index * 3 + 1]) , uint16ToFloat(color[index * 3 + 2]));
#endif
		return attributes;
	}
	StructuredBuffer<float> positions;
	StructuredBuffer<float4> qtangents;
	StructuredBuffer<uint16_t> color;
	StructuredBuffer<uint16_t> texCoords[MAX_TEXCOORDS];
};
layout(set=1)
ParameterBlock<StaticMeshVertexData> pVertexData;
