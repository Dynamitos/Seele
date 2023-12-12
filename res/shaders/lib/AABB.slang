import Common;

struct AABB
{
	float3 min;
    float pad0;
	float3 max;
    float pad1;
	bool insideFrustum(float4x4 transform, Frustum frustum)
	{
		float3 corners[8];
        corners[0] = mul(transform, float4(min.x, min.y, min.z, 1.0f)).xyz;
        corners[1] = mul(transform, float4(min.x, min.y, max.z, 1.0f)).xyz;
        corners[2] = mul(transform, float4(min.x, max.y, min.z, 1.0f)).xyz;
        corners[3] = mul(transform, float4(min.x, max.y, max.z, 1.0f)).xyz;
        corners[4] = mul(transform, float4(max.x, min.y, min.z, 1.0f)).xyz;
        corners[5] = mul(transform, float4(max.x, min.y, max.z, 1.0f)).xyz;
        corners[6] = mul(transform, float4(max.x, max.y, min.z, 1.0f)).xyz;
        corners[7] = mul(transform, float4(max.x, max.y, max.z, 1.0f)).xyz;
        for(int i = 0; i < 8; ++i)
        {
            if(frustum.pointInside(corners[i]))
            {
                return true;
            }
        }
        return false;
	}
};
