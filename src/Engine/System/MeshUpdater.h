#pragma once
#include "ComponentSystem.h"
#include "Component/Transform.h"
#include "Component/Mesh.h"

namespace Seele
{
namespace System
{
class MeshUpdater : public ComponentSystem<Component::Transform, Component::Mesh>
{
public:
	MeshUpdater(PScene scene);
	virtual ~MeshUpdater();
	virtual void update(Component::Transform& transform, Component::Mesh& mesh) override;
private:
};
} // namespace System
} // namespace Seele