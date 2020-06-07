#include "MinimalEngine.h"

Seele::Map<void *, void *> registeredObjects;
std::mutex registeredObjectsLock;
