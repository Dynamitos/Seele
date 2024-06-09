#pragma once
#include "Resources.h"

namespace Seele
{
namespace Gfx
{
class BottomLevelAS
{
public:
    BottomLevelAS();
    ~BottomLevelAS();
private:
};
DEFINE_REF(BottomLevelAS)
class TopLevelAS
{
public:
    TopLevelAS();
    ~TopLevelAS();
private:
};
DEFINE_REF(TopLevelAS)
}
}