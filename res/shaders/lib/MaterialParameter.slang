import Common;

struct MaterialParameter
{
	float3 position_TS;
	float3 position_WS;
	float2 texCoords;
	float3 vertexColor;
};

// data used by light environment
struct LightingParameter
{
	// world to tangent space
	float3x3 tbn;
	float3 position_TS;
	float3 viewDir_TS;
};

// data passed to fragment shader
struct FragmentParameter
{
	float4 position_CS : SV_Position;
	float3 position_TS : POSITION0;
	float3 viewDir_TS : POSITION1;
	float3 normal_WS : NORMAL0;
	float3 tangent_WS : TANGENT0;
	float3 biTangent_WS : TANGENT1;
	float3 position_WS : POSITION2;
	float2 texCoords : TEXCOORD0;
	float3 vertexColor : COLOR0;
	MaterialParameter getMaterialParameter()
	{
		MaterialParameter result;
		result.position_TS = position_TS;
		result.position_WS = position_WS;
		result.texCoords = texCoords;
		result.vertexColor = vertexColor;
		return result;
	}
	LightingParameter getLightingParameter()
	{
		LightingParameter result;
		result.tbn = transpose(float3x3(normalize(tangent_WS), normalize(biTangent_WS), normalize(normal_WS)));
		result.position_TS = position_TS;
		result.viewDir_TS = viewDir_TS;
		return result;
	}
};

// data retrieved from VertexData
struct VertexAttributes
{
	float3 position_MS;
	float3 normal_MS;
	float3 tangent_MS;
	float3 biTangent_MS;
	float2 texCoords;
	float3 vertexColor;
	FragmentParameter getParameter(float4x4 transformMatrix)
	{
		float4 modelPos = float4(position_MS, 1);
		float4 worldPos = mul(transformMatrix, modelPos);
		float4 viewPos = mul(pViewParams.viewMatrix, worldPos);
		float4 clipPos = mul(pViewParams.projectionMatrix, viewPos);
		float3 tangent_WS = mul(transformMatrix, float4(normalize(tangent_MS), 0)).xyz;
		float3 biTangent_WS = mul(transformMatrix, float4(normalize(biTangent_MS), 0)).xyz;
		float3 normal_WS = mul(transformMatrix, float4(normalize(normal_MS), 0)).xyz;
		// Transforms from world space into tangent space
		float3x3 tbn = transpose(float3x3(tangent_WS, biTangent_WS, normal_WS));
		FragmentParameter result;
		result.position_TS = mul(tbn, worldPos.xyz);
		result.viewDir_TS = mul(tbn, pViewParams.cameraPos_WS.xyz - worldPos.xyz);
		result.normal_WS = normal_WS;
		result.tangent_WS = tangent_WS;
		result.biTangent_WS = biTangent_WS;
		result.position_WS = worldPos.xyz;
		result.position_CS = clipPos;
		result.texCoords = texCoords;
		result.vertexColor = vertexColor;
		return result;
	}
};
