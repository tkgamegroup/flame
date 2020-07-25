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

		virtual Component* get_component(uint64 hash) const = 0;
		virtual void add_component(Component* c) = 0;
		virtual void remove_component(Component* c, bool destroy = true) = 0;
		virtual void remove_all_components(bool destroy = true) = 0;
		virtual void report_data_changed(Component* c, uint hash) = 0;

		virtual uint get_children_count() const = 0;
		virtual Entity* get_child(uint idx) const = 0;
		virtual void add_child(Entity* e, int position = -1/* -1 is end */) = 0;
		virtual void reposition_child(uint pos1, uint pos2) = 0;
		virtual void remove_child(Entity* e, bool destroy = true) = 0;
		virtual void remove_all_children(bool destroy = true) = 0;

		//inline bool is_child_of_r(const Entity* p, const Entity* e) const
		//{
		//	if (!e)
		//		return false;
		//	if (p == e)
		//		return true;
		//	return is_child_of_r(p, e->parent);
		//}

		//inline bool is_child_of(const Entity* p) const
		//{
		//	return is_child_of_r(p, this);
		//}

		//inline Entity* first_child(uint name_hash_check = 0) const
		//{
		//	auto c = children.s > 0 ? children[0] : nullptr;
		//	if (c && name_hash_check)
		//		return c->name.h == name_hash_check ? c : nullptr;
		//	return c;
		//}

		//inline Entity* last_child(uint name_hash_check = 0) const
		//{
		//	auto c = children.s > 0 ? children[children.s - 1] : nullptr;
		//	if (c && name_hash_check)
		//		return c->name.h == name_hash_check ? c : nullptr;
		//	return c;
		//}

		//Entity* find_child(const std::string& name) const
		//{
		//	for (auto e : children)
		//	{
		//		if (e->name == name)
		//			return e;
		//	}
		//	return nullptr;
		//}

		//int find_child(Entity* e) const
		//{
		//	for (auto i = 0; i < children.s; i++)
		//	{
		//		if (children[i] == e)
		//			return i;
		//	}
		//	return -1;
		//}

		virtual void load(const wchar_t* filename) = 0;
		virtual void save(const wchar_t* filename) = 0;

		FLAME_UNIVERSE_EXPORTS static Entity* create();
	};
}
