#include "Entity.h"

using namespace Seele;

Entity::Entity(PScene scene)
    : identifier(scene->createEntity())
    , scene(scene)
{
}


Entity::~Entity()
{
    
}