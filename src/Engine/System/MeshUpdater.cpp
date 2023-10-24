#include "MeshUpdater.h"

using namespace Seele;
using namespace Seele::System;

void MeshUpdater::update(Component::Transform& transform, Component::Mesh& mesh)
{
    mesh.vertexData->updateMesh(transform, mesh);
}
