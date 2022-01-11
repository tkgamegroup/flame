#pragma once

#include "component.h"

namespace flame
{
	/// Reflect ctor
	struct Entity
	{
		virtual ~Entity() {}

		/// Reflect
		std::string name;
		/// Reflect
		uint tag = 0x80000000;

		/// Reflect
		bool enable = true;
		bool global_enable = false;

		WorldPtr world = nullptr;
		EntityPtr parent = nullptr;

		uint depth = 0;
		uint index = 0;

		StateFlags state = StateNone;
		StateFlags last_state = StateNone;

		std::unordered_map<uint, Component*> component_map;
		/// Reflect
		std::vector<std::unique_ptr<Component>> components;
		/// Reflect
		std::vector<std::unique_ptr<EntityT>> children;

		std::filesystem::path path;

		Listeners<void(uint, void*, void*)> message_listeners;

		void* userdata = nullptr;

		virtual void set_enable(bool v) = 0;

		virtual void set_state(StateFlags state) = 0;

		inline Component* get_component(uint type_hash) const
		{
			auto it = component_map.find(type_hash);
			if (it != component_map.end())
				return it->second;
			return nullptr;
		}

		template<typename T> 
		inline T* get_component_t() const 
		{ 
			return (T*)get_component(th<T>()); 
		}

		template<typename T> 
		inline T* get_component_i(uint idx) const
		{
			if (idx >= components.size())
				return nullptr;
			auto ret = components[idx].get();
			return ret->type_hash == th<T>() ? (T*)ret : nullptr;
		}

		template<typename T>
		inline T* get_parent_component_t() const 
		{ 
			return parent ? ((Entity*)parent)->get_component_t<T>() : nullptr; 
		}

		template<typename T>
		inline T* get_parent_component_i(uint idx) const
		{
			return parent ? ((Entity*)parent)->get_component_i<T>(0) : nullptr;
		}

		virtual Component* add_component(uint hash) = 0;
		virtual void remove_component(uint hash, bool destroy = true) = 0;

		virtual void add_child(EntityPtr e, int position = -1 /* -1 is end */ ) = 0;
		virtual void remove_child(EntityPtr e, bool destroy = true) = 0;
		virtual void remove_all_children(bool destroy = true) = 0;

		inline EntityPtr find_child(std::string_view name) const
		{
			for (auto& cc : children)
			{
				auto c = (Entity*)cc.get();
				if (c->name == name)
					return (EntityPtr)c;
				auto res = c->find_child(name);
				if (res)
					return res;
			}
			return nullptr;
		}

		inline void forward_traversal(const std::function<void(EntityPtr)>& callback)
		{
			callback((EntityPtr)this);
			for (auto& cc : children)
			{
				auto c = (Entity*)cc.get();
				c->forward_traversal(callback);
			}
		}

		inline void backward_traversal(const std::function<void(EntityPtr)>& callback)
		{
			for (auto& cc : children)
			{
				auto c = (Entity*)cc.get();
				c->backward_traversal(callback);
			}
			callback((EntityPtr)this);
		}

		virtual EntityPtr copy() = 0;
		virtual bool load(const std::filesystem::path& filename) = 0;
		virtual bool save(const std::filesystem::path& filename) = 0;

		struct Create
		{
			virtual EntityPtr operator()() = 0;
		};
		/// Reflect
		FLAME_UNIVERSE_EXPORTS static Create& create;
	};
}
