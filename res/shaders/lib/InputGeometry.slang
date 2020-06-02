interface IVertexShaderInput
{
//internally, the vertex layout is stored in vec4 for faster access,
//but here we unwrap them for convenience
	float3 getVertexPosition();
	float2 getTexCoords();
	float3 getNormal();
	float3 getTangent();
	float3 getBiTangent();
};

struct PositionOnlyVertexInput : IVertexShaderInput
{
	float4 position;
	
	float3 getVertexPosition() { return position.xyz; }
	float2 getTexCoords() { return float2(0, 0); }
	float3 getNormal() { return float3(0, 1, 0); }
	float3 getTangent() { return float3(1, 0, 0); }
	float3 getBiTangent() { return float3(0, 0, 1); }
};

struct PositionAndNormalVertexInput : IVertexShaderInput
{
	float4 position;
	float4 normal;
	
	float3 getVertexPosition() { return position.xyz; }
	float2 getTexCoords() { return float2(0, 0); }
	float3 getNormal() { return normal.xyz; }
	float3 getTangent() { return float3(1, 0, 0); }
	float3 getBiTangent() { return float3(0, 0, 1); }
};

struct InputGeometry : IVertexShaderInput
{
	float4 position;
	float2 texCoord;
	float4 normal;
	float4 tangent;
	float4 bitangent;

	float3 getVertexPosition() { return position.xyz; }
	float2 getTexCoords() { return texCoord; }
	float3 getNormal() { return normal.xyz; }
	float3 getTangent() { return tangent.xyz; }
	float3 getBiTangent() { return bitangent.xyz; }
};

struct MaterialPixelParameter
{
    float3 position;
    float2 texCoord;
    float3 viewDir;
    float3 normal;
    float3 tangent;
    float3 biTangent;
	float4 clipPosition;
    float3 transformLocalToWorld(float3 input)
    {
		float3 unitNormal = normalize(normal);
        float3 unitTangent = normalize(tangent);
        unitTangent = normalize(unitTangent - dot(unitTangent, unitNormal) * unitNormal);
        float3 unitBitangent = cross(unitTangent, unitNormal);
		float3x3 tbn = float3x3(unitTangent, unitBitangent, unitNormal);
		float3 result = mul(tbn, input);
		result = normalize(result);
		return result;
	}
};