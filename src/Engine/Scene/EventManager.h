#pragma once
#include <entt/entt.hpp>

namespace Seele
{
template<typename T>
struct Event
{
};
class EventManager
{
public:
    template<typename T>
    void pushEvent(Event<T> event)
    {

    }
    template<typename T>
    void subscribe()
    {
        
    }
private:
};
} // namespace Seele
