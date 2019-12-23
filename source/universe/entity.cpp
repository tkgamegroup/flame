#include <flame/foundation/serialize.h>
#include "entity_private.h"

namespace flame
{
	EntityPrivate::EntityPrivate()
	{
		order_ = 0;
		created_frame_ = looper().frame;
		dying_ = false;

		visibility_ = true;
		global_visibility_ = true;

		name_hash = 0;

		world_ = nullptr;
		parent = nullptr;
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

	Mail<std::vector<Component*>> EntityPrivate::get_components()
	{
		auto ret = new_mail<std::vector<Component*>>();
		for (auto& c : components)
			ret.p->push_back(c.second.get());
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
		if (e == FLAME_INVALID_POINTER)
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
		ret->set_name(name);
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

	const std::string& Entity::name() const
	{
		return ((EntityPrivate*)this)->name;
	}

	uint Entity::name_hash() const
	{
		return ((EntityPrivate*)this)->name_hash;
	}

	void Entity::set_name(const std::string& name) const
	{
		auto thiz = ((EntityPrivate*)this);
		thiz->name = name;
		thiz->name_hash = H(name.c_str());
	}

	void Entity::set_visibility(bool v)
	{
		((EntityPrivate*)this)->set_visibility(v);
	}

	Component* Entity::get_component_plain(uint name_hash) const
	{
		return ((EntityPrivate*)this)->get_component_plain(name_hash);
	}

	Mail<std::vector<Component*>> Entity::get_components() const
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

	Entity* Entity::find_child(const std::string& name) const
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

	Entity* Entity::create()
	{
		return new EntityPrivate;
	}

	static Entity* load_prefab(World* w, const std::vector<TypeinfoDatabase*>& dbs, SerializableNode* src)
	{
		auto e = Entity::create();
		e->set_name(src->find_attr("name")->value());
		e->set_visibility(src->find_attr("visibility")->value() == "1");

		auto this_module = load_module(L"flame_universe.dll");
		TypeinfoDatabase* this_db = nullptr;
		for (auto db : dbs)
		{
			if (std::filesystem::path(db->module_name()).filename() == L"flame_universe.dll")
			{
				this_db = db;
				break;
			}
		}
		assert(this_module && this_db);

		auto n_cs = src->find_node("components");
		if (n_cs)
		{
			for (auto i_c = 0; i_c < n_cs->node_count(); i_c++)
			{
				auto n_c = n_cs->node(i_c);

				auto udt = find_udt(dbs, H(("D#Serializer_" + n_c->name()).c_str()));
				assert(udt);
				auto object = malloc(udt->size());
				auto module = load_module(udt->db()->module_name());
				{
					auto f = udt->find_function("ctor");
					if (f && f->parameter_count() == 0)
						cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), object);
				}
				for (auto i = 0; i < n_c->node_count(); i++)
				{
					auto n_v = n_c->node(i);

					auto v = udt->find_variable(n_v->name());
					if (v->type().is_vector)
					{
						auto size = std::stoi(n_v->find_attr("size")->value());

						for (auto j = 0; j < n_v->node_count(); j++)
						{
							auto n_i = n_v->node(j);

						}
					}
					else
						v->type().unserialize(dbs, n_v->find_attr("v")->value(), (char*)object + v->offset(), this_module, this_db);
				}
				void* component;
				{
					auto f = udt->find_function("create");
					assert(f && f->return_type().hash == TypeInfo(TypePointer, "Component").hash && f->parameter_count() == 1 && f->parameter_type(0).hash == TypeInfo(TypePointer, "World").hash);
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
		}

		auto n_es = src->find_node("children");
		if (n_es)
		{
			for (auto i_e = 0; i_e < n_es->node_count(); i_e++)
				e->add_child(load_prefab(w, dbs, n_es->node(i_e)));
		}

		free_module(this_module);

		return e;
	}

	Entity* Entity::create_from_file(World* w, const std::vector<TypeinfoDatabase*>& dbs, const std::wstring& filename)
	{
		auto file = SerializableNode::create_from_xml_file(filename);
		if (!file || file->name() != "prefab")
			return nullptr;

		return load_prefab(w, dbs, file->node(0));
	}

	static void save_prefab(const std::vector<TypeinfoDatabase*>& dbs, SerializableNode* dst, EntityPrivate* src)
	{
		auto n = dst->new_node("entity");
		n->new_attr("name", src->name.empty() ? "unnamed" : src->name);
		n->new_attr("visibility", src->visibility_ ? "1" : "0");

		if (!src->components.empty())
		{
			auto n_cs = n->new_node("components");
			for (auto& _c : src->components)
			{
				auto c = _c.second.get();

				auto n_c = n_cs->new_node(c->name);

				auto udt = find_udt(dbs, H((std::string("Serializer_") + c->name).c_str()));
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
					assert(f && f->return_type().hash == TypeInfo(TypeData, "void").hash && f->parameter_count() == 2 && f->parameter_type(0).hash == TypeInfo(TypeData, "Component").hash && f->parameter_type(1).hash == TypeInfo(TypeData, "int").hash);
					cmf(p2f<MF_v_vp_u>((char*)module + (uint)f->rva()), object, c, -1);
				}
				for (auto i = 0; i < udt->variable_count(); i++)
				{
					auto v = udt->variable(i);
					auto p = (char*)object + v->offset();
					if ((v->type().tag == TypeData && (v->type().hash == cH("std::string") || v->type().hash == cH("std::wstring"))) || memcmp(p, v->default_value(), v->size()) != 0)
					{
						auto n = n_c->new_node(v->name());
						n->new_attr("v", v->type().serialize(dbs, p, 2));
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
			auto n_es = n->new_node("children");
			for (auto& e : src->children)
				save_prefab(dbs, n_es, e.get());
		}
	}

	void Entity::save_to_file(const std::vector<TypeinfoDatabase*>& dbs, Entity* e, const std::wstring& filename)
	{
		auto file = SerializableNode::create("prefab");

		save_prefab(dbs, file, (EntityPrivate*)e);

		SerializableNode::save_to_xml_file(file, filename);
	}

	void Entity::destroy(Entity* w)
	{
		delete (EntityPrivate*)w;
	}
}
