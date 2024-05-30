import Scene;
import MaterialParameter;

[shader("pixel")]
uint fragmentMain(in uint32_t encoded : SV_PrimitiveID) : SV_Target
{
    return encoded;
}