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
		void add_child(Entity* e, int position) override;
		void remove_child(Entity* e, bool destroy) override;
		Entity* find_child(const char* name) const override;
		Driver* find_driver(const char* name) const override;
		bool load(const wchar_t* filename) override;
		bool save(const wchar_t* filename) override;
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
		bool redirectable = true;

		StateFlags state = StateNone;
		std::vector<std::unique_ptr<StateRule>> state_rules;

		uint depth = 0;
		uint index = 0;

		std::vector<std::filesystem::path> srcs;
		std::wstring srcs_str;

		int created_frame;
		int created_id;
#ifdef FLAME_UNIVERSE_DEBUG
		std::vector<StackFrameInfo> created_stack;
#endif
		uint created_location;

		std::vector<DriverSlot> drivers;
		std::unordered_map<uint64, ComponentSlot> components;
		uint component_id = 0;
		std::vector<std::unique_ptr<EntityPrivate, Delector>> children;

		std::vector<void*> events;

		void* userdata = nullptr;

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

		void add_src(const std::filesystem::path& p);
		const wchar_t* get_srcs() const override;

		Component* get_component(uint64 hash) const override;
		Component* find_component(const std::string& name) const;
		template <class T> inline T* get_parent_component_t() const { return !parent ? nullptr : parent->get_component_t<T>(); }
		void get_components(void (*callback)(Capture& c, Component* cmp), const Capture& capture) const override;
		void add_component(Component* c);
		void remove_component(Component* c, bool destroy = true);

		uint get_children_count() const override { return children.size(); }
		Entity* get_child(uint idx) const override { fassert(idx < children.size()); return children[idx].get(); }
		void add_child(EntityPrivate* e, int position = -1);
		void reposition_child(uint pos1, uint pos2) override;
		void on_child_removed(EntityPrivate* e) const;
		void remove_child(EntityPrivate* e, bool destroy = true);
		EntityPrivate* find_child(const std::string& name) const;

		void traversal(const std::function<bool(EntityPrivate*)>& callback);

		Driver* get_driver(uint64 hash, int idx = -1) const override;
		Driver* find_driver(const std::string& name) const;

		void push_driver(Driver* d) override;
		void pop_driver() override;

		void component_data_changed(Component* c, uint64 h) override;
		void* add_component_data_listener(void (*callback)(Capture& c, uint64 hash), const Capture& capture, Component* c) override;
		void remove_component_data_listener(void* lis, Component* c) override;

		void driver_data_changed(Driver* d, uint64 h) override;
		void* add_driver_data_listener(void (*callback)(Capture& c, uint64 hash), const Capture& capture, Driver* d) override;
		void remove_driver_data_listener(void* lis, Driver* d) override;

		void* add_event(void (*callback)(Capture& c), const Capture& capture, float interval = 0.f) override;
		void remove_event(void* ev) override;

		bool load(const std::filesystem::path& filename);
		bool save(const std::filesystem::path& filename);

		void* get_userdata() const override { return userdata; }
		void set_userdata(void* d) override { userdata = d; }
	};

	inline Component* EntityBridge::find_component(const char* name) const
	{
		return ((EntityPrivate*)this)->find_component(name);
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

	inline bool EntityBridge::load(const wchar_t* filename)
	{
		return ((EntityPrivate*)this)->load(filename);
	}

	inline bool EntityBridge::save(const wchar_t* filename)
	{
		return ((EntityPrivate*)this)->save(filename);
	}
}
