import Common;

struct AABB
{
	float3 min;
    float pad0;
	float3 max;
    float pad1;
	bool insideFrustum(float4x4 transform, Frustum frustum)
	{
		float4 corners[8];
        corners[0] = mul(transform, float4(min.x, min.y, min.z, 1.0f));
        corners[1] = mul(transform, float4(min.x, min.y, max.z, 1.0f));
        corners[2] = mul(transform, float4(min.x, max.y, min.z, 1.0f));
        corners[3] = mul(transform, float4(min.x, max.y, max.z, 1.0f));
        corners[4] = mul(transform, float4(max.x, min.y, min.z, 1.0f));
        corners[5] = mul(transform, float4(max.x, min.y, max.z, 1.0f));
        corners[6] = mul(transform, float4(max.x, max.y, min.z, 1.0f));
        corners[7] = mul(transform, float4(max.x, max.y, max.z, 1.0f));
        for(int i = 0; i < 8; ++i)
        {
            float3 adjusted = corners[i].xyz / corners[i].w;
            if(adjusted.z > 0)
            {
                if(frustum.pointInside(adjusted))
                {
                    return true;
                }
            }
        }
        return false;
	}
};
