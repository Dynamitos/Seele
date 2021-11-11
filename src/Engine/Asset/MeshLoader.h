#pragma once
#include "MinimalEngine.h"
#include "Containers/List.h"
#include "ThreadPool.h"
#include <thread>
#include <future>
#include <filesystem>

struct aiScene;
struct aiTexel;
namespace Seele
{
DECLARE_REF(Mesh)
DECLARE_REF(MeshAsset)
DECLARE_REF(MaterialAsset)
DECLARE_NAME_REF(Gfx, Graphics)
class MeshLoader
{
public:
    MeshLoader(Gfx::PGraphics graphic);
    ~MeshLoader();
    void importAsset(const std::filesystem::path& filePath);
private:
    void loadMaterials(const aiScene* scene, Array<PMaterialAsset>& globalMaterials, Gfx::PGraphics graphics);
    void loadTextures(const aiScene* scene, const std::filesystem::path& meshPath);
    void loadGlobalMeshes(const aiScene* scene, Array<PMesh>& globalMeshes, const Array<PMaterialAsset>& materials, Gfx::PGraphics graphics);
    void convertAssimpARGB(unsigned char* dst, aiTexel* src, uint32 numPixels);

    Job import(std::filesystem::path path, PMeshAsset meshAsset);
    List<std::future<void>> futures;
    Gfx::PGraphics graphics;
};
DEFINE_REF(MeshLoader)
} // namespace Seele