#include "MyOtherComponent.h"

using namespace Seele;

void MyOtherComponent::tick(float) const
{
    //std::cout << *data << std::endl;
    //co_return;
}

void MyOtherComponent::update() 
{
    data.update();
    //co_return;
}
