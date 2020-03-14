#include "entity_private.h"
#include <flame/universe/system.h>

namespace flame
{
	System::System(const char* name) :
		Object(name),
		world_(nullptr)
	{
		before_update_listeners.impl = ListenerHubImpl::create();
		after_update_listeners.impl = ListenerHubImpl::create();
	}

	System::~System()
	{
		ListenerHubImpl::destroy(before_update_listeners.impl);
		ListenerHubImpl::destroy(after_update_listeners.impl);
	}
}
