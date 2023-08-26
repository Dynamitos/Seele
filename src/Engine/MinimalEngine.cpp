#include "MinimalEngine.h"
#include <mutex>

std::map<void*, void*>& getRegisteredObjects() 
{
    static std::map<void*, void*> map;
    return map;
}

std::mutex& getRegisteredObjectLock() 
{
    static std::mutex lock;
    return lock;
}
