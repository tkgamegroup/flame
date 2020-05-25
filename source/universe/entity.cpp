#include <flame/foundation/typeinfo.h>
#include <flame/universe/entity.h>
#include "world_private.h"

namespace flame
{
	void EntityPrivate::set_visible(bool v)
	{
		if (visible_ == v)
			return;
		visible_ = v;
		update_visibility();
	}

	Component* EntityPrivate::get_component_plain(uint hash)
	{
		auto it = components.find(hash);
		if (it == components.end())
			return nullptr;
		return it->second.get();
	}

	Array<Component*> EntityPrivate::get_components()
	{
		auto ret = Array<Component*>();
		ret.resize(components.size());
		auto i = 0;
		for (auto& c : components)
		{
			ret[i] = c.second.get();
			i++;
		}
		return ret;
	}

	void EntityPrivate::add_component(Component* c)
	{
		auto hash = c->name_hash;
		if (c->id)
			hash = hash_update(hash, c->id);
		assert(!get_component_plain(hash));

		c->entity = this;
		if (world)
			c->on_entered_world();
		for (auto& _c : components)
			_c.second->on_component_added(c);
		for (auto& _c : components)
			c->on_component_added(_c.second.get());
		components[hash].reset(c);
		c->on_added();
		if (parent)
		{
			for (auto& _c : ((EntityPrivate*)parent)->components)
				_c.second->on_child_component_added(c);
		}
	}

	void EntityPrivate::remove_component(Component* c)
	{
		auto it = components.find(c->name_hash);
		if (it != components.end())
		{
			for (auto& _c : components)
			{
				if (_c.second.get() != c)
					_c.second->on_component_removed(c);
			}
			if (parent)
			{
				for (auto& _c : ((EntityPrivate*)parent)->components)
					_c.second->on_child_component_removed(c);
			}
			components.erase(it);
		}
	}

	EntityPrivate* EntityPrivate::find_child(const std::string& name) const
	{
		for (auto& e : children)
		{
			if (e->name == name)
				return e.get();
		}
		return nullptr;
	}

	int EntityPrivate::find_child(EntityPrivate* e) const
	{
		for (auto i = 0; i < children.size(); i++)
		{
			if (children[i].get() == e)
				return i;
		}
		return -1;
	}

	static void enter_world(World* w, EntityPrivate* e)
	{
		e->world = (WorldPrivate*)w;
		for (auto& c : e->components)
			c.second->on_entered_world();
		for (auto& c : e->children)
			enter_world(w, c.get());
	}

	static void leave_world(EntityPrivate* e)
	{
		for (auto it = e->children.rbegin(); it != e->children.rend(); it++)
			leave_world(it->get());
		e->world = nullptr;
		for (auto& c : e->components)
			c.second->on_left_world();
	}

	static void inherit(EntityPrivate* e, void* gene)
	{
		if (!gene)
			return;
		e->gene = gene;
		for (auto& c : e->children)
			inherit(c.get(), gene);
	}

	void EntityPrivate::add_child(EntityPrivate* e, int position)
	{
		if (position == -1)
			position = children.size();
		for (auto i = position; i < children.size(); i++)
			children[i]->index_ += 1;
		children.emplace(children.begin() + position, e);
		inherit(e, gene);
		e->depth_ = depth_ + 1;
		e->index_ = position;
		e->parent = this;
		if (!e->world && world)
			enter_world(world, e);
		if (world)
			e->update_visibility();
		for (auto& c : components)
		{
			for (auto& _c : e->components)
				c.second->on_child_component_added(_c.second.get());
		}
		for (auto& c : e->components)
			c.second->on_added();
	}

	void EntityPrivate::reposition_child(EntityPrivate* e, int position)
	{
		if (position == -1)
			position = children.size() - 1;
		assert(position < children.size());
		auto old_position = e->index_;
		if (old_position == position)
			return;
		auto dst = children[position].get();
		std::swap(children[old_position], children[position]);
		dst->index_ = old_position;
		e->index_ = position;
		for (auto& c : e->components)
			c.second->on_position_changed();
		for (auto& c : components)
			c.second->on_child_position_changed();
	}

