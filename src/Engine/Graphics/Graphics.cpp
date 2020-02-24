#include "Graphics.h"
#include <iostream>
#include <map>

Seele::Graphics::Graphics()
{
	Array<uint8> arr;
	arr.add('2');
	arr.add('4');
	std::cout << "Test" << std::endl;
	for (auto a : arr)
	{
		std::cout << "Element: " << a << std::endl;
	}
}

Seele::Graphics::~Graphics()
{

}