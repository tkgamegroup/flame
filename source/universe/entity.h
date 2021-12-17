#pragma once

#include "universe.h"

namespace flame
{
	struct Entity
	{
		virtual ~Entity() {}

		std::string name;
		uint tag = 0x80000000;

		bool visible = true;
		bool global_visibility = false;

		WorldPtr world = nullptr;
		EntityPtr parent = nullptr;

		uint depth = 0;
		uint index = 0;

		StateFlags state = StateNone;
		StateFlags last_state = StateNone;

		std::unordered_map<uint, std::unique_ptr<Component>> components;
		std::vector<Component*> components_list;
		std::vector<std::unique_ptr<EntityT>> children;

		void* userdata = nullptr;

		virtual void set_visible(bool v) = 0;

		virtual void set_state(StateFlags state) = 0;

		virtual const wchar_t* get_srcs() const = 0;

		inline Component* get_component(uint hash) const
		{
			auto it = components.find(hash);
			if (it != components.end())
				return it->second.get();
			return nullptr;
		}

		template<typename T> 
		inline T* get_component_t() const { return (T*)get_component(T::type_hash); }

		template<typename T> 
		inline T* get_component_i(uint idx) const
		{
			if (idx >= components.size())
				return nullptr;
			auto ret = components_list[idx];
			return ret->type_hash == T::type_hash ? (T*)ret : nullptr;
		}

		template<typename T>
		inline T* get_parent_component_t() const { return parent ? parent->get_component_t<T>() : nullptr; }

		inline Component* find_component(std::string_view _name) const
		{
			Component* ret = nullptr;
			for (auto& c : components)
			{
				if (c.second->type_name == _name)
				{
					ret = c.second.get();
					break;
				}
			}
			auto name = "flame::" + std::string(_name);
			for (auto& c : components)
			{
				if (c.second->type_name == name)
				{
					ret = c.second.get();
					break;
				}
			}
			return ret;
		}

		virtual void add_component(Component* c) = 0;
		virtual void remove_component(Component* c, bool destroy = true) = 0;

		virtual void add_child(EntityPtr e, int position = -1 /* -1 is end */ ) = 0;
		virtual void reposition_child(uint pos1, uint pos2) = 0;
		virtual void remove_child(EntityPtr e, bool destroy = true) = 0;
		virtual void remove_all_children(bool destroy = true) = 0;

		inline EntityPtr find_child(std::string_view name) const
		{
			for (auto& c : children)
			{
				if (c->name == name)
					return c.get();
				auto res = c->find_child(name);
				if (res)
					return res;
			}
			return nullptr;
		}

		virtual void* add_message_listener(const std::function<void(uint msg, void* parm1, void* parm2)>& callback) = 0;
		virtual void remove_message_listener(void* lis) = 0;

		virtual void component_data_changed(Component* c, uint h) = 0;
		virtual void* add_component_data_listener(const std::function<void(uint name_hash)>& callback, Component* c) = 0;
		virtual void remove_component_data_listener(void* lis, Component* c) = 0;

		virtual EntityPtr copy() = 0;
		virtual bool load(const std::filesystem::path& filename) = 0;
		virtual bool save(const std::filesystem::path& filename) = 0;

		FLAME_UNIVERSE_EXPORTS static Entity* create();
	};
}
