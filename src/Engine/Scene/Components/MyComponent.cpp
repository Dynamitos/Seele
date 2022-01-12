#include "MyComponent.h"

using namespace Seele;

MyComponent::MyComponent() 
{
}

void MyComponent::start() 
{
    otherComp = getComponent<MyOtherComponent>();
    otherComp.update();
}

Job MyComponent::tick(float deltatime) const
{
    writable++;
    otherComp->data = *writable;
    co_return;
}

Job MyComponent::update() 
{
    writable.update();
    co_return;
}
