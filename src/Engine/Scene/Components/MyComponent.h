#pragma once
#include "Component.h"
#include "MyOtherComponent.h"

namespace Seele
{
class MyComponent : public Component
{
public:
    MyComponent();
    virtual void start();
    virtual Job tick(float deltatime) const;
    virtual Job update();
private:
    Writable<PMyOtherComponent> otherComp;
    Writable<uint32> writable = 0;
    uint32 notWritable = 10;
};
DECLARE_REF(MyComponent);
} // namespace Seele
