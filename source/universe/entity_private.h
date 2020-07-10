#pragma once

#include <flame/universe/entity.h>
#include "universe_private.h"

namespace flame
{
	struct WorldPrivate;

	struct EntityBridge : Entity
	{
		void add_child(Entity* e, int position) override;
		void remove_child(Entity* e, bool destroy) override;
		void load(const wchar_t* filename) override;
		void save(const wchar_t* filename) override;
	};

	struct EntityPrivate : EntityBridge
	{
		std::string name;

		bool visible = true;
		bool global_visibility = false;

		void* gene = nullptr;

		WorldPrivate* world = nullptr;

		EntityPrivate* parent = nullptr;
		std::unordered_map<uint64, std::unique_ptr<Component, Delecter>> components;
		std::vector<std::unique_ptr<EntityPrivate, Delecter>> children;
		std::vector<Component*> local_event_dispatch_list;
		std::vector<Component*> child_event_dispatch_list;
		std::vector<Component*> local_data_changed_dispatch_list;
		std::vector<Component*> child_data_changed_dispatch_list;

		uint depth = 0;
		uint index = 0;
		int _created_frame;
		std::vector<void*> created_stack;

		EntityPrivate();
		~EntityPrivate();

		void release() override;

		const char* get_name() const override { return name.c_str(); };
		void set_name(const char* _name) override { name = _name; }

		bool get_visible() const override { return visible; }
		void update_visibility();
		void set_visible(bool v) override;

		World* get_world() const override { return (World*)world; }

		Entity* get_parent() const override { return parent; }

		Component* get_component(uint64 hash) const override;
		void add_component(Component* c);
		void info_component_removed(Component* c) const;
		void remove_component(Component* c, bool destroy = true);
		void remove_all_components(bool destroy) override;
		void data_changed(Component* c, uint hash) override;

		uint get_children_count() const override { return children.size(); }
		Entity* get_child(uint idx) const override { return children[idx].get(); }
		void enter_world();
		void leave_world();
		void inherit();
		void add_child(EntityPrivate* e, int position = -1);
		void reposition_child(uint pos1, uint pos2) override;
		void info_child_removed(EntityPrivate* e) const;
		void remove_child(EntityPrivate* e, bool destroy = true);
		void remove_all_children(bool destroy) override;

		void load(const std::filesystem::path& filename);
		void save(const std::filesystem::path& filename);

		static EntityPrivate* create();
	};
}
