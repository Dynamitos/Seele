import Common;
import VertexData;
import MaterialParameter;
import Scene;

[shader("vertex")]
FragmentParameter vertexMain(	
	uint vertexId: SV_VertexID,
	uint instanceId: SV_InstanceID,
){
	InstanceData inst = pScene.instances[instanceId];
	VertexAttributes attr = pVertexData.getAttributes(vertexId);
	return attr.getParameter(inst.transformMatrix);
}