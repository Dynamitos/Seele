#include "MeshUpdater.h"

using namespace Seele;
using namespace Seele::System;

MeshUpdater::MeshUpdater(PScene scene)
    : ComponentSystem<Component::Transform, Component::Mesh>(scene)
{
}

MeshUpdater::~MeshUpdater()
{
}

void MeshUpdater::update(Component::Transform& transform, Component::Mesh& meshComp)
{
    for(const auto& mesh : meshComp.asset->meshes)
    {
        mesh->vertexData->updateMesh(transform, mesh);
    }
}
