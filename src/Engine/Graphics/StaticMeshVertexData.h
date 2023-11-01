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
	void loadPositions(MeshId id, const Array<Vector>& data);
	void loadTexCoords(MeshId id, const Array<Vector2>& data);
	void loadNormals(MeshId id, const Array<Vector>& data);
	void loadTangents(MeshId id, const Array<Vector>& data);
	void loadBiTangents(MeshId id, const Array<Vector>& data);
	virtual void serializeMesh(MeshId id, uint64 numVertices, ArchiveBuffer& buffer) override;
	virtual void deserializeMesh(MeshId id, ArchiveBuffer& buffer) override;
	virtual void init(Gfx::PGraphics graphics) override;
	virtual void bindBuffers(Gfx::PRenderCommand command) override;
	virtual Gfx::PDescriptorLayout getVertexDataLayout() override;
	virtual Gfx::PDescriptorSet getVertexDataSet() override;
	virtual std::string getTypeName() const override { return "StaticMeshVertexData"; }
private:
	virtual void resizeBuffers() override;
	virtual void updateBuffers() override;
    Gfx::OShaderBuffer positions;
	Array<Vector> positionData;
    Gfx::OShaderBuffer texCoords;
	Array<Vector2> texCoordsData;
    Gfx::OShaderBuffer normals;
	Array<Vector> normalData;
    Gfx::OShaderBuffer tangents;
	Array<Vector> tangentData;
    Gfx::OShaderBuffer biTangents;
	Array<Vector> biTangentData;
	Gfx::ODescriptorLayout descriptorLayout;
	Gfx::ODescriptorSet descriptorSet;
};
}