	void EntityPrivate::mark_dying()
	{
		dying_ = true;
		for (auto& e : children)
			e->mark_dying();
	}

	void EntityPrivate::remove_child(EntityPrivate* e, bool destroy)
	{
		for (auto it = children.begin(); it != children.end(); it++)
		{
			if (it->get() == e)
			{
				for (auto _it = it + 1; _it != children.end(); _it++)
					(*_it)->index_ -= 1;
				for (auto& c : components)
				{
					for (auto& _c : e->components)
						c.second->on_child_component_removed(_c.second.get());
				}
				e->on_removed_listeners.call();
				leave_world(e);
				if (!destroy)
				{
					e->parent = nullptr;
					it->release();
				}
				else
					e->mark_dying();
				children.erase(it);
				return;
			}
		}
	}

	void EntityPrivate::remove_children(int from, int to, bool destroy)
	{
		for (auto& c : components)
		{
			for (auto& e : children)
			{
				for (auto& _c : e->components)
					c.second->on_child_component_removed(_c.second.get());
			}
		}
		if (to == -1)
			to = children.size() - 1;
		auto count = to - from + 1;
		for (auto i = 0; i < count; i++)
		{
			auto& e = children[from];
			e->on_removed_listeners.call();
			leave_world(e.get());
			if (!destroy)
			{
				e->parent = nullptr;
				e.release();
			}
			else
				e->mark_dying();
			children.erase(children.begin() + from);
		}
	}

	void EntityPrivate::update_visibility()
	{
		auto prev_visibility = global_visibility;
		if (!parent)
			global_visibility = visible_;
		else
			global_visibility = visible_ && parent->global_visibility;
		if (global_visibility != prev_visibility)
		{
			for (auto& c : components)
				c.second->on_visibility_changed();
			if (parent)
			{
				for (auto& c : ((EntityPrivate*)parent)->components)
					c.second->on_child_visibility_changed();
			}
		}

		for (auto& e : children)
			e->update_visibility();
	}

	Entity::Entity()
	{
		on_removed_listeners.impl = ListenerHubImpl::create();
		on_destroyed_listeners.impl = ListenerHubImpl::create();
		event_listeners.impl = ListenerHubImpl::create();

		gene = nullptr;

		depth_ = 0;
		index_ = 0;
		created_frame_ = looper().frame;
		dying_ = false;

		visible_ = true;
		global_visibility = false;

		world = nullptr;
		parent = nullptr;

#ifdef _DEBUG
		created_stack_frames_ = get_stack_frames();
#endif
	}

	Entity::~Entity()
	{
		for (auto& c : children)
			c->on_removed_listeners.call();
		on_destroyed_listeners.call();
		ListenerHubImpl::destroy(on_removed_listeners.impl);
		ListenerHubImpl::destroy(on_destroyed_listeners.impl);
		ListenerHubImpl::destroy(event_listeners.impl);
	}

	void Entity::set_visible(bool v)
	{
		((EntityPrivate*)this)->set_visible(v);
	}

	Component* Entity::get_component_plain(uint name_hash) const
	{
		return ((EntityPrivate*)this)->get_component_plain(name_hash);
	}

	Array<Component*> Entity::get_components() const
	{
		return ((EntityPrivate*)this)->get_components();
	}

	void Entity::add_component(Component* c)
	{
		((EntityPrivate*)this)->add_component(c);
	}

	void Entity::remove_component(Component* c)
	{
		((EntityPrivate*)this)->remove_component(c);
	}

	uint Entity::child_count() const
	{
		return ((EntityPrivate*)this)->children.size();
	}

	Entity* Entity::child(uint index) const
	{
		return ((EntityPrivate*)this)->children[index].get();
	}

	Entity* Entity::find_child(const char* name) const
	{
		return ((EntityPrivate*)this)->find_child(name);
	}

	int Entity::find_child(Entity* e) const
	{
		return ((EntityPrivate*)this)->find_child((EntityPrivate*)e);
	}

