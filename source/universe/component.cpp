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
		data_changed_listeners.impl->release();
	}

	void Component::data_changed(uint hash, void* sender)
	{
		if (sender != INVALID_POINTER)
		{
			for (auto c : entity->components.get_all())
			{
				if (c != this)
					c->on_sibling_data_changed(this, hash, sender);
			}
			auto p = entity->parent;
			if (p)
			{
				for (auto c : p->components.get_all())
					c->on_child_data_changed(this, hash, sender);
			}
			data_changed_listeners.call_with_current(this, hash, sender);
		}
	}
}
