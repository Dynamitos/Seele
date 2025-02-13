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
            if(dot(frustum.sides[i].getNormal(), getCenter()) - frustum.sides[i].getDistance() < -getRadius())
            {
                result = false;
            }
        }
        return result;
    }
};

struct AABB
{
	float3 minCorner;
    float pad0;
	float3 maxCorner;
    float pad1;
    // modified version from https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
	bool insideFrustum(Frustum frustum)
    {
        float3 center = (minCorner + maxCorner) * 0.5;
        float3 extents = float3(maxCorner.x - center.x, maxCorner.y - center.y, maxCorner.z - center.z);
        bool result = true;
        for(int i = 0; i < 4 && result; ++i)
        {
            const float r = extents.x * abs(frustum.sides[i].getNormal().x) 
                          + extents.y * abs(frustum.sides[i].getNormal().y) 
                          + extents.z * abs(frustum.sides[i].getNormal().z);

            const float signedDistance = dot(frustum.sides[i].getNormal(), center) - frustum.sides[i].getDistance();
            if(signedDistance < -r)
            {
                result = false;
            }
        }
        return result;
	}
    float projectScreenDepth(float4x4 mvp, inout int2 screenCornerMin, inout int2 screenCornerMax)
    {
        float maxDepth = 0;
        float4 corners[8];
        corners[0] = float4(minCorner.x, minCorner.y, minCorner.z, 1.0f);
        corners[1] = float4(minCorner.x, minCorner.y, maxCorner.z, 1.0f);
        corners[2] = float4(minCorner.x, maxCorner.y, minCorner.z, 1.0f);
        corners[3] = float4(minCorner.x, maxCorner.y, maxCorner.z, 1.0f);
        corners[4] = float4(maxCorner.x, minCorner.y, minCorner.z, 1.0f);
        corners[5] = float4(maxCorner.x, minCorner.y, maxCorner.z, 1.0f);
        corners[6] = float4(maxCorner.x, maxCorner.y, minCorner.z, 1.0f);
        corners[7] = float4(maxCorner.x, maxCorner.y, maxCorner.z, 1.0f);
        for(uint i = 0; i < 8; ++i)
        {
            float4 clipCorner = mul(mvp, corners[i]);
            float4 screenCorner = clipToScreen(clipCorner);
            int2 screenCoords = int2(clamp(int(screenCorner.x), 0, int(pViewParams.screenDimensions.x)), clamp(int(screenCorner.y), 0, int(pViewParams.screenDimensions.y)));
            screenCornerMin = int2(min(screenCornerMin.x, screenCoords.x), min(screenCornerMin.y, screenCoords.y));
            screenCornerMax = int2(max(screenCornerMax.x, screenCoords.x), max(screenCornerMax.y, screenCoords.y));
            maxDepth = max(maxDepth, screenCorner.z);
        }
        return maxDepth;
    }
};
