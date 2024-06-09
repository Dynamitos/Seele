#include "SystemGraph.h"

using namespace Seele;

void SystemGraph::addSystem(System::OSystemBase system) { systems.add(std::move(system)); }

void SystemGraph::run(float deltaTime) {
    for (auto& system : systems) {
        system->run(deltaTime);
    }
}
