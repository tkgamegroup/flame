#include <flame/universe/component.h>

namespace flame
{
	Component::Component(const char* name) :
		Object(name),
		entity(nullptr)
	{
		data_changed_listeners.impl = ListenerHubImpl::create();
	}

	Component::~Component()
	{
		ListenerHubImpl::destroy(data_changed_listeners.impl);
	}
}
