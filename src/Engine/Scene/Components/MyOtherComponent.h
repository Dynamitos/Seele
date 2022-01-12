#pragma once
#include "Component.h"

namespace Seele
{
class MyOtherComponent : public Component
{
public:
    virtual Job tick(float deltaTime) const;
    virtual Job update();
    Writable<uint32> data = Writable<uint32>(0);
private:
};
DEFINE_REF(MyOtherComponent);
} // namespace Seele
