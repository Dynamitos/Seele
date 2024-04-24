const static float PI = 3.1415926535897932f;
const static uint MAX_PARTICLES = 65536;
const static uint BLOCK_SIZE = 32;

struct ViewParameter
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 inverseProjection;
    float4 cameraPos_WS;
    float2 screenDimensions;
}
layout(set=0)
ParameterBlock<ViewParameter> pViewParams;

float4 clipToView(float4 clip)
{
    float4 view = mul(pViewParams.inverseProjection, clip);

    view = view / view.w;

    return view;
}

float4 screenToView(float4 screen)
{
    float2 texCoord = screen.xy / pViewParams.screenDimensions;
 
    // Convert to clip space 
    float4 clip = float4( float2( texCoord.x, 1.0f-texCoord.y ) * 2.0f - 1.0f, screen.z, screen.w);

    return clipToView(clip);
}

struct Plane
{
    float3 n;
    float d;
    bool pointInside(float3 point)
    {
        return dot(n, point) - d > 0.0f;
    }
};

struct Frustum
{
    Plane sides[4];
    bool pointInside(float3 point)
    {
        for(int p = 0; p < 4; ++p)
        {
            if(!sides[p].pointInside(point))
            {
                return false;
            }
        }
        return true;
    }
};
Plane computePlane(float3 p0, float3 p1, float3 p2)
{
    Plane plane;

    float3 v0 = p1 - p0;
    float3 v2 = p2 - p0;

    plane.n = normalize(cross(v0, v2));

    plane.d = dot(plane.n, p0);

    return plane;
}
