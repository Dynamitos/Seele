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

Map<PMaterial, DrawState> Scene::getMeshBatches()
{
    meshBatches.clear();
    for (auto primitive : primitives)
    {
        PMaterialInstance matInstance = primitive->instance;
        PMaterial mat = matInstance->getBaseMaterial();
        DrawInstance inst;
        inst.instance = primitive->instance;
        inst.vertexBuffer = primitive->vertexBuffer;
        inst.indexBuffer = primitive->indexBuffer;
        inst.modelMatrix = primitive->getRenderMatrix();

        if (meshBatches.find(mat) != meshBatches.end())
        {
            DrawState &state = meshBatches[mat];
            state.instances.add(inst);
        }
        else
        {
            DrawState state;
            state.instances.add(inst);
            meshBatches[mat] = state;
        }
    }
    return meshBatches;
}