#include <flame/serialize.h>
#include "entity_private.h"

namespace flame
{
	EntityPrivate::EntityPrivate()
	{
		on_removed_listeners.impl = ListenerHubImpl::create();

		order_ = 0;
		created_frame_ = looper().frame;
		dying_ = false;

		visibility_ = true;
		global_visibility_ = true;

		name_hash = 0;

		world_ = nullptr;
		parent = nullptr;
	}

	EntityPrivate::~EntityPrivate()
	{
		ListenerHubImpl::destroy(on_removed_listeners.impl);
		for (auto& r : resources)
			r.second(r.first);
	}

	void EntityPrivate::set_visibility(bool v)
	{
		visibility_ = v;
		for (auto& c : components)
			c.second->on_visibility_changed();
		if (parent)
		{
			for (auto& c : parent->components)
				c.second->on_child_visibility_changed();
		}
	}

	Component* EntityPrivate::get_component_plain(uint name_hash)
	{
		auto it = components.find(name_hash);
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
			ret.v[i] = c.second.get();
			i++;
		}
		return ret;
	}

	void EntityPrivate::add_component(Component* c)
	{
		assert(!get_component_plain(c->name_hash));

		c->entity = this;
		if (world_)
			c->on_entered_world();
		for (auto& _c : components)
			_c.second->on_component_added(c);
		for (auto& _c : components)
			c->on_component_added(_c.second.get());
		components[c->name_hash].reset(c);
		c->on_added();
		if (parent)
		{
			for (auto& _c : parent->components)
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
				for (auto& _c : parent->components)
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

	static void enter_world(World* w, EntityPrivate* e)
	{
		e->world_ = w;
		for (auto& c : e->components)
			c.second->on_entered_world();
		for (auto& c : e->children)
			enter_world(w, c.get());
	}

	static void leave_world(EntityPrivate* e)
	{
		e->world_ = nullptr;
		for (auto& c : e->components)
			c.second->on_left_world();
		for (auto& c : e->children)
			leave_world(c.get());
	}

	void EntityPrivate::add_child(EntityPrivate* e, int position)
	{
		if (position == -1)
			position = children.size();
		for (auto i = position; i < children.size(); i++)
			children[i]->order_ += 1;
		children.insert(children.begin() + position, std::unique_ptr<EntityPrivate>(e));
		e->order_ = (order_ & 0xff000000) + (1 << 24) + position;
		e->parent = this;
		if (!e->world_ && world_)
			enter_world(world_, e);
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
		auto old_position = e->order_ & 0xffffff;
		if (old_position == position)
			return;
		auto dst = children[position].get();
		std::swap(children[old_position], children[position]);
		dst->order_ = (dst->order_ & 0xff000000) + old_position;
		e->order_ = (e->order_ & 0xff000000) + position;
		for (auto& c : e->components)
			c.second->on_position_changed();
		for (auto& c : components)
			c.second->on_child_position_changed(e);
	}

	void EntityPrivate::mark_dying()
	{
		dying_ = true;
		for (auto& e : children)
			e->mark_dying();
	}

	void EntityPrivate::remove_child(EntityPrivate* e, bool destroy)
	{
		if (e == INVALID_POINTER)
		{
			for (auto& c : components)
			{
				for (auto& e : children)
				{
					for (auto& _c : e->components)
						c.second->on_child_component_removed(_c.second.get());
				}
			}
			for (auto& e : children)
			{
				e->on_removed_listeners.call(e.get());
				leave_world(e.get());
				if (!destroy)
				{
					e->parent = nullptr;
					e.release();
				}
				else
					e->mark_dying();
			}
			children.clear();
		}
		else
		{
			for (auto it = children.begin(); it != children.end(); it++)
			{
				if (it->get() == e)
				{
					for (auto _it = it + 1; _it != children.end(); _it++)
						(*_it)->order_ -= 1;
					for (auto& c : components)
					{
						for (auto& _c : e->components)
							c.second->on_child_component_removed(_c.second.get());
					}
					e->on_removed_listeners.call(e);
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
	}

	EntityPrivate* EntityPrivate::copy()
	{
		auto ret = new EntityPrivate;

		ret->visibility_ = visibility_;
		ret->set_name(name.c_str());
		for (auto& c : components)
			ret->add_component(c.second->copy());
		for (auto& e : children)
			ret->add_child(e->copy(), -1);

		return ret;
	}

	void EntityPrivate::update_visibility()
	{
		if (!parent)
			global_visibility_ = visibility_;
		else
			global_visibility_ = visibility_ && parent->global_visibility_;

		for (auto& e : children)
			e->update_visibility();
	}

	const char* Entity::name() const
	{
		return ((EntityPrivate*)this)->name.c_str();
	}

	uint Entity::name_hash() const
	{
		return ((EntityPrivate*)this)->name_hash;
	}

	void Entity::set_name(const char* name) const
	{
		auto thiz = ((EntityPrivate*)this);
		thiz->name = name;
		thiz->name_hash = FLAME_HASH(name);
	}

	void Entity::set_visibility(bool v)
	{
		((EntityPrivate*)this)->set_visibility(v);
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

	Entity* Entity::parent() const
	{
		return ((EntityPrivate*)this)->parent;
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

	Entity* Entity::copy()
	{
		return ((EntityPrivate*)this)->copy();
	}

	void Entity::associate_resource(void* res, void(*deleter)(void* res))
	{
		((EntityPrivate*)this)->resources.emplace_back(res, deleter);
	}

	Entity* Entity::create()
	{
		return new EntityPrivate;
	}

	static Entity* load_prefab(World* w, pugi::xml_node src)
	{
		auto e = Entity::create();
		e->set_name(src.attribute("name").value());
		e->set_visibility(src.attribute("visibility").as_bool());

		for (auto n_c : src.child("components"))
		{
			auto udt = find_udt(FLAME_HASH((std::string("D#Serializer_") + n_c.name()).c_str()));
			assert(udt);
			auto object = malloc(udt->size());
			auto module = load_module(udt->db()->module_name());
			{
				auto f = udt->find_function("ctor");
				if (f && f->parameter_count() == 0)
					cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), object);
			}
			for (auto n_v : n_c)
			{
				auto v = udt->find_variable(n_v.name());
				v->type()->unserialize(n_v.attribute("v").value(), (char*)object + v->offset());
			}
			void* component;
			{
				auto f = udt->find_function("create");
				assert(f && check_function(f, "P#Component", { "P#World" }));
				component = cmf(p2f<MF_vp_vp>((char*)module + (uint)f->rva()), object, w);
			}
			e->add_component((Component*)component);
			{
				auto f = udt->find_function("dtor");
				if (f)
					cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), object);
			}
			free_module(module);
			free(object);
		}

		for (auto n_e : src.child("children"))
			e->add_child(load_prefab(w, n_e));

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
		n.append_attribute("name").set_value(src->name.empty() ? "unnamed" : src->name.c_str());
		n.append_attribute("visibility").set_value(src->visibility_ );

		if (!src->components.empty())
		{
			auto n_cs = n.append_child("components");
			for (auto& _c : src->components)
			{
				auto c = _c.second.get();

				auto n_c = n_cs.append_child(c->name);

				auto udt = find_udt(FLAME_HASH((std::string("D#Serializer_") + c->name).c_str()));
				assert(udt);
				auto object = malloc(udt->size());
				auto module = load_module(L"flame_universe.dll");
				{
					auto f = udt->find_function("ctor");
					if (f && f->parameter_count() == 0)
						cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), object);
				}
				{
					auto f = udt->find_function("serialize");
					assert(f && check_function(f, "D#void", { "P#Component", "D#int" }));
					cmf(p2f<MF_v_vp_u>((char*)module + (uint)f->rva()), object, c, -1);
				}
				for (auto i = 0; i < udt->variable_count(); i++)
				{
					auto v = udt->variable(i);
					auto type = v->type();
					auto p = (char*)object + v->offset();
					if (type->tag() == TypeData)
					{
						auto str = type->serialize(p, 2);
						if (str != v->default_value())
							n_c.append_child(v->name()).append_attribute("v").set_value(str.c_str());
					}
				}
				{
					auto f = udt->find_function("dtor");
					if (f)
						cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), object);
				}
				free_module(module);
				free(object);
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

		save_prefab(file, (EntityPrivate*)e);

		file.save_file(filename);
	}

	void Entity::destroy(Entity* _e)
	{
		auto e = (EntityPrivate*)_e;
		e->mark_dying();
		delete e;
	}
}
