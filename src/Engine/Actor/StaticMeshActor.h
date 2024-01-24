#pragma once
#include "Actor.h"
#include "Component/Mesh.h"

namespace Seele
{
class StaticMeshActor : public Actor
{
public:
    StaticMeshActor(PScene scene, PMeshAsset mesh);
    virtual ~StaticMeshActor();
    Component::Mesh& getMesh();
    const Component::Mesh& getMesh() const;
private:
};
} // namespace Seele
