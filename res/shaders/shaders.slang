import Mat;
import Simple;

[shader("fragment")]
float4 fragmentMain(float2 texCoord)
{
    return gMaterial.sample(texCoord);
}