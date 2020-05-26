#include <flame/universe/component.h>

namespace flame
{
	Component::Component(const char* name) :
		Object(name),
		entity(nullptr),
		user_data(nullptr)
	{
		data_changed_listeners.impl = ListenerHubImpl::create();
	}

	Component::~Component()
	{
		ListenerHubImpl::destroy(data_changed_listeners.impl);
	}

	void Component::data_changed(uint hash, void* sender)
	{
		if (sender != INVALID_POINTER)
		{
			for (auto c : sibling_data_changed_info_targets)
				c->on_sibling_data_changed(this, hash, sender);
			for (auto c : parent_data_changed_info_targets)
				c->on_child_data_changed(this, hash, sender);
			data_changed_listeners.call_with_current(this, hash, sender);
		}
	}
}
