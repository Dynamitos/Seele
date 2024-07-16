#include "Asset/Asset.h"
#include "Asset/AssetRegistry.h"
#include "Asset/FontAsset.h"
#include "Asset/MaterialAsset.h"
#include "Asset/MaterialInstanceAsset.h"
#include "Asset/MeshAsset.h"
#include "Asset/TextureAsset.h"
#include <assimp/Exporter.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <fstream>
#include <iostream>

using namespace Seele;

AssetRegistry* _instance = new AssetRegistry();

int main(int, char**) {
    Assimp::Importer importer;
    const aiScene* scene =
        importer.ReadFile("C:\\Users\\Dynamitos\\MeshShadingDemo\\import\\models\\after-the-rain-vr-sound\\source\\WhitechapelSrc.glb",
                          (uint32)(aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_ImproveCacheLocality));

    auto interpolate = [](Vector p0, Vector p1, Vector p2) { return (p0 + p1 + p2) / 3.0f; };
    
    //std::fstream stats("s.csv");
    //stats << "area" << std::endl;
    for (uint32 m = 0; m < scene->mNumMeshes; ++m) {
        aiMesh* mesh = scene->mMeshes[m];
        Array<Vector> positions(mesh->mNumVertices);
        Array<Vector> normals(mesh->mNumVertices);
        Array<Vector> tangents(mesh->mNumVertices);
        Array<Vector> biTangents(mesh->mNumVertices);
        Array<Vector> texCoords(mesh->mNumVertices);
        Array<Vector> texCoords1(mesh->mNumVertices);
        if (mesh->HasTextureCoords(2))
            abort();
        for (int32 i = 0; i < mesh->mNumVertices; ++i) {
            positions[i] = Vector(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

            if (mesh->mTextureCoords[0] != nullptr)
                texCoords[i] = Vector(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y, 0);

            if (mesh->mTextureCoords[1] != nullptr)
                texCoords1[i] = Vector(mesh->mTextureCoords[1][i].x, mesh->mTextureCoords[1][i].y, 0);

            if (mesh->mNormals != nullptr)
                normals[i] = Vector(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

            if (mesh->mTangents != nullptr)
                tangents[i] = Vector(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);

            if (mesh->mBitangents != nullptr)
                biTangents[i] = Vector(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
        }

        struct Tri {
            uint32 p[3];
        };
        auto calcArea = [&positions](Tri t) {
            Vector p1 = positions[t.p[0]];
            Vector p2 = positions[t.p[1]];
            Vector p3 = positions[t.p[2]];
            return 0.5 * glm::length(glm::cross(p2 - p1, p3 - p1));
        };
        Array<Tri> tris(mesh->mNumFaces);
        for (int32 faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
            tris[faceIndex].p[0] = mesh->mFaces[faceIndex].mIndices[0];
            tris[faceIndex].p[1] = mesh->mFaces[faceIndex].mIndices[1];
            tris[faceIndex].p[2] = mesh->mFaces[faceIndex].mIndices[2];
        }
        auto tesselate = [&](uint32 i) {
            uint32 p0 = tris[i].p[0];
            uint32 p1 = tris[i].p[1];
            uint32 p2 = tris[i].p[2];
            uint32 p3 = positions.size();
            positions.add(interpolate(positions[p0], positions[p1], positions[p2]));

            if (mesh->mTextureCoords[0] != nullptr)
                texCoords.add(interpolate(texCoords[p0], texCoords[p1], texCoords[p2]));

            if (mesh->mTextureCoords[1] != nullptr)
                texCoords1.add(interpolate(texCoords1[p0], texCoords1[p1], texCoords1[p2]));

            if (mesh->mNormals != nullptr)
                normals.add(interpolate(normals[p0], normals[p1], normals[p2]));

            if (mesh->mTangents != nullptr)
                tangents.add(interpolate(tangents[p0], tangents[p1], tangents[p2]));

            if (mesh->mBitangents != nullptr)
                biTangents.add(interpolate(biTangents[p0], biTangents[p1], biTangents[p2]));

            tris[i].p[2] = p3;

            tris.add(Tri{p1, p2, p3});

            tris.add(Tri{p2, p0, p3});
        };
        
        //for (uint32 i = 0; i < tris.size(); ++i) {
        //    stats << calcArea(tris[i]) << std::endl;
        //}
        bool tess = false;
        //while (!tess) {
        //    tess = true;
        //    for (uint32 i = 0; i < tris.size(); ++i) {
        //        if (calcArea(tris[i]) > 100.f) {
        //            tesselate(i);
        //            tess = false;
        //        }
        //    }
        //}

        aiVector3D* newPos = new aiVector3D[positions.size()];
        std::memcpy(newPos, positions.data(), positions.size() * sizeof(Vector));

        if (mesh->mTextureCoords[0] != nullptr) {
            aiVector3D* newTex = new aiVector3D[texCoords.size()];
            std::memcpy(newTex, texCoords.data(), texCoords.size() * sizeof(Vector));
            mesh->mTextureCoords[0] = newTex;
        }

        if (mesh->mTextureCoords[1] != nullptr) {
            aiVector3D* newTex = new aiVector3D[texCoords1.size()];
            std::memcpy(newTex, texCoords1.data(), texCoords1.size() * sizeof(Vector));
            mesh->mTextureCoords[1] = newTex;
        }
        if (mesh->mNormals != nullptr) {
            aiVector3D* newNor = new aiVector3D[normals.size()];
            std::memcpy(newNor, normals.data(), normals.size() * sizeof(Vector));
            mesh->mNormals = newNor;
        }
        if (mesh->mTangents != nullptr) {
            aiVector3D* newTan = new aiVector3D[tangents.size()];
            std::memcpy(newTan, tangents.data(), tangents.size() * sizeof(Vector));
            mesh->mTangents = newTan;
        }
        if (mesh->mBitangents != nullptr) {
            aiVector3D* newBit = new aiVector3D[biTangents.size()];
            std::memcpy(newBit, biTangents.data(), biTangents.size() * sizeof(Vector));
            mesh->mBitangents = newBit;
        }

        uint32 numFaces = tris.size();
        aiFace* newFaces = new aiFace[numFaces];
        for (uint32 i = 0; i < numFaces; ++i) {
            newFaces[i].mNumIndices = 3;
            newFaces[i].mIndices = new uint32[3];
            newFaces[i].mIndices[0] = tris[i].p[0];
            newFaces[i].mIndices[1] = tris[i].p[1];
            newFaces[i].mIndices[2] = tris[i].p[2];
        }

        mesh->mNumVertices = positions.size();
        mesh->mVertices = newPos;
        mesh->mFaces = newFaces;
        mesh->mNumFaces = tris.size();
    }

    Assimp::Exporter exporter;
    exporter.Export(scene, "glb2",
                    "C:\\Users\\Dynamitos\\MeshShadingDemo\\import\\models\\after-the-rain-vr-sound\\source\\Whitechapel.glb");
    for (uint32 i = 0; i < exporter.GetExportFormatCount(); ++i) {
        auto desc = exporter.GetExportFormatDescription(i);
        std::cout << desc->description << " " << desc->fileExtension << " " << desc->id << " " << std::endl;
    }
}