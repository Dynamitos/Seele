const static float PI = 3.1415926535897932f;
const static uint BLOCK_SIZE = 32;
static const uint64_t MAX_TEXCOORDS = 8;
static const uint32_t TASK_GROUP_SIZE = 32;
static const uint32_t MESH_GROUP_SIZE = 32;

struct ViewParameter
{
    float4x4 viewMatrix;
    float4x4 inverseViewMatrix;
    float4x4 projectionMatrix;
    float4x4 inverseProjection;
    float4 cameraPos_WS;
    float2 screenDimensions;
};
layout(set = 0)
ParameterBlock<ViewParameter> pViewParams;

float4 worldToModel(float4x4 inverseTransform, float4 world)
{
    float4 model = mul(inverseTransform, world);

    model = model / model.w;

    return model;
}

float4 viewToWorld(float4 view)
{
    float4 world = mul(pViewParams.inverseViewMatrix, view);

    world = world / world.w;

    return world;
}

float4 viewToModel(float4x4 inverseTransform, float4 view)
{
    float4 world = viewToWorld(view);

    return worldToModel(inverseTransform, world);
}

float4 clipToView(float4 clip)
{
    float4 view = mul(pViewParams.inverseProjection, clip);

    view = view / view.w;

    return view;
}

float4 clipToWorld(float4 clip)
{
    float4 view = clipToView(clip);

    return viewToWorld(view);
}

float4 screenToView(float4 screen)
{
    float2 texCoord = screen.xy / pViewParams.screenDimensions;
 
    // Convert to clip space 
    float4 clip = float4( float2( texCoord.x, 1.0f-texCoord.y ) * 2.0f - 1.0f, screen.z, screen.w);

    return clipToView(clip);
}

float4 screenToWorld(float4 screen)
{
    float4 view = screenToView(screen);

    return viewToWorld(view);
}

float4 screenToModel(float4x4 inverseTransform, float4 screen)
{
    float4 world = screenToWorld(screen);

    return worldToModel(inverseTransform, world);
}

float4 clipToScreen(float4 clip)
{
    float4 ndc = clip / clip.w;
    float2 texCoords = (float2(ndc.xy) + 1.0f) / 2.0f;
    float oz = 1;
    float pz = 0 - 1;
    float zf = pz * ndc.z + oz;
    return float4(float2(texCoords.x, 1 - texCoords.y) * pViewParams.screenDimensions, zf, 1.0f);
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
