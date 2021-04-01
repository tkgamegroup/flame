#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct Component;
	struct Driver;
	struct World;

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
		virtual uint get_index() const = 0;

		virtual StateFlags get_state() const = 0;
		virtual void set_state(StateFlags state) = 0;

		virtual const wchar_t* get_srcs() const = 0;

		virtual Component* get_component(uint hash) const = 0;
		virtual Component* find_component(const char* name) const = 0;
		template <class T> inline T* get_component_t() const { return (T*)get_component(T::type_hash); }
		virtual void get_components(void (*callback)(Capture& c, Component* cmp), const Capture& capture) const = 0;

		virtual void add_component(Component* c) = 0;
		virtual void remove_component(Component* c, bool destroy = true) = 0;

		virtual uint get_children_count() const = 0;
		virtual Entity* get_child(uint idx) const = 0;
		virtual void add_child(Entity* e, int position = -1 /* -1 is end */ ) = 0;
		virtual void reposition_child(uint pos1, uint pos2) = 0;
		virtual void remove_child(Entity* e, bool destroy = true) = 0;
		virtual Entity* find_child(const char* name) const = 0;

		virtual Driver* get_driver(uint hash, int idx = -1) const = 0;
		virtual Driver* find_driver(const char* name) const = 0;
		template <class T> inline T* get_driver_t() const { return (T*)get_driver(T::type_hash); }

		virtual void push_driver(Driver* d) = 0;
		virtual void pop_driver() = 0;

		virtual void* add_message_listener(void (*callback)(Capture& c, uint msg, void* parm1, void* parm2), const Capture& capture) = 0;
		virtual void remove_message_listener(void* lis) = 0;

		virtual void component_data_changed(Component* c, uint h) = 0;
		virtual void* add_component_data_listener(void (*callback)(Capture& c, uint hash), const Capture& capture, Component* c) = 0;
		virtual void remove_component_data_listener(void* lis, Component* c) = 0;

		virtual void driver_data_changed(Driver* d, uint h) = 0;
		virtual void* add_driver_data_listener(void (*callback)(Capture& c, uint hash), const Capture& capture, Driver* d) = 0;
		virtual void remove_driver_data_listener(void* lis, Driver* d) = 0;

		virtual void* add_event(void (*callback)(Capture& c), const Capture& capture, float interval = 0.f /* 0 means every frame */ ) = 0;
		virtual void remove_event(void* ev) = 0;

		virtual bool load(const wchar_t* filename) = 0;
		virtual bool save(const wchar_t* filename) = 0;

		virtual void* get_userdata() const = 0;
		virtual void set_userdata(void* d) = 0;

		FLAME_UNIVERSE_EXPORTS static Entity* create();
		FLAME_UNIVERSE_EXPORTS static void initialize();
	};
}
