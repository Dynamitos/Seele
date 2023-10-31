#pragma once
#include "VertexData.h"

namespace Seele
{
class StaticMeshVertexData : public VertexData
{
public:
    StaticMeshVertexData();
    virtual ~StaticMeshVertexData();
	static StaticMeshVertexData* getInstance();
    virtual MeshId allocateVertexData(uint64 numVertices) override;
	void loadPositions(MeshId id, const Array<Vector>& data);
	void loadTexCoords(MeshId id, const Array<Vector2>& data);
	void loadNormals(MeshId id, const Array<Vector>& data);
	void loadTangents(MeshId id, const Array<Vector>& data);
	void loadBiTangents(MeshId id, const Array<Vector>& data);
	virtual void init(Gfx::PGraphics graphics) override;
private:
	virtual void updateBuffers() override;
    Gfx::PShaderBuffer positions;
	Array<Vector> positionData;
    Gfx::PShaderBuffer texCoords;
	Array<Vector2> texCoordsData;
    Gfx::PShaderBuffer normals;
	Array<Vector> normalData;
    Gfx::PShaderBuffer tangents;
	Array<Vector> tangentData;
    Gfx::PShaderBuffer biTangents;
	Array<Vector> biTangentData;
    Map<MeshId, uint64_t> meshOffsets;
	uint64 head;
	uint64 verticesAllocated;
};
}
