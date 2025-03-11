import Common;
import VertexData;
import StaticMeshVertexData;
import MaterialParameter;
import Scene;

[shader("vertex")]
FragmentParameter vertexMain(
	VertexInput input
){
	InstanceData inst = pScene.instances[input.instanceId];
	VertexAttributes attr = pVertexData.getAttributes(input.vertexId);
	return attr.getParameter(inst.transformMatrix, inst.inverseTransformMatrix);
}