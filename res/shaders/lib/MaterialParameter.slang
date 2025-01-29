import Common;

struct MaterialParameter
{
	float3 position_WS;
	float2 texCoords[MAX_TEXCOORDS];
	float3 vertexColor;
};

struct LightingParameter
{
    float3x3 tangentToWorld;
    float3 viewDir_WS;
    float3 position_WS;
}


float3x3 qTangentToMatrix(float4 q){  
    q = normalize(q);
    float qx2 = q.x + q.x;
    float qy2 = q.y + q.y;
    float qz2 = q.z + q.z;
    float qxqx2 = q.x * qx2;
    float qxqy2 = q.x * qy2;
    float qxqz2 = q.x * qz2;
    float qxqw2 = q.w * qx2;
    float qyqy2 = q.y * qy2;
    float qyqz2 = q.y * qz2;
    float qyqw2 = q.w * qy2;
    float qzqz2 = q.z * qz2;
    float qzqw2 = q.w * qz2;
    float3x3 m = float3x3(1.0 - (qyqy2 + qzqz2), qxqy2 + qzqw2, qxqz2 - qyqw2,
                qxqy2 - qzqw2, 1.0 - (qxqx2 + qzqz2), qyqz2 + qxqw2,
                qxqz2 + qyqw2, qyqz2 - qxqw2, 1.0 - (qxqx2 + qyqy2));
    m[2] = normalize(cross(m[0], m[1])) * ((q.w < 0.0) ? -1.0 : 1.0);
    return m;
}

float4 qTangentSlerp(float4 q0, float4 q1, float t){
    float co = dot(q0, q1), so, s0, s1, s2 = 1.0, Omega;
    if(co < 0.0){
        co = -co;
        s2 = -s2;
    }
    if((1.0 - co) > 1e-8){
        Omega = acos(co);
        so = sin(Omega);
        s0 = sin((1.0 - t) * Omega) / so;
        s1 = sin(t * Omega) / so;
    } else {
        s0 = 1.0 - t;
        s1 = t;
    }
    float4 r = ((q0 * s0) + (q1 * (s1 * s2)));
    return r * ((((q0.w < 0.0) || (q1.w < 0.0)) != (r.w < 0.0)) ? -1.0 : 1.0);
}

// data passed to fragment shader
struct FragmentParameter
{
    float4 position_CS : SV_Position;
#ifndef POS_ONLY
    float3 normal_WS : NORMALWS;
    float3 tangent_WS : TANGENTWS;
    float3 biTangent_WS : BITANGENTWS;
	float3 position_WS : POSITIONWS;
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
        LightingParameter result;
        result.tangentToWorld = getTangentToWorld();
        result.viewDir_WS = normalize(pViewParams.cameraPosition_WS.xyz - position_WS);
        result.position_WS = position_WS;
        return result;
    }

    float3x3 getTangentToWorld()
    {
        // in theory, transposing would make this a world-to-tangent matrix, but because we are building the matrix ourselves
        // and something with matrix layouts is working the opposite direction, we need to transpose here
        return transpose(float3x3(tangent_WS, biTangent_WS, normal_WS));
    }
#endif
    static FragmentParameter interpolate(FragmentParameter f0, FragmentParameter f1, FragmentParameter f2, float3 barycentricCoords)
    {
        FragmentParameter result;
        result.position_CS = f0.position_CS * barycentricCoords.x + f1.position_CS * barycentricCoords.y + f2.position_CS * barycentricCoords.z;
#ifndef POS_ONLY
        result.normal_WS    = f0.normal_WS    * barycentricCoords.x + f1.normal_WS    * barycentricCoords.y + f2.normal_WS    * barycentricCoords.z;
        result.tangent_WS   = f0.tangent_WS   * barycentricCoords.x + f1.tangent_WS   * barycentricCoords.y + f2.tangent_WS   * barycentricCoords.z;
        result.biTangent_WS = f0.biTangent_WS * barycentricCoords.x + f1.biTangent_WS * barycentricCoords.y + f2.biTangent_WS * barycentricCoords.z;
        result.position_WS  = f0.position_WS  * barycentricCoords.x + f1.position_WS  * barycentricCoords.y + f2.position_WS  * barycentricCoords.z;
	    result.vertexColor  = f0.vertexColor  * barycentricCoords.x + f1.vertexColor  * barycentricCoords.y + f2.vertexColor  * barycentricCoords.z;
        result.texCoords0   = f0.texCoords0   * barycentricCoords.x + f1.texCoords0   * barycentricCoords.y + f2.texCoords0   * barycentricCoords.z;
        result.texCoords1   = f0.texCoords1   * barycentricCoords.x + f1.texCoords1   * barycentricCoords.y + f2.texCoords1   * barycentricCoords.z;
        result.texCoords2   = f0.texCoords2   * barycentricCoords.x + f1.texCoords2   * barycentricCoords.y + f2.texCoords2   * barycentricCoords.z;
        result.texCoords3   = f0.texCoords3   * barycentricCoords.x + f1.texCoords3   * barycentricCoords.y + f2.texCoords3   * barycentricCoords.z;
#endif
        return result;
    }
};

// data retrieved from VertexData
struct VertexAttributes
{
	float3 position_MS;
#ifndef POS_ONLY
	//float4 qTangent;
    float3 normal_MS;
    float3 tangent_MS;
    float3 biTangent_MS;
	float3 vertexColor;
	float2 texCoords[MAX_TEXCOORDS];
#endif
	FragmentParameter getParameter(float4x4 transformMatrix, float4x4 inverseTransformMatrix)
	{
		float4 modelPos = float4(position_MS, 1);
		float4 worldPos = mul(transformMatrix, modelPos);
		float4 viewPos = mul(pViewParams.viewMatrix, worldPos);
		float4 clipPos = mul(pViewParams.projectionMatrix, viewPos);
		FragmentParameter result;
		result.position_CS = clipPos;
#ifndef POS_ONLY
        //float3x3 tbn = qTangentToMatrix(qTangent);
        result.tangent_WS = normalize(mul(transformMatrix, float4(tangent_MS, 0)).xyz);
        result.biTangent_WS = normalize(mul(transformMatrix, float4(biTangent_MS, 0)).xyz);
		result.normal_WS = normalize(mul(transformMatrix, float4(normal_MS, 0)).xyz);
		result.vertexColor = vertexColor;
		result.position_WS = worldPos.xyz;
        result.texCoords0 = float4(texCoords[0], texCoords[1]);
        result.texCoords1 = float4(texCoords[2], texCoords[3]);
        result.texCoords2 = float4(texCoords[4], texCoords[5]);
        result.texCoords3 = float4(texCoords[6], texCoords[7]);
#endif
		return result;
	}
};
