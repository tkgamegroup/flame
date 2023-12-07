#pragma once

#include "component.h"

namespace flame
{
	enum ModificationType
	{
		ModificationWrong,
		ModificationAttributeModify,
		ModificationEntityAdd,
		ModificationComponentAdd,
		ModificationComponentRemove
	};

	struct ModificationParsedData
	{
		union
		{
			const Attribute* attr;
			uint hash;
			GUID guid;
		}d;
	};

	struct PrefabInstance
	{
		EntityPtr e;
		std::filesystem::path filename;
		std::vector<std::string> modifications;

		inline PrefabInstance(EntityPtr e, const std::filesystem::path& filename);

		int find_modification(const std::string& target_string)
		{
			for (auto i = 0; i < modifications.size(); i++)
			{
				if (modifications[i] == target_string)
					return i;
			}
			return -1;
		}

		void mark_modification(const std::string& target_string)
		{
			if (find_modification(target_string) == -1)
				modifications.push_back(target_string);
		}

		void remove_modification(const std::string& target_string)
		{
			auto idx = find_modification(target_string);
			if (idx != -1)
				modifications.erase(modifications.begin() + idx);
		}
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
		uint layer = 1;

		// Reflect
		bool enable = true;
		bool global_enable = false;
		// Reflect
		virtual void set_enable(bool v) = 0;

		EntityPtr parent = nullptr;

		ushort depth = (ushort)-1;
		ushort index = 0;

		GUID instance_id;
		GUID file_id;

		// Reflect
		std::vector<std::unique_ptr<Component>> components;
		// Reflect
		std::vector<std::unique_ptr<EntityT>> children;

		Listeners<void(uint, void*, void*)> message_listeners;

		std::unique_ptr<PrefabInstance> prefab_instance;

		inline Component* get_component_h(uint type_hash) const
		{
			for (auto& c : components)
			{
				if (c->type_hash == type_hash)
					return c.get();
			}
			return nullptr;
		}

		template<typename T>
		inline T* get_component() const
		{
			return (T*)get_component_h(th<T>());
		}

		template<typename T>
		inline T* get_parent_component() const
		{
			return parent ? ((Entity*)parent)->get_component<T>() : nullptr;
		}

		template<typename T>
		std::vector<T*> get_components(uint lv) const
		{
			std::vector<T*> ret;
			if (auto comp = get_component<T>(); comp)
				ret.push_back(comp);
			if (lv > 0)
			{
				for (auto& cc : children)
				{
					auto c = (Entity*)cc.get();
					auto vec = c->get_components<T>(lv - 1);
					ret.insert(ret.end(), vec.begin(), vec.end());
				}
			}
			return ret;
		}

		virtual Component* add_component_h(uint hash) = 0;
		template<typename T>
		inline T* add_component()
		{
			return (T*)add_component_h(th<T>());
		}
		virtual bool remove_component_h(uint hash) = 0;
		template<typename T>
		inline bool remove_component()
		{
			return remove_component_h(th<T>());
		}
		virtual void remove_all_components() = 0;
		virtual bool reposition_component(uint comp_index, int new_index) = 0;

		virtual void add_child(EntityPtr e, int position = -1 /* -1 is end */) = 0;
		virtual void remove_child(EntityPtr e, bool destroy = true) = 0;
		inline void remove_from_parent(bool destroy = true)
		{
			if (parent)
				((Entity*)parent)->remove_child((EntityPtr)this, destroy);
		}
		virtual void remove_all_children(bool destroy = true) = 0;

		inline int find_child_i(std::string_view name) const
		{
			for (auto i = 0; i < children.size(); i++)
			{
				auto c = (Entity*)children[i].get();
				if (c->name == name)
					return i;
			}
			return -1;
		}

		inline EntityPtr find_child(std::string_view name) const
		{
			auto idx = find_child_i(name);
			if (idx != -1)
				return (EntityPtr)children[idx].get();
			return nullptr;
		}

