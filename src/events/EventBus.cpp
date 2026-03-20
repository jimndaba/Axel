#include "axelpch.h"
#include "EventBus.h"

std::unordered_map<std::type_index, Axel::HandlerList*> Axel::Eventbus::subscribers;

Axel::Eventbus::Eventbus()
{
}
