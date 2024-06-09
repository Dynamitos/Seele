#pragma once
#include "Component/Mesh.h"
#include "Component/Transform.h"
#include "ComponentSystem.h"

namespace Seele {
namespace System {
class MeshUpdater : public ComponentSystem<Component::Transform, Component::Mesh> {
  public:
    MeshUpdater(PScene scene);
    virtual ~MeshUpdater();
    virtual void update(entt::entity id, Component::Transform& transform, Component::Mesh& mesh) override;

  private:
};
} // namespace System
} // namespace Seele