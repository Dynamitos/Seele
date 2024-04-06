import Common;

struct BoundingSphere
{
    float4 centerRadius;
    float3 getCenter()
    {
        return centerRadius.xyz;
    }
    float getRadius()
    {
        return centerRadius.w;
    }
    bool insideFrustum(float4x4 transform, Frustum frustum)
    {
        float3 transformed = mul(transform, float4(getCenter(), 1)).xyz;
        float maxScale = max(max(transform[0][0], transform[1][1]), transform[2][2]);
        float scaledRange = getRadius() * maxScale;
        bool result = true;
        for(int i = 0; i < 4 && result; ++i)
        {
            if(dot(frustum.sides[i].n, transformed) - frustum.sides[i].d < -scaledRange)
            {
                result = false;
            }
        }
        return result;
    }
};

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
            if(frustum.pointInside(corners[i].xyz))
            {
                return true;
            }
        }
        return false;
	}
};
