#include "MyOtherComponent.h"

using namespace Seele;

Job MyOtherComponent::tick(float) const
{
    //std::cout << *data << std::endl;
    co_return;
}

Job MyOtherComponent::update() 
{
    data.update();
    co_return;
}