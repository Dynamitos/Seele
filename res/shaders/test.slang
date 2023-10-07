struct Payload
{
	uint num;
};

[shader("mesh")]
[numthreads(32, 1, 1)]
[outputtopology("triangle")]
void taskMain(uint threadID: SV_DispatchThreadID)
{
	SetOutputMeshCounts(64, 126);
}