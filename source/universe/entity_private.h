#pragma once

#include "entity.h"
#include "component.h"
#include "driver.h"

#include <functional>

namespace flame
{
	struct StateRule
	{
		void* o;
		std::string vname;
		TypeInfo* type;
		FunctionInfo* setter;
		std::vector<std::pair<StateFlags, void*>> values;

		~StateRule();
	};

	struct EntityPrivate : Entity
	{
		typedef std::vector<std::unique_ptr<Closure<void(Capture&, uint)>>> DataListeners;

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

		std::vector<std::unique_ptr<Driver, Delector>> drivers;
		std::unordered_map<uint, std::pair<Driver*, DataListeners>> drivers_map;
		std::vector<std::unique_ptr<Component, Delector>> components;
		std::unordered_map<uint, std::pair<Component*, DataListeners>> components_map;
		std::vector<std::unique_ptr<EntityPrivate, Delector>> children;

		std::vector<std::unique_ptr<Closure<void(Capture&, uint, void*, void*)>>> message_listeners;

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

		WorldPtr get_world() const override { return world; }
		EntityPtr get_parent() const override { return parent; }
		uint get_index() const override { return index; }

		StateFlags get_state() const override { return state; }
		void set_state(StateFlags state) override;

		void add_src(const std::filesystem::path& p);
		const wchar_t* get_srcs() const override;
		std::filesystem::path get_src(uint id) const { return srcs[srcs.size() - id - 1]; }

		Component* get_component(uint hash) const override;
		template <class T> inline T* get_component_i(uint idx) const 
		{
			if (idx >= components.size())
				return nullptr;
			auto ret = components[idx].get();
			return ret->type_hash == T::type_hash ? (T*)ret : nullptr; 
		}
		Component* find_component(const std::string& name) const;
		Component* find_component(const char* name) const override { return find_component(std::string(name)); }
		template <class T> inline T* get_parent_component_t() const { return !parent ? nullptr : parent->get_component_t<T>(); }
		void get_components(void (*callback)(Capture& c, Component* cmp), const Capture& capture) const override;
		void add_component(Component* c);
		void remove_component(Component* c, bool destroy = true);

		uint get_children_count() const override { return children.size(); }
		EntityPtr get_child(uint idx) const override { fassert(idx < children.size()); return children[idx].get(); }
		void add_child(EntityPtr e, int position = -1) override;
		void reposition_child(uint pos1, uint pos2) override;
		void on_child_removed(EntityPrivate* e) const;
		void remove_child(EntityPtr e, bool destroy = true) override;
		void remove_all_children(bool destroy = true) override;
		EntityPrivate* find_child(const std::string& name) const;
		EntityPtr find_child(const char* name) const override { return find_child(std::string(name)); }

		void on_entered_world(WorldPrivate* world);
		void on_left_world();

		void traversal(const std::function<bool(EntityPrivate*)>& callback);

		Driver* get_driver(uint hash, int idx = -1) const override;
		Driver* find_driver(const std::string& name) const;
		Driver* find_driver(const char* name) const override { return find_driver(std::string(name)); }

		void push_driver(Driver* d) override;
		void pop_driver() override;

		void* add_message_listener(void (*callback)(Capture& c, uint msg, void* parm1, void* parm2), const Capture& capture) override;
		void remove_message_listener(void* lis) override;

		void component_data_changed(Component* c, uint h) override;
		void* add_component_data_listener(void (*callback)(Capture& c, uint hash), const Capture& capture, Component* c) override;
		void remove_component_data_listener(void* lis, Component* c) override;

		void driver_data_changed(Driver* d, uint h) override;
		void* add_driver_data_listener(void (*callback)(Capture& c, uint hash), const Capture& capture, Driver* d) override;
		void remove_driver_data_listener(void* lis, Driver* d) override;

		void* add_event(void (*callback)(Capture& c), const Capture& capture, float interval = 0.f) override;
		void remove_event(void* ev) override;

		bool load(const std::filesystem::path& filename);
		bool load(const wchar_t* filename) override { return load(std::filesystem::path(filename)); }
		bool save(const std::filesystem::path& filename);
		bool save(const wchar_t* filename) override { return save(std::filesystem::path(filename)); }

		void* get_userdata() const override { return userdata; }
		void set_userdata(void* d) override { userdata = d; }
	};
}
