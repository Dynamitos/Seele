#pragma once
#include "MinimalEngine.h"
#include <functional>

namespace Seele {
namespace UI {
struct Attributes {
    std::function<void()> onClick;
};
} // namespace UI
} // namespace Seele