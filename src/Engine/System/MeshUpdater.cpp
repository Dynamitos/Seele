#include "MeshUpdater.h"

using namespace Seele;
using namespace Seele::System;

MeshUpdater::MeshUpdater(PScene scene)
    : ComponentSystem<Component::Transform, Component::Mesh>(scene)
{
}

void MeshUpdater::update(Component::Transform& transform, Component::Mesh& mesh)
{
    mesh.vertexData->updateMesh(transform, mesh);
}
