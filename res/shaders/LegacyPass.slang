import Common;
import VertexData;
import MaterialParameter;
import Scene;

[shader("vertex")]
FragmentParameter vertexMain(
	VertexInput input
){
	InstanceData inst = pScene.instances[pOffsets.instanceOffset + input.instanceId];
	VertexAttributes attr = pVertexData.getAttributes(input.vertexId);
	return attr.getParameter(inst.transformMatrix);
}