		inline EntityPtr find_child_recursively(std::string_view name) const
		{
			for (auto& cc : children)
			{
				auto c = (Entity*)cc.get();
				if (c->name == name)
					return (EntityPtr)c;
				auto res = c->find_child_recursively(name);
				if (res)
					return res;
			}
			return nullptr;
		}

		inline int find_component_i(uint hash) const
		{
			for (auto i = 0; i < components.size(); i++)
			{
				if (components[i]->type_hash == hash)
					return i;
			}
			return -1;
		}

		inline Component* find_component(uint hash) const
		{
			auto idx = find_component_i(hash);
			return idx == -1 ? nullptr : components[idx].get();
		}

		inline Component* find_component_recursively(uint hash) const
		{
			for (auto& comp : components)
			{
				if (comp->type_hash == hash)
					return comp.get();
			}
			for (auto& cc : children)
			{
				auto c = (Entity*)cc.get();
				auto res = c->find_component_recursively(hash);
				if (res)
					return res;
			}
			return nullptr;
		}

		inline EntityPtr find_with_instance_id(const GUID& guid) const
		{
			if (instance_id == guid)
				return (EntityPtr)this;
			for (auto& c : children)
			{
				auto res = ((Entity*)c.get())->find_with_instance_id(guid);
				if (res)
					return res;
			}
			return nullptr;
		}

		inline EntityPtr find_with_file_id(const GUID& guid) const
		{
			if (file_id == guid)
				return (EntityPtr)this;
			for (auto& c : children)
			{
				auto res = ((Entity*)c.get())->find_with_file_id(guid);
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

		inline void traversal_bfs(const std::function<bool(EntityPtr, int depth)>& callback)
		{
			std::deque<Entity*> queue;
			queue.push_back((Entity*)this);
			if (!callback((EntityPtr)this, 0))
				return;
			auto depth = 1;
			while (!queue.empty())
			{
				auto e = queue.front();
				queue.pop_front();
				for (auto& cc : e->children)
				{
					auto c = (Entity*)cc.get();
					if (callback((EntityPtr)c, depth++))
						queue.push_back(c);
					else
						return;
				}
			}
		}

		inline std::vector<EntityPtr> get_all_children()
		{
			std::vector<EntityPtr> ret;
			forward_traversal([&](EntityPtr e) {
				ret.push_back(e);
			});
			return ret;
		}

		virtual EntityPtr duplicate(EntityPtr dst = nullptr) = 0;

		virtual ModificationType parse_modification_target(const std::string& target, ModificationParsedData& out, voidptr& obj) = 0;
		virtual bool load(const std::filesystem::path& filename, bool only_root = false) = 0;
		virtual bool save(const std::filesystem::path& filename, bool only_root = false) = 0;

		struct Create
		{
			virtual EntityPtr operator()(GUID* file_id = nullptr) = 0;
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

	inline void resolve_address(const std::string& address, Entity* e, const Attribute*& attr, voidptr& obj, uint& index)
	{
		auto sp = SUS::split(address, '|');

		auto ui = TypeInfo::get<Entity>()->retrive_ui();
		obj = e;
		if (sp.size() > 1)
		{
			if (auto entity_chain = SUS::split(sp.front(), '.'); !entity_chain.empty())
			{
				auto i = 0;
				while (true)
				{
					auto e1 = (Entity*)e->find_child(entity_chain[i]);
					if (!e1)
					{
						if (i == entity_chain.size() - 1)
						{
							auto comp_hash = sh(entity_chain[i].data());
							if (auto comp = e->find_component(comp_hash); comp)
							{
								ui = find_udt(comp_hash);
								obj = comp;
								break;
							}
						}
						return;
					}

					e = e1;

					i++;
					if (i >= entity_chain.size())
						break;
				}
			}
		}

		if (!sp.empty())
			attr = ui->find_attribute(SUS::split(sp.back(), '.'), obj, &index);
	}
}
