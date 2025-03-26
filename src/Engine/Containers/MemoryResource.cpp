#include "MemoryResource.h"
#include "Map.h"
#include <iostream>

using namespace Seele;


RefPtr<uint64> MemoryManager::getAllocationCounter(const std::string& name) {
    std::unique_lock l(getInstance().allocationMutex);
    if (!getInstance().allocationCounters.contains(name)) {
        getInstance().allocationCounters[name] = new uint64(0);
    }
    return getInstance().allocationCounters[name];
}

void MemoryManager::printAllocations() {
    std::unique_lock l(getInstance().allocationMutex);
    for (const auto& [name, counter] : getInstance().allocationCounters) {
        if (name.empty()) {
            std::cout << name << ": " << **counter << std::endl;
        } else {
            std::cout << name << ": " << **counter << std::endl;
        }
    }
}

MemoryManager& MemoryManager::getInstance() {
    static MemoryManager instance;
    return instance;
}

debug_resource* debug_resource::get(const std::string& name) {
    static std::mutex resourcesMutex;
    static std::map<std::string, debug_resource> debugResources;
    std::unique_lock l(resourcesMutex);
    if (!debugResources.count(name)) {
        debugResources.emplace(std::pair(name, debug_resource(name)));
    }
    return &debugResources[name];
}