#include "universe_private.h"
#include <flame/universe/component.h>

namespace flame
{
	Component::Component(const char* name) :
		type_name(name),
		type_hash(H(name))
	{
		data_changed_listeners.hub = new ListenerHub;
	}

	Component::~Component()
	{
		delete (ListenerHub*)data_changed_listeners.hub;
	}
}
