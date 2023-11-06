#include "Entity.h"

using namespace Seele;

Entity::Entity(PScene scene)
    : scene(scene)
    , identifier(scene->createEntity())
{
}


Entity::~Entity()
{
    scene->destroyEntity(identifier);
}