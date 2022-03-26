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

Job MyComponent::tick(float) const
{
    //std::cout << "MyComponent::tick" << std::endl;
    ++writable;
    //std::cout << "MyComponent::tick finished" << std::endl;
    otherComp->data = *writable;
    co_return;
}

Job MyComponent::update() 
{
    //std::cout << "MyComponent::update" << std::endl;
    writable.update();
    //std::cout << "MyComponent::update finished" << std::endl;
    co_return;
}
