#include "MinimalEngine.h"

std::map<void *, void *> registeredObjects;
std::mutex registeredObjectsLock;
