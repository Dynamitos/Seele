#pragma once
#include "MinimalEngine.h"
#include "Containers/List.h"
#include "Component/Collider.h"
#include <filesystem>

struct aiScene;
struct aiTexel;
namespace Seele
{
DECLARE_REF(Mesh)
DECLARE_REF(MeshAsset)
DECLARE_REF(MaterialAsset)
DECLARE_NAME_REF(Gfx, Graphics)
struct MeshImportArgs
{
    std::filesystem::path filePath;
    std::string importPath;
};
class MeshLoader
{
public:
    MeshLoader(Gfx::PGraphics graphic);
    ~MeshLoader();
    void importAsset(MeshImportArgs args);
private:
    void loadMaterials(const aiScene* scene, const std::string& baseName, const std::filesystem::path& meshDirectory, const std::string& importPath, Array<PMaterialAsset>& globalMaterials);
    void loadTextures(const aiScene* scene, const std::filesystem::path& meshDirectory, const std::string& importPath);
    void loadGlobalMeshes(const aiScene* scene, const Array<PMaterialAsset>& materials, Array<PMesh>& globalMeshes, Component::Collider& collider);
    void convertAssimpARGB(unsigned char* dst, aiTexel* src, uint32 numPixels);

    void import(MeshImportArgs args, PMeshAsset meshAsset);
    Gfx::PGraphics graphics;
};
DEFINE_REF(MeshLoader)
} // namespace Seele