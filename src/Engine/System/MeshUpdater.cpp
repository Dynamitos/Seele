#include "MeshUpdater.h"

using namespace Seele;
using namespace Seele::System;

MeshUpdater::MeshUpdater(PScene scene)
    : SystemBase(scene)
{
    scene->view<Component::Mesh>([&](entt::entity id, Component::Mesh& mesh) {
        meshEntities.add(id);
        });
    scene->constructCallback<Component::Mesh>().connect<&MeshUpdater::on_construct>(this);
    scene->destroyCallback<Component::Mesh>().connect<&MeshUpdater::on_destroy>(this);
}

MeshUpdater::~MeshUpdater()
{
}

void MeshUpdater::update()
{
}

void MeshUpdater::on_construct(entt::registry& reg, entt::entity id)
{
    meshEntities.add(id);
}

void MeshUpdater::on_destroy(entt::registry& reg, entt::entity id)
{
    meshEntities.remove(id, false);
}
