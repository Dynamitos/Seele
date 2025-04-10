#pragma once
#include <memory_resource>
#include <map>
#include "MinimalEngine.h"

namespace Seele {
class MemoryManager {
  public:
    static RefPtr<uint64> getAllocationCounter(const std::string& name);
    static void printAllocations();

  private:
    static MemoryManager& getInstance();
    std::mutex allocationMutex;
    std::map<std::string, OwningPtr<uint64>> allocationCounters;
};

class debug_resource : public std::pmr::memory_resource {
  public:
    static debug_resource* get(const std::string& name);
    debug_resource() : name(""), upstream(std::pmr::get_default_resource()), counter(MemoryManager::getAllocationCounter(name)) {}
    debug_resource(std::string name, std::pmr::memory_resource* up = std::pmr::get_default_resource())
        : name(name), upstream(up), counter(MemoryManager::getAllocationCounter(name)) {}
    debug_resource(const debug_resource& other) = delete;
    debug_resource(debug_resource&& other) = default;
    debug_resource& operator=(const debug_resource& other) = delete;
    debug_resource& operator=(debug_resource&& other) = default;

    void* do_allocate(size_t bytes, size_t alignment) override {
//        std::cout << name << " + " << bytes << std::endl;
        *counter.getHandle() += bytes;
        return upstream->allocate(bytes, alignment);
    }

    void do_deallocate(void* ptr, size_t bytes, size_t alignment) override {
        upstream->deallocate(ptr, bytes, alignment);
//        std::cout << name << " - " << bytes << std::endl;
        *counter.getHandle() -= bytes;
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override { return this == &other; }

  private:
    std::string name;
    std::pmr::memory_resource* upstream;
    RefPtr<uint64> counter;
};
} // namespace Seele