#pragma once

#include <flame/foundation/foundation.h>
#include <flame/universe/universe.h>

namespace flame
{
	struct TypeInfoDatabase;

	struct World;
	struct Component;

	enum EntityEvent
	{
		EntityDestroyed,
		EntityVisibilityChanged,
		EntityAdded,
		EntityRemoved,
		EntityPositionChanged,
		EntityEnteredWorld,
		EntityLeftWorld,
		EntityComponentAdded,
		EntityComponentRemoved,
		EntityChildVisibilityChanged,
		EntityChildAdded,
		EntityChildRemoved,
		EntityChildPositionChanged,
		EntityChildComponentAdded,
		EntityChildComponentRemoved
	};

	struct Entity
	{
		virtual void release() = 0;

		virtual bool get_visible() const = 0;
		virtual void set_visible(bool v) = 0;

		virtual Component* get_component_plain(uint hash) const = 0;

		template <class T>
		T* get_component_t(uint hash) const
		{
			return (T*)get_component_plain(hash);
		}

#define get_component(T) get_component_t<T>(FLAME_CHASH(#T))
#define get_id_component(T, id) get_component_t<T>(hash_update(FLAME_CHASH(#T), id))
		virtual void add_component(Component* c) = 0;
		virtual void remove_component(Component* c) = 0;

		virtual void data_changed(Component* c, uint hash, void* sender) = 0;

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

		virtual void add_child(Entity* e, int position = -1); /* -1 is end */
		virtual void reposition_child(Entity* e, int position); /* -1 is last */
		virtual void remove_child(Entity* e, bool destroy = true);
		virtual void remove_children(int from, int to /* -1 is end */, bool destroy = true);

		virtual void load(const wchar_t* filename);
		virtual void save(const wchar_t* filename);

		FLAME_UNIVERSE_EXPORTS static Entity* create();
	};
}
