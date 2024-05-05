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
    // modified version from https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
	bool insideFrustum(Frustum frustum)
    {
        float3 center = (min + max) * 0.5;
        float3 extents = float3(max.x - center.x, max.y - center.y, max.z - center.z);
        bool result = true;
        for(int i = 0; i < 4 && result; ++i)
        {
            const float r = extents.x * abs(frustum.sides[i].n.x) 
                          + extents.y * abs(frustum.sides[i].n.y) 
                          + extents.z * abs(frustum.sides[i].n.z);

            const float signedDistance = dot(frustum.sides[i].n, center) - frustum.sides[i].d;
            if(signedDistance < -r)
            {
                result = false;
            }
        }
        return result;
	}
};
