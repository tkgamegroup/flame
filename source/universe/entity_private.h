#pragma once

#include <flame/universe/entity.h>
#include "universe_private.h"

namespace flame
{
	struct WorldPrivate;

	struct EntityPrivate : Entity
	{
		std::string _name;
		uint _name_hash;

		bool _visible = true;
		bool _global_visibility = false;

		void* _gene = nullptr;

		WorldPrivate* _world = nullptr;

		EntityPrivate* _parent = nullptr;
		std::unordered_map<uint, std::unique_ptr<Component, Delecter>> _components;
		std::vector<std::unique_ptr<EntityPrivate, Delecter>> _children;
		std::vector<Component*> _local_event_dispatch_list;
		std::vector<Component*> _child_event_dispatch_list;
		std::vector<Component*> _local_data_changed_dispatch_list;
		std::vector<Component*> _child_data_changed_dispatch_list;

		uint _depth = 0;
		uint _index = 0;
		int _created_frame;
		std::vector<void*> _created_stack;

		EntityPrivate();
		~EntityPrivate();

		void _release();

		void _set_name(const std::string& name);

		void _update_visibility();
		void _set_visible(bool v);

		Component* _get_component_plain(uint hash) const;
		void _add_component(Component* c);
		void _remove_component(Component* c, bool destroy = true);
		void _data_changed(Component* c, uint hash);

		void _enter_world();
		void _leave_world();
		void _inherit();
		void _add_child(EntityPrivate* e, int position = -1);
		void _reposition_child(uint pos1, uint pos2);
		void _remove_child(EntityPrivate* e, bool destroyy = true);

		void _load(const std::filesystem::path& filename);
		void _save(const std::filesystem::path& filename);

		static EntityPrivate* _create();

		void release() { _release(); }

		const char* get_name() const override { return _name.c_str(); };
		uint get_name_hash() const override { return _name_hash; };
		void set_name(const char* name) override { return _set_name(name); }

		bool get_visible() const override { return _visible; }
		void set_visible(bool v) override { _set_visible(v); }

		World* get_world() const override { return (World*)_world; }

		Entity* get_parent() const override { return _parent; }

		Component* get_component_plain(uint hash) const override { return _get_component_plain(hash); }
		void add_component(Component * c) override { _add_component(c); }
		void remove_component(Component* c, bool destroy) override { _remove_component(c, destroy); }
		void data_changed(Component* c, uint hash) override { _data_changed(c, hash); }

		uint get_children_count() const override { return _children.size(); }
		Entity* get_child(uint idx) const override { return _children[idx].get(); }
		void add_child(Entity* e, int position) override { _add_child((EntityPrivate*)e, position); }
		void reposition_child(uint pos1, uint pos2) override { _reposition_child(pos1, pos2); }
		void remove_child(Entity* e, bool destroy) override { _remove_child((EntityPrivate*)e, destroy); }

		void load(const wchar_t* filename) override { _load(filename); }
		void save(const wchar_t* filename) override { _save(filename); }
	};
}
