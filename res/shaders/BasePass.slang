import Common;

[shader("pixel")]
void pixelMain(in VertexAttributes attribs, out float4 baseColor)
{
	BRDF brdf = gMaterial.prepare();
}
