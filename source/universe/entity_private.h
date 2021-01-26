#pragma once

#include <flame/universe/entity.h>
#include <flame/universe/component.h>
#include <flame/universe/driver.h>

#include <functional>

namespace flame
{
	struct TypeInfo;
	struct UdtInfo;
	struct FunctionInfo;
	struct VariableInfo;

	struct Driver;
	struct WorldPrivate;

	struct StateRule
	{
		void* o;
		std::string vname;
		TypeInfo* type;
		FunctionInfo* setter;
		std::vector<std::pair<StateFlags, void*>> values;

		~StateRule();
	};

	struct EntityBridge : Entity
	{
		Component* find_component(const char* name) const override;
		Component* find_first_dfs_component(const char* name) const override;
		void add_child(Entity* e, int position) override;
		void remove_child(Entity* e, bool destroy) override;
		Entity* find_child(const char* name) const override;
		Driver* find_driver(const char* name) const override;
		void load(const wchar_t* filename) override;
		void save(const wchar_t* filename) override;
	};

	struct EntityPrivate : EntityBridge
	{
		struct ComponentSlot
		{
			std::unique_ptr<Component, Delector> c;
			uint id;
			std::vector<std::unique_ptr<Closure<void(Capture&, uint64)>>> data_listeners;
		};

		struct DriverSlot
		{
			std::unique_ptr<Driver, Delector> d;
			std::vector<std::unique_ptr<Closure<void(Capture&, uint64)>>> data_listeners;
		};

		std::string name;

		bool visible = true;
		bool global_visibility = false;

		WorldPrivate* world = nullptr;
		EntityPrivate* parent = nullptr;

		StateFlags state = StateNone;
		std::vector<std::unique_ptr<StateRule>> state_rules;

		uint depth = 0;
		uint index = 0;

		std::string src;
		std::filesystem::path path;

		int created_frame;
		int created_id;
#ifdef FLAME_UNIVERSE_DEBUG
		std::vector<StackFrameInfo> created_stack;
#endif
		std::pair<std::filesystem::path, uint> created_location;

		std::vector<std::unique_ptr<Driver, Delector>> drivers;
		std::unordered_map<uint64, ComponentSlot> components;
		uint component_id = 0;
		std::vector<std::unique_ptr<EntityPrivate, Delector>> children;

		std::vector<void*> events;

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
		uint get_index() const override { return index; }

		StateFlags get_state() const override { return state; }
		void set_state(StateFlags state) override;

		Component* get_component(uint64 hash) const override;
		Component* find_component(const std::string& name) const;
		Component* find_first_dfs_component(const std::string& name) const;
		template <class T> inline T* get_parent_component_t() const { return !parent ? nullptr : parent->get_component_t<T>(); }
		void traversal(const std::function<bool(EntityPrivate*)>& callback);
		void add_component(Component* c);
		void remove_component(Component* c, bool destroy = true);
		void remove_all_components(bool destroy) override;

		uint get_children_count() const override { return children.size(); }
		Entity* get_child(uint idx) const override { return children[idx].get(); }
		void add_child(EntityPrivate* e, int position = -1);
		void reposition_child(uint pos1, uint pos2) override;
		void on_child_removed(EntityPrivate* e) const;
		void remove_child(EntityPrivate* e, bool destroy = true);
		void remove_all_children(bool destroy) override;
		EntityPrivate* find_child(const std::string& name) const;

		Driver* get_driver(uint64 hash = 0, uint idx = 0) const override;
		Driver* find_driver(const std::string& name) const;

		void data_changed(Component* c, uint64 h) override;
		void* add_data_listener(void (*callback)(Capture& c, uint64 hash), const Capture& capture, Component* c) override;
		void remove_data_listener(void* lis, Component* c) override;

		void* add_event(void (*callback)(Capture& c), const Capture& capture, float interval = 0.f) override;
		void remove_event(void* ev) override;

		void load(const std::filesystem::path& filename);
		void save(const std::filesystem::path& filename);
	};

	inline Component* EntityBridge::find_component(const char* name) const
	{
		return ((EntityPrivate*)this)->find_component(name);
	}

	inline Component* EntityBridge::find_first_dfs_component(const char* name) const
	{
		return ((EntityPrivate*)this)->find_first_dfs_component(name);
	}

	inline void EntityBridge::add_child(Entity* e, int position)
	{
		((EntityPrivate*)this)->add_child((EntityPrivate*)e, position);
	}

	inline Entity* EntityBridge::find_child(const char* name) const
	{
		return ((EntityPrivate*)this)->find_child(name);
	}

	inline void EntityBridge::remove_child(Entity* e, bool destroy)
	{
		((EntityPrivate*)this)->remove_child((EntityPrivate*)e, destroy);
	}

	inline Driver* EntityBridge::find_driver(const char* name) const
	{
		return ((EntityPrivate*)this)->find_driver(name);
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
