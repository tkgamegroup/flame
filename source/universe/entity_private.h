#pragma once

#include <flame/universe/entity.h>

namespace flame
{
	struct UdtInfo;

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

		StateFlags state = StateNone;

		struct ComponentWrapper
		{
			std::unique_ptr<Component, Delector> p;
			UdtInfo* udt[2];
			bool want_local_event;
			bool want_child_event;
			bool want_local_data_changed;
			bool want_child_data_changed;
		};
		std::unordered_map<uint64, ComponentWrapper> components;
		std::vector<std::unique_ptr<EntityPrivate, Delector>> children;
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

		StateFlags get_state() const override { return state; }
		void set_state(StateFlags state) override;

		Component* get_component(uint64 hash) const override;
		void add_component(Component* c);
		void info_component_removed(ComponentWrapper& cw) const;
		void remove_component(Component* c, bool destroy = true);
		void remove_all_components(bool destroy) override;
		void report_data_changed(Component* c, uint hash) override;

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
		EntityPrivate* find_child(const std::string& name) const;

		void load(const std::filesystem::path& filename);
		void save(const std::filesystem::path& filename);
	};

	inline void EntityBridge::add_child(Entity* e, int position)
	{
		((EntityPrivate*)this)->add_child((EntityPrivate*)e, position);
	}

	inline void EntityBridge::remove_child(Entity* e, bool destroy)
	{
		((EntityPrivate*)this)->remove_child((EntityPrivate*)e, destroy);
	}

	inline void EntityBridge::load(const wchar_t* filename)
	{
		((EntityPrivate*)this)->load(filename);
	}

	inline void EntityBridge::save(const wchar_t* filename)
	{
		((EntityPrivate*)this)->save(filename);
	}
}
