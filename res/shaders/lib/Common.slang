const static float PI = 3.1415926535897932f;
const static uint MAX_PARTICLES = 65536;
const static uint BLOCK_SIZE = 32;

struct ViewParameter
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4 cameraPos_WS;
    float2 screenDimensions;
}
//layout(set = INDEX_VIEW_PARAMS, binding = 0, std430)
ParameterBlock<ViewParameter> viewParams;

 
// Convert screen space coordinates to view space.
float4 screenToClip( float4 screen )
{
    // Convert to normalized texture coordinates
    float2 texCoord = screen.xy / viewParams.screenDimensions;
 
    // Convert to clip space 
    return float4( float2( texCoord.x, 1.0f-texCoord.y ) * 2.0f - 1.0f, screen.z, screen.w );
}

struct Plane
{
    float3 n;
    float d;
};

struct Frustum
{
    Plane planes[4];
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
