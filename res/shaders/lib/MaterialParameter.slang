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
    float3 normal_WS;
	float3 position_WS;
	float3 viewDir_WS;
};

// data passed to fragment shader
struct FragmentParameter
{
    float4 position_CS : SV_Position;
    float3 cameraPos_WS: POSITION0;
	float3 normal_WS : NORMAL0;
	float3 tangent_WS : TANGENT0;
	float3 biTangent_WS : TANGENT1;
	float3 position_WS : POSITION2;
	float3 vertexColor : COLOR0;
	float2 texCoords[MAX_TEXCOORDS] : TEXCOORD0;
	MaterialParameter getMaterialParameter()
	{
		MaterialParameter result;
		result.position_WS = position_WS;
		result.vertexColor = vertexColor;
		for(uint i = 0; i < MAX_TEXCOORDS; ++i)
		{
			result.texCoords[i] = texCoords[i];
		}
		return result;
	}
	LightingParameter getLightingParameter()
	{
		LightingParameter result;
		result.tbn = float3x3(normalize(tangent_WS), normalize(biTangent_WS), normalize(normal_WS));
		result.position_WS = position_WS;
        result.viewDir_WS = normalize(cameraPos_WS - position_WS);
        result.normal_WS = normal_WS;
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
	float3 normal_MS;
	float3 tangent_MS;
	float3 biTangent_MS;
	float3 vertexColor;
	float2 texCoords[MAX_TEXCOORDS];
	FragmentParameter getParameter(float4x4 transformMatrix)
	{
		float4 modelPos = float4(position_MS, 1);
		float4 worldPos = mul(transformMatrix, modelPos);
		float4 viewPos = mul(pViewParams.viewMatrix, worldPos);
		float4 clipPos = mul(pViewParams.projectionMatrix, viewPos);
		FragmentParameter result;
		result.position_CS = clipPos;
		float3x3 normalMatrix = float3x3(transformMatrix);
		float3 T = mul(normalMatrix, tangent_MS);
		float3 N = mul(normalMatrix, normal_MS);
		float3 B = mul(normalMatrix, biTangent_MS);
		result.cameraPos_WS = pViewParams.cameraPos_WS.xyz;
		result.normal_WS = N;
		result.tangent_WS = T;
		result.biTangent_WS = B;
		result.position_WS = worldPos.xyz;
		result.vertexColor = vertexColor;
		for(uint i = 0; i < MAX_TEXCOORDS; ++i)
		{
			result.texCoords[i] = texCoords[i];
		}
		return result;
	}
};
