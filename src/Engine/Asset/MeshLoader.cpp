#include "MeshLoader.h"
#include "Graphics/Graphics.h"
#include "MeshAsset.h"
#include "Graphics/Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace Seele;

MeshLoader::MeshLoader(Gfx::PGraphics graphics)
    : graphics(graphics)
{
}

MeshLoader::~MeshLoader()
{
}

void MeshLoader::importAsset(const std::string& path)
{
    futures.add(std::async(std::launch::async, &MeshLoader::import, this, path));
}

void findMeshRoots(aiNode* node, List<aiNode*>& meshNodes) 
{
    if(node->mNumMeshes > 0)
    {
        meshNodes.add(node);
        return;
    }
    for(uint32 i = 0; i < node->mNumChildren; ++i)
    {
        findMeshRoots(node->mChildren[i], meshNodes);
    }
}

void MeshLoader::import(const std::string& path)
{
    PMeshAsset asset = new MeshAsset(path);
    asset->setStatus(Asset::Status::Loading);
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path.c_str(),
        aiProcess_CalcTangentSpace |
        aiProcess_FlipUVs |
        aiProcess_ImproveCacheLocality |
        aiProcess_OptimizeMeshes |
        aiProcess_GenBoundingBoxes |
        aiProcessPreset_TargetRealtime_Fast);
    
    List<aiNode*> meshNodes;
    findMeshRoots(scene->mRootNode, meshNodes);
    for(auto meshNode : meshNodes)
    {
        std::cout << "Test:" << meshNode->mNumMeshes << std::endl;
    }

    PMesh mesh = new Mesh(nullptr, nullptr);
    asset->setMesh(mesh);
    asset->setStatus(Asset::Status::Ready);
}
