#include "MemoryResource.h"
#include "Map.h"
#include <iostream>

using namespace Seele;

std::mutex allocationMutex;
std::map<std::string, std::atomic_uint64_t> allocationCounters;
std::mutex resourcesMutex;
std::map<std::string, debug_resource> debugResources;

std::atomic_uint64_t& MemoryManager::getAllocationCounter(const std::string& name) {
    std::unique_lock l(allocationMutex);
    return allocationCounters[name];
}

void MemoryManager::printAllocations() {
    std::unique_lock l(allocationMutex);
    for (const auto& [name, counter] : allocationCounters) {
        if (name.empty())
        {
            std::cout << name << ": " << counter << std::endl;
        }
        else
        {
            std::cout << name << ": " << counter << std::endl;
        }
    }
}

debug_resource* debug_resource::get(const std::string& name) {
    std::unique_lock l(resourcesMutex);
    return &debugResources[name];
}