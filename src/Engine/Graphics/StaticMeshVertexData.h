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
		Gfx::PDescriptorLayout descriptorLayout;
		Gfx::PDescriptorSet descriptorSet;
};
}
