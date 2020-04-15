#include "MinimalEngine.h"
#include "Actor/Actor.h"

namespace Seele
{
class Scene
{
public:
    Scene();
    ~Scene();
private:
    Array<PActor> rootActors;
};
}