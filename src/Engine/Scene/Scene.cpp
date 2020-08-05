#include "Scene.h"
#include "Components/PrimitiveComponent.h"
#include "Material/MaterialInstance.h"
#include "Material/Material.h"

using namespace Seele;

Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::tick(float deltaTime)
{
    for (auto actor : rootActors)
    {
        actor->tick(deltaTime);
    }
}

void Scene::addActor(PActor actor)
{
    rootActors.add(actor);
    actor->notifySceneAttach(this);
}

void Scene::addPrimitiveComponent(PPrimitiveComponent comp)
{
    primitives.add(comp);
}
