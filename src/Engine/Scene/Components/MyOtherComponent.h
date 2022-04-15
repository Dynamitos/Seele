#pragma once
#include "Component.h"

namespace Seele
{
class MyOtherComponent : public Component
{
public:
    virtual void tick(float deltaTime) const;
    virtual void update();
    Writable<uint32> data = Writable<uint32>(0);
private:
};
DEFINE_REF(MyOtherComponent);
} // namespace Seele
