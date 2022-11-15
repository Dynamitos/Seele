#pragma once
#include "MinimalEngine.h"
#include "SystemBase.h"
#include <thread_pool/thread_pool.h>

namespace Seele
{
namespace System
{
class Executor
{
public:
    Executor();
    ~Executor();
private:
    dp::thread_pool<> pool;
};
} // namespace System
} // namespace Seele
