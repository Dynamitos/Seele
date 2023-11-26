#include "MeshUpdater.h"
#include "Component/Mesh.h"

using namespace Seele;
using namespace Seele::System;

MeshUpdater::MeshUpdater(PScene scene)
    : ComponentSystem<Component::Transform, Component::Mesh>(scene)
{
}

MeshUpdater::~MeshUpdater()
{
}

void MeshUpdater::update(Component::Transform& transform, Component::Mesh& comp)
{
    for (auto& mesh : comp.asset->meshes)
    {
        mesh->vertexData->updateMesh(mesh, transform);
    }
}
