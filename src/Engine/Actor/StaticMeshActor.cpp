#include "StaticMeshActor.h"

using namespace Seele;

StaticMeshActor::StaticMeshActor(PScene scene, PMeshAsset mesh) : Actor(scene) { attachComponent<Component::Mesh>(mesh); }

Seele::StaticMeshActor::~StaticMeshActor() {}

Component::Mesh& Seele::StaticMeshActor::getMesh() { return accessComponent<Component::Mesh>(); }

const Component::Mesh& Seele::StaticMeshActor::getMesh() const { return accessComponent<Component::Mesh>(); }
