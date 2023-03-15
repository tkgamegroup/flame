#pragma once

#include "universe.h"

namespace flame
{
	// Reflect
	struct Component
	{
		uint type_hash = 0;
		EntityPtr entity = nullptr;
		// Reflect
		bool enable = true;
		Listeners<void(uint)> data_listeners;

		ushort ref = 0;
		ushort update_times = 0;

		virtual ~Component() 
		{
			on_inactive();
		}

		inline void data_changed(uint h)
		{
			for (auto& l : data_listeners.list)
				l.first(h);
		}

		// Reflect
		FLAME_UNIVERSE_API void set_enable(bool v);

		// Reflect
		virtual void on_init() {}
		// Reflect
		virtual void on_active() {}
		// Reflect
		virtual void on_inactive() {}
		// Reflect
		virtual void on_component_added(Component* c) {}
		// Reflect
		virtual void on_component_removed(Component* c) {}
		// Reflect
		virtual void on_entity_added() {}
		// Reflect
		virtual void on_entity_removed() {}
		// Reflect
		virtual void on_child_added(EntityPtr e) {}
		// Reflect
		virtual void on_child_removed(EntityPtr e) {}

		// Reflect
		virtual void start() {}
		// Reflect
		virtual void update() {}
		virtual void send_message(uint hash, void* data, uint size) {}
	};
}
