#include <flame/universe/component.h>

namespace flame
{
	Component::Component(const char* name) :
		Object(name),
		entity(nullptr)
	{
		data_changed_listeners.hub = listeners_init();
	}

	Component::~Component()
	{
		listeners_deinit(data_changed_listeners.hub);
	}

	void Component::data_changed(uint hash, void* sender)
	{
		auto hub = data_changed_listeners.hub;
		for (auto i = 0; i < listeners_count(hub); i++)
			listeners_listener(hub, i).call<void(void*, Component * thiz, uint hash, void* sender)>(this, hash, sender);
	}
}
