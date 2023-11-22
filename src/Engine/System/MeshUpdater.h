#pragma once
#include "SystemBase.h"
#include "Component/Transform.h"
#include "Component/Mesh.h"

namespace Seele
{
namespace System
{
class MeshUpdater : public SystemBase
{
public:
	MeshUpdater(PScene scene);
	virtual ~MeshUpdater();
	virtual void update() override;
private:
	Array<entt::entity> meshEntities;
	void on_construct(entt::registry& reg, entt::entity id);
	void on_destroy(entt::registry& reg, entt::entity id);
};
} // namespace System
} // namespace Seele