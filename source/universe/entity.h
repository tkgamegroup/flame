#pragma once

#include "component.h"

namespace flame
{
	struct PrefabInstance
	{
		EntityPtr e;
		std::filesystem::path filename;
		std::vector<std::string> modifications;

		inline PrefabInstance(EntityPtr e, const std::filesystem::path& filename);

		void mark_modifier(const std::string& tar_id, const std::string& tar_comp, const std::string& attr_name)
		{
			std::string target;
			target = tar_id;
			if (!tar_comp.empty())
				target = target + "|" + tar_comp;
			target = target + "|" + attr_name;
			for (auto& m : modifications)
			{
				if (m == target)
					return;
			}
			modifications.push_back(target);
		}
	};

	enum TagFlags
	{
		TagNone = 0,
		TagGeneral = 1 << 0,
		TagMarkNavMesh = 1 << 1,
		TagUser = 1 << 2
	};

	// Reflect ctor
	struct Entity
	{
		virtual ~Entity() {}

		// Reflect
		std::string name;
		// Reflect
		TagFlags tag = TagGeneral;

		// Reflect
		bool enable = true;
		bool global_enable = false;
		// Reflect
		virtual void set_enable(bool v) = 0;

		WorldPtr world = nullptr;
		EntityPtr parent = nullptr;

		uint depth = 0;
		uint index = 0;

		std::string instance_id;
		std::string file_id;

		// Reflect
		std::vector<std::unique_ptr<Component>> components;
		std::unordered_map<uint, Component*> component_map;
		// Reflect
		std::vector<std::unique_ptr<EntityT>> children;

		Listeners<void(uint, void*, void*)> message_listeners;

		std::unique_ptr<PrefabInstance> prefab_instance;

		void* userdata = nullptr;

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

		inline cNodePtr node() const
		{
			return (cNodePtr)get_component_i<cNode>(0);
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
		template<typename T>
		inline T* add_component_t()
		{
			return (T*)add_component(th<T>());
		}
		virtual bool remove_component(uint hash) = 0;
		template<typename T>
		inline bool remove_component()
		{
			return remove_component(th<T>());
		}

		virtual bool reposition_component(Component* comp) = 0;

		virtual void add_child(EntityPtr e, int position = -1 /* -1 is end */) = 0;
		virtual void remove_child(EntityPtr e, bool destroy = true) = 0;
		inline void remove_from_parent(bool destroy = true)
		{
			if (parent)
				((Entity*)parent)->remove_child((EntityPtr)this, destroy);
		}
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

		inline Component* find_component(uint hash) const
		{
			for (auto& comp : components)
			{
				if (comp->type_hash == hash)
					return comp.get();
			}
			for (auto& cc : children)
			{
				auto c = (Entity*)cc.get();
				auto res = c->find_component(hash);
				if (res)
					return res;
			}
			return nullptr;
		}

		inline EntityPtr find_child_with_instance_id(std::string_view instance_id) const
		{
			for (auto& cc : children)
			{
				auto c = (Entity*)cc.get();
				if (c->instance_id == instance_id)
					return (EntityPtr)c;
				auto res = c->find_child_with_instance_id(instance_id);
				if (res)
					return res;
			}
			return nullptr;
		}

		inline bool compare_depth(EntityPtr e)
		{
			if (depth < ((Entity*)e)->depth)
				return true;
			if (depth > ((Entity*)e)->depth)
				return false;
			return index < ((Entity*)e)->index;
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

		inline std::vector<EntityPtr> get_all_children()
		{
			std::vector<EntityPtr> ret;
			forward_traversal([&](EntityPtr e) {
				ret.push_back(e);
			});
			return ret;
		}

		virtual EntityPtr copy() = 0;

		virtual bool load(const std::filesystem::path& filename) = 0;
		virtual bool save(const std::filesystem::path& filename) = 0;

		struct Create
		{
			virtual EntityPtr operator()(std::string* file_id = nullptr) = 0;
			virtual EntityPtr operator()(const std::filesystem::path& filename) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};

	PrefabInstance::PrefabInstance(EntityPtr _e, const std::filesystem::path& filename) :
		e(_e),
		filename(filename)
	{
		auto e = (Entity*)_e;
		assert(!e->prefab_instance);
		e->prefab_instance.reset(this);
	}
}
