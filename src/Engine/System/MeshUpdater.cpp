#include "MeshUpdater.h"
#include "Component/Mesh.h"
#include "Graphics/Mesh.h"

using namespace Seele;
using namespace Seele::System;

MeshUpdater::MeshUpdater(PScene scene) : ComponentSystem<Component::Transform, Component::Mesh>(scene) {}

MeshUpdater::~MeshUpdater() {}

void MeshUpdater::update(entt::entity id, Component::Transform& transform, Component::Mesh& comp) {
    scene->accessComponent<Component::Camera>(id);
    if (comp.meshletOffsets.empty()) {
        for (uint32 i = 0; i < comp.asset->meshes.size(); ++i) {
            comp.meshletOffsets.add(comp.asset->meshes[i]->vertexData->addCullingMapping(comp.asset->meshes[i]->id));
        }
    }
    for (uint32 i = 0; i < comp.asset->meshes.size(); ++i) {
        comp.asset->meshes[i]->vertexData->updateMesh(comp.meshletOffsets[i], comp.asset->meshes[i], transform);
    }
}
