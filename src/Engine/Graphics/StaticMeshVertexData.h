#pragma once
#include "Graphics/Initializer.h"
#include "Graphics/Command.h"
#include "Math/Vector.h"
#include "VertexData.h"
#include "entt/entt.hpp"

namespace Seele
{
class StaticMeshVertexData : public VertexData
{
public:
	struct StaticMeshMapping
	{
		MeshId original;
		MeshId mapped;
		PMaterialInstance material;
	};
    StaticMeshVertexData();
    virtual ~StaticMeshVertexData();
	static StaticMeshVertexData* getInstance();
	void loadPositions(MeshId id, const Array<Vector>& data);
	void loadTexCoords(MeshId id, uint64 index, const Array<Vector2>& data);
	void loadNormals(MeshId id, const Array<Vector>& data);
	void loadTangents(MeshId id, const Array<Vector>& data);
	void loadBiTangents(MeshId id, const Array<Vector>& data);
	void loadColors(MeshId id, const Array<Vector>& data);
	virtual void serializeMesh(MeshId id, uint64 numVertices, ArchiveBuffer& buffer) override;
	virtual void deserializeMesh(MeshId id, ArchiveBuffer& buffer) override;
	virtual void init(Gfx::PGraphics graphics) override;
	virtual void destroy() override;
	virtual void bindBuffers(Gfx::PRenderCommand command) override;
	virtual Gfx::PDescriptorLayout getVertexDataLayout() override;
	virtual Gfx::PDescriptorSet getVertexDataSet() override;
	virtual std::string getTypeName() const override { return "StaticMeshVertexData"; }
	void registerStaticMesh(const Array<OMesh>& meshes, const Component::Transform& transform);
private:
	virtual void resizeBuffers() override;
	virtual void updateBuffers() override;
	Array<MeshletDescription> staticMeshlets;

	std::mutex mutex;
    Gfx::OShaderBuffer positions;
	Array<Vector> positionData;
    Gfx::OShaderBuffer texCoords[MAX_TEXCOORDS];
	Array<Vector2> texCoordsData[MAX_TEXCOORDS];
    Gfx::OShaderBuffer normals;
	Array<Vector> normalData;
    Gfx::OShaderBuffer tangents;
	Array<Vector> tangentData;
    Gfx::OShaderBuffer biTangents;
	Array<Vector> biTangentData;
	Gfx::OShaderBuffer colors;
	Array<Vector> colorData;
	Gfx::ODescriptorLayout descriptorLayout;
	Gfx::PDescriptorSet descriptorSet;
};
}
