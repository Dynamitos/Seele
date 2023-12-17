#pragma once
#include "MinimalEngine.h"
#include "SystemBase.h"

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
    ThreadPool pool;
};
} // namespace System
} // namespace Seele
