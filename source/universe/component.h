#pragma once

#include "universe.h"

namespace flame
{
	struct Component
	{
		uint type_hash = 0;

		EntityPtr entity = nullptr;

		Listeners<void(uint)> data_listeners;

		virtual ~Component() {}

		inline void data_changed(uint h)
		{
			for (auto& l : data_listeners.list)
				l(h);
		}

		virtual void on_enable_changed(bool v) {}
		virtual void on_state_changed(StateFlags state) {}
		virtual void on_entered_world() {}
		virtual void on_left_world() {}
		virtual void on_component_added(Component* c) {}
		virtual void on_component_removed(Component* c) {}
		virtual void on_entity_added() {}
		virtual void on_entity_removed() {}
		virtual void on_child_added(EntityPtr e) {}
		virtual void on_child_removed(EntityPtr e) {}
	};
}
