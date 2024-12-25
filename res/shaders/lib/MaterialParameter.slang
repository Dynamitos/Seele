import Common;

struct MaterialParameter
{
	float3 position_WS;
	float2 texCoords[MAX_TEXCOORDS];
	float3 vertexColor;
};

struct LightingParameter
{
    float3x3 worldToTangent;
    float3 viewDir_TS;
    float3 position_TS;
}

// Constructs a TBN matrix from a unpacked qtangent for example for after vertex interpolation in the fragment shader
float3x3 constructTBNFromQTangent(float4 q){
    q = normalize(q); // Ensure that the quaternion is normalized in case it is not, for example after interpolation and so on 
    float3 t2 = q.xyz * 2.0, tx = q.xxx * t2.xyz, ty = q.yyy * t2.xyz, tz = q.www * t2.xyz;
    float3 tangent = float3(1.0 - (ty.y + (q.z * t2.z)), tx.y + tz.z, tx.z - tz.y);
    float3 normal = float3(tx.z + tz.y, ty.z - tz.x, 1.0 - (tx.x + ty.y));
    return float3x3(tangent, cross(tangent, normal) * ((q.w < 0.0) ? -1.0 : 1.0), normal);
}

// data passed to fragment shader
struct FragmentParameter
{
    float4 position_CS : SV_Position;
#ifndef POS_ONLY
	float4 qTangent : QTANGENT;
	float3 position_WS : POSITIONWS;
    float3 position_TS : POSITIONTS;
    float3 cameraPos_TS : CAMPOSTS;
	float3 vertexColor : COLOR;
	float4 texCoords0 : TEXCOORDS0;
	float4 texCoords1 : TEXCOORDS1;
	float4 texCoords2 : TEXCOORDS2;
	float4 texCoords3 : TEXCOORDS3;
    MaterialParameter getMaterialParameter()
    {
        MaterialParameter result;
        result.position_WS = position_WS;
        result.texCoords[0] = texCoords0.xy;
        result.texCoords[1] = texCoords0.zw;
        result.texCoords[2] = texCoords1.xy;
        result.texCoords[3] = texCoords1.zw;
        result.texCoords[4] = texCoords2.xy;
        result.texCoords[5] = texCoords2.zw;
        result.texCoords[6] = texCoords3.xy;
        result.texCoords[7] = texCoords3.zw;
        result.vertexColor = vertexColor;
        return result;
    }

    LightingParameter getLightingParameter()
    {
        float3x3 worldToTangent = transpose(constructTBNFromQTangent(qTangent));
        LightingParameter result;
        result.worldToTangent = worldToTangent;
        result.viewDir_TS = cameraPos_TS - position_TS;
        result.position_TS = position_TS;
        return result;
    }

    float3x3 getTangentToWorld()
    {
        return constructTBNFromQTangent(qTangent);
    }
#endif
    static FragmentParameter interpolate(FragmentParameter f0, FragmentParameter f1, FragmentParameter f2, float3 barycentricCoords)
    {
        FragmentParameter result;
        result.position_CS = f0.position_CS * barycentricCoords.x + f1.position_CS * barycentricCoords.y + f2.position_CS * barycentricCoords.z;
#ifndef POS_ONLY
        result.qTangent = f0.qTangent * barycentricCoords.x + f1.qTangent * barycentricCoords.y + f2.qTangent * barycentricCoords.z;
	    result.position_WS = f0.position_WS * barycentricCoords.x + f1.position_WS * barycentricCoords.y + f2.position_WS * barycentricCoords.z;
	    result.vertexColor = f0.vertexColor * barycentricCoords.x + f1.vertexColor * barycentricCoords.y + f2.vertexColor * barycentricCoords.z;
        result.texCoords0 = f0.texCoords0 * barycentricCoords.x + f1.texCoords0 * barycentricCoords.y + f2.texCoords0 * barycentricCoords.z;
        result.texCoords1 = f0.texCoords1 * barycentricCoords.x + f1.texCoords1 * barycentricCoords.y + f2.texCoords1 * barycentricCoords.z;
        result.texCoords2 = f0.texCoords2 * barycentricCoords.x + f1.texCoords2 * barycentricCoords.y + f2.texCoords2 * barycentricCoords.z;
        result.texCoords3 = f0.texCoords3 * barycentricCoords.x + f1.texCoords3 * barycentricCoords.y + f2.texCoords3 * barycentricCoords.z;

#endif
        return result;
    }
};

// data retrieved from VertexData
struct VertexAttributes
{
	float3 position_MS;
#ifndef POS_ONLY
	float4 qTangent;
	float3 vertexColor;
	float2 texCoords[MAX_TEXCOORDS];
#endif
	FragmentParameter getParameter(float4x4 transformMatrix)
	{
		float4 modelPos = float4(position_MS, 1);
		float4 worldPos = mul(transformMatrix, modelPos);
		float4 viewPos = mul(pViewParams.viewMatrix, worldPos);
		float4 clipPos = mul(pViewParams.projectionMatrix, viewPos);
		FragmentParameter result;
		result.position_CS = clipPos;
#ifndef POS_ONLY
		result.qTangent = qTangent;
        float3x3 tbn = constructTBNFromQTangent(qTangent);
        result.position_TS = mul(tbn, worldPos.xyz);
		result.vertexColor = vertexColor;
		result.position_WS = worldPos.xyz;
        result.cameraPos_TS = mul(tbn, pViewParams.cameraPosition_WS.xyz);
        result.texCoords0 = float4(texCoords[0], texCoords[1]);
        result.texCoords1 = float4(texCoords[2], texCoords[3]);
        result.texCoords2 = float4(texCoords[4], texCoords[5]);
        result.texCoords3 = float4(texCoords[6], texCoords[7]);
#endif
		return result;
	}
};
