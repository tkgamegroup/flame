#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct Entity;
	struct EntityPrivate;

	struct Component
	{
		const char* type_name;
		const uint type_hash;

#ifdef FLAME_UNIVERSE_MODULE
		EntityPrivate* entity = nullptr;
#else
		Entity* entity = nullptr;
#endif

		int src_id = -1;

		Component(const char* name, uint hash) :
			type_name(name),
			type_hash(hash)
		{
		}

		virtual ~Component() {}

		virtual void on_added() {}
		virtual void on_removed() {}
		virtual void on_destroyed() {}
		virtual void on_visibility_changed(bool v) {}
		virtual void on_state_changed(StateFlags state) {}
		virtual void on_entered_world() {}
		virtual void on_left_world() {}
		virtual void on_self_added() {}
		virtual void on_self_removed() {}
		virtual void on_child_added(Entity* e) {}
		virtual void on_child_removed(Entity* e) {}
		virtual void on_reposition(uint from, uint to) {}

		virtual bool on_save_attribute(uint h) { return true; }
	};
}