	void Entity::add_child(Entity* e, int position)
	{
		((EntityPrivate*)this)->add_child((EntityPrivate*)e, position);
	}

	void Entity::reposition_child(Entity* e, int position)
	{
		((EntityPrivate*)this)->reposition_child((EntityPrivate*)e, position);
	}

	void Entity::remove_child(Entity* e, bool destroy)
	{
		((EntityPrivate*)this)->remove_child((EntityPrivate*)e, destroy);
	}

	void Entity::remove_children(int from, int to, bool destroy)
	{
		((EntityPrivate*)this)->remove_children(from, to, destroy);
	}

	Entity* Entity::create()
	{
		return new EntityPrivate;
	}

	static EntityPrivate* load_prefab(World* w, pugi::xml_node src)
	{
		auto e = new EntityPrivate;
		e->name = src.attribute("name").value();
		e->visible_ = src.attribute("visible").as_bool();

		for (auto n_c : src.child("components"))
		{
			auto udt = find_udt(FLAME_HASH((std::string("flame::") + n_c.name()).c_str()));
			assert(udt && udt->base_name.str() == "Component");
			auto library = udt->db->library;
			auto f = udt->find_function("create");
			assert(f);
			auto component = cf(p2f<F_vp_v>((char*)library + (uint)f->rva));
			for (auto n_v : n_c)
			{
				auto v = udt->find_variable(n_v.name());
				auto type = v->type;
				auto p = (char*)component + v->offset;
				if (type->tag == TypePointer)
				{
					auto& s = type->base_name;
					auto name = s.v;
					auto len = s.s;
					for (auto i = len - 1; i >= 0; i--)
					{
						if (name[i] == ':')
						{
							name = name + i + 1;
							break;
						}
					}
					*(Object**)p = w->find_object(FLAME_HASH(name), n_v.attribute("v").as_uint());
				}
				else
					type->unserialize(n_v.attribute("v").value(), p);
			}
			e->add_component((Component*)component);
		}

		for (auto n_e : src.child("children"))
			e->add_child(load_prefab(w, n_e), -1);

		return e;
	}

	Entity* Entity::create_from_file(World* w, const wchar_t* filename)
	{
		pugi::xml_document file;
		pugi::xml_node file_root;
		if (!file.load_file(filename) || (file_root = file.first_child()).name() != std::string("prefab"))
			return nullptr;

		return load_prefab(w, file_root.first_child());
	}

	static void save_prefab(pugi::xml_node dst, EntityPrivate* src)
	{
		auto n = dst.append_child("entity");
		n.append_attribute("name").set_value(src->name.s ? "unnamed" : src->name.v);
		n.append_attribute("visible").set_value(src->visible_ );

		if (!src->components.empty())
		{
			auto n_cs = n.append_child("components");
			for (auto& component : src->components)
			{
				auto n_c = n_cs.append_child(component.second->name);

				auto udt = find_udt(FLAME_HASH((std::string("flame::") + component.second->name).c_str()));
				assert(udt && udt->base_name.str() == "Component");
				for (auto v : udt->variables)
				{
					auto type = v->type;
					auto p = (char*)component.second.get() + v->offset;
					if (type->tag == TypePointer)
						n_c.append_child(v->name.v).append_attribute("v").set_value((*(Object**)p)->id);
					else
					{
						auto dv = v->default_value;
						if (!dv || memcmp(dv, p, v->size) != 0)
							n_c.append_child(v->name.v).append_attribute("v").set_value(type->serialize(p).c_str());
					}
				}
			}
		}

		if (!src->children.empty())
		{
			auto n_es = n.append_child("children");
			for (auto& e : src->children)
				save_prefab(n_es, e.get());
		}
	}

	void Entity::save_to_file(Entity* e, const wchar_t* filename)
	{
		pugi::xml_document file;
		auto file_root = file.append_child("prefab");

		save_prefab(file_root, (EntityPrivate*)e);

		file.save_file(filename);
	}

	void Entity::destroy(Entity* _e)
	{
		auto e = (EntityPrivate*)_e;
		e->mark_dying();
		delete e;
	}
}
