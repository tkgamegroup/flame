#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct World;
	struct Component;

	struct Entity
	{
		// if it is a child, it will be removed first
		virtual void release() = 0;

		virtual const char* get_name() const = 0;
		virtual void set_name(const char* name) = 0;

		virtual bool get_visible() const = 0;
		virtual void set_visible(bool v) = 0;

		virtual World* get_world() const = 0;

		virtual Entity* get_parent() const = 0;

		virtual StateFlags get_state() const = 0;
		virtual void set_state(StateFlags state) = 0;

		virtual void on_message(Message msg) = 0;

		virtual Component* get_component(uint64 hash) const = 0;
		template <class T> inline T* get_component_t() const { return (T*)get_component(T::type_hash); }
		virtual void add_component(Component* c) = 0;
		virtual void remove_component(Component* c, bool destroy = true) = 0;
		virtual void remove_all_components(bool destroy = true) = 0;

		virtual uint get_children_count() const = 0;
		virtual Entity* get_child(uint idx) const = 0;
		virtual void add_child(Entity* e, int position = -1 /* -1 is end */ ) = 0;
		virtual void reposition_child(uint pos1, uint pos2) = 0;
		virtual void remove_child(Entity* e, bool destroy = true) = 0;
		virtual void remove_all_children(bool destroy = true) = 0;

		virtual void* add_data_changed_listener(void (*callback)(Capture& c, Component* t, uint64 data_name_hash), const Capture& capture) = 0;
		virtual void remove_data_changed_listener(void* lis) = 0;

		virtual void load(const wchar_t* filename) = 0;
		virtual void save(const wchar_t* filename) = 0;

		FLAME_UNIVERSE_EXPORTS static void report_data_changed(Component* c, uint64 hash);
		FLAME_UNIVERSE_EXPORTS static Entity* create();
	};
}
