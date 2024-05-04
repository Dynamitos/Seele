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
    bool insideFrustum(Frustum frustum)
    {
        bool result = true;
        for(int i = 0; i < 4 && result; ++i)
        {
            if(dot(frustum.sides[i].n, centerRadius.xyz) - frustum.sides[i].d < -getRadius())
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
	bool insideFrustum(Frustum frustum)
    {
		float3 corners[8];
        corners[0] = float3(min.x, min.y, min.z);
        corners[1] = float3(min.x, min.y, max.z);
        corners[2] = float3(min.x, max.y, min.z);
        corners[3] = float3(min.x, max.y, max.z);
        corners[4] = float3(max.x, min.y, min.z);
        corners[5] = float3(max.x, min.y, max.z);
        corners[6] = float3(max.x, max.y, min.z);
        corners[7] = float3(max.x, max.y, max.z);
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
