import Common;

struct MaterialParameter
{
	float3 position_WS;
	float2 texCoords[MAX_TEXCOORDS];
	float3 vertexColor;
};

// data used by light environment
struct LightingParameter
{
    float3x3 tbn;
    float3 normal_TS;
	float3 position_TS;
	float3 viewDir_TS;
};

// data passed to fragment shader
struct FragmentParameter
{
    float4 position_CS : SV_Position;
#ifndef POS_ONLY
	float3 normal_WS : NORMALWS;
	float3 tangent_WS : TANGENTWS;
	float3 biTangent_WS : BITANGENTWS;
	float3 position_WS : POSITIONWS;
	float3 vertexColor : VERTEXCOLOR;
	float4 texCoords0 : TEXCOORDS0;
	float4 texCoords1 : TEXCOORDS1;
	float4 texCoords2 : TEXCOORDS2;
	float4 texCoords3 : TEXCOORDS3;
	MaterialParameter getMaterialParameter()
	{
		MaterialParameter result;
		result.position_WS = position_WS;
		result.vertexColor = vertexColor;
        result.texCoords[0] = texCoords0.xy;
        result.texCoords[1] = texCoords0.zw;
        result.texCoords[2] = texCoords1.xy;
        result.texCoords[3] = texCoords1.zw;
        result.texCoords[4] = texCoords2.xy;
        result.texCoords[5] = texCoords2.zw;
        result.texCoords[6] = texCoords3.xy;
        result.texCoords[7] = texCoords3.zw;
		return result;
	}
	LightingParameter getLightingParameter()
	{
		LightingParameter result;
		float3x3 tbn = float3x3(normalize(tangent_WS), normalize(biTangent_WS), normalize(normal_WS));
		result.tbn = tbn;
        result.position_TS = mul(tbn, position_WS);
        result.viewDir_TS = mul(tbn, normalize(pViewParams.c.cameraPos_WS.xyz - position_WS));
        result.normal_TS = mul(tbn, normal_WS);
		return result;
	}
    float3x3 getTangentToWorld()
    {
        float3x3 tbn = float3x3(normalize(tangent_WS), normalize(biTangent_WS), normalize(normal_WS));
		return transpose(tbn);
    }
#endif
    static FragmentParameter interpolate(FragmentParameter f0, FragmentParameter f1, FragmentParameter f2, float3 barycentricCoords)
    {
        FragmentParameter result;
        result.position_CS = f0.position_CS * barycentricCoords.x + f1.position_CS * barycentricCoords.y + f2.position_CS * barycentricCoords.z;
#ifndef POS_ONLY
        result.normal_WS = f0.normal_WS * barycentricCoords.x + f1.normal_WS * barycentricCoords.y + f2.normal_WS * barycentricCoords.z;
	    result.tangent_WS = f0.tangent_WS * barycentricCoords.x + f1.tangent_WS * barycentricCoords.y + f2.tangent_WS * barycentricCoords.z;
	    result.biTangent_WS = f0.biTangent_WS * barycentricCoords.x + f1.biTangent_WS * barycentricCoords.y + f2.biTangent_WS * barycentricCoords.z;
	    result.position_WS = f0.position_WS * barycentricCoords.x + f1.position_WS * barycentricCoords.y + f2.position_WS * barycentricCoords.z;
	    result.vertexColor = f0.vertexColor * barycentricCoords.x + f1.vertexColor * barycentricCoords.y + f2.vertexColor * barycentricCoords.z;
	    //for(uint i = 0; i < MAX_TEXCOORDS; ++i)
        //{
        //    result.texCoords[i] = f0.texCoords[i] * barycentricCoords.x + f1.texCoords[i] * barycentricCoords.y + f2.texCoords[i] * barycentricCoords.z;
        //}
#endif
        return result;
    }
};

// data passed to visibility render
struct VisibilityParameter
{
    uint32_t triangleIndex : POSITION5;
    uint32_t meshletId : POSITION6;
};

// data retrieved from VertexData
struct VertexAttributes
{
	float3 position_MS;
#ifndef POS_ONLY
	float3 normal_MS;
	float3 tangent_MS;
	float3 biTangent_MS;
	float3 vertexColor;
	float2 texCoords[MAX_TEXCOORDS];
#endif
	FragmentParameter getParameter(float4x4 transformMatrix)
	{
		float4 modelPos = float4(position_MS, 1);
		float4 worldPos = mul(transformMatrix, modelPos);
		float4 viewPos = mul(pViewParams.c.viewMatrix, worldPos);
		float4 clipPos = mul(pViewParams.c.projectionMatrix, viewPos);
		FragmentParameter result;
		result.position_CS = clipPos;
#ifndef POS_ONLY
		float3x3 normalMatrix = float3x3(transformMatrix);
		float3 T = mul(normalMatrix, tangent_MS);
		float3 N = mul(normalMatrix, normal_MS);
		float3 B = mul(normalMatrix, biTangent_MS);
		result.normal_WS = N;
		result.tangent_WS = T;
		result.biTangent_WS = B;
		result.position_WS = worldPos.xyz;
		result.vertexColor = vertexColor;
        result.texCoords0 = float4(texCoords[0], texCoords[1]);
        result.texCoords1 = float4(texCoords[2], texCoords[3]);
        result.texCoords2 = float4(texCoords[4], texCoords[5]);
        result.texCoords3 = float4(texCoords[6], texCoords[7]);
#endif
		return result;
	}
};
