#include <flame/foundation/serialize.h>
#include <flame/universe/entity.h>
#include <flame/universe/component.h>

namespace flame
{
	struct EntityPrivate : Entity
	{
		std::string name;
		uint name_hash;
		std::vector<std::unique_ptr<Component>> components;
		EntityPrivate* parent;
		std::vector<std::unique_ptr<EntityPrivate>> children;

		EntityPrivate() :
			parent(nullptr)
		{
			created_frame = looper().frame;

			visible = true;
			global_visible = false;

			first_update = true;

			name_hash = 0;
		}

		Component* find_component(uint type_hash)
		{
			for (auto& c : components)
			{
				if (c->type_hash == type_hash)
					return c.get();
			}
			return nullptr;
		}

		Mail<std::vector<Component*>> find_components(uint type_hash)
		{
			auto ret = new_mail<std::vector<Component*>>();
			for (auto& c : components)
			{
				if (c->type_hash == type_hash)
					ret.p->push_back(c.get());
			}
			return ret;
		}

		void add_component(Component* c)
		{
			c->entity = this;
			components.emplace_back(c);
		}

		EntityPrivate* find_child(const std::string& name) const
		{
			for (auto& e : children)
			{
				if (e->name == name)
					return e.get();
			}
			return nullptr;
		}

		void add_child(EntityPrivate* e, int position)
		{
			if (position == -1)
				position = children.size();
			children.insert(children.begin() + position, std::unique_ptr<EntityPrivate>(e));
			e->parent = this;
			for (auto& c : e->components)
				c->on_entity_added_to_parent();
		}

		void reposition_child(EntityPrivate* e, int position)
		{
			if (position == -1)
				position = children.size() - 1;
			assert(position < children.size());
			if (children[position].get() == e)
				return;
			for (auto& _e : children)
			{
				if (_e.get() == e)
				{
					std::swap(_e, children[position]);
					break;
				}
			}
		}

		void remove_child(EntityPrivate* e)
		{
			for (auto it = children.begin(); it != children.end(); it++)
			{
				if (it->get() == e)
				{
					children.erase(it);
					return;
				}
			}
		}

		void take_child(EntityPrivate* e)
		{
			for (auto it = children.begin(); it != children.end(); it++)
			{
				if (it->get() == e)
				{
					e->parent = nullptr;
					it->release();
					children.erase(it);
					return;
				}
			}
		}

		void remove_all_children()
		{
			children.clear();
		}

		void take_all_children()
		{
			for (auto& e : children)
			{
				e->parent = nullptr;
				e.release();
			}
			children.clear();
		}

		EntityPrivate* copy()
		{
			auto ret = new EntityPrivate;

			ret->visible = visible;
			ret->set_name(name);
			for (auto& c : components)
			{
				auto copy = c->copy();
				ret->add_component(copy);
			}
			for (auto& e : children)
			{
				auto copy = e->copy();
				ret->add_child(copy, -1);
			}

			return ret;
		}

		void traverse_forward(void (*callback)(void* c, Entity* n), const Mail<>& capture)
		{
			callback(capture.p, this);
			for (auto& c : children)
			{
				if (c->global_visible)
					c->traverse_forward(callback, capture);
			}
		}

		void traverse_backward(void (*callback)(void* c, Entity* n), const Mail<>& capture)
		{
			for (auto it = children.rbegin(); it != children.rend(); it++)
			{
				auto c = it->get();
				if (c->global_visible)
					c->traverse_backward(callback, capture);
			}
			callback(capture.p, this);
		}

		void update()
		{
			if (!parent)
				global_visible = visible;
			else
				global_visible = visible && parent->global_visible;
			if (!global_visible)
				return;
			if (first_update)
			{
				first_update = false;
				for (auto& c : components)
					c->start();
			}
			for (auto& c : components)
				c->update();
			for (auto& e : children)
				e->update();
		}
	};

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

	uint Entity::component_count() const
	{
		return ((EntityPrivate*)this)->components.size();
	}

	Component* Entity::component(uint index) const
	{
		return ((EntityPrivate*)this)->components[index].get();
	}

	Component* Entity::find_component(uint type_hash) const
	{
		return ((EntityPrivate*)this)->find_component(type_hash);
	}

	Mail<std::vector<Component*>> Entity::find_components(uint type_hash) const
	{
		return ((EntityPrivate*)this)->find_components(type_hash);
	}

	void Entity::add_component(Component* c)
	{
		((EntityPrivate*)this)->add_component(c);
	}

	Entity* Entity::parent() const
	{
		return ((EntityPrivate*)this)->parent;
	}

	uint Entity::child_count() const
	{
		return ((EntityPrivate*)this)->children.size();
	}

	int Entity::child_position(Entity* e) const
	{
		auto& children = ((EntityPrivate*)this)->children;
		for (auto i = 0; i < children.size(); i++)
		{
			if (children[i].get() == e)
				return i;
		}
		return -1;
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

	void Entity::remove_child(Entity* e)
	{
		((EntityPrivate*)this)->remove_child((EntityPrivate*)e);
	}

	void Entity::take_child(Entity* e)
	{
		((EntityPrivate*)this)->take_child((EntityPrivate*)e);
	}

	void Entity::remove_all_children()
	{
		((EntityPrivate*)this)->remove_all_children();
	}

	void Entity::take_all_children()
	{
		((EntityPrivate*)this)->take_all_children();
	}

	Entity* Entity::copy()
	{
		return ((EntityPrivate*)this)->copy();
	}

	void Entity::traverse_forward(void (*callback)(void* c, Entity* n), const Mail<>& capture)
	{
		((EntityPrivate*)this)->traverse_forward(callback, capture);
		delete_mail(capture);
	}

	void Entity::traverse_backward(void (*callback)(void* c, Entity* n), const Mail<>& capture)
	{
		((EntityPrivate*)this)->traverse_backward(callback, capture);
		delete_mail(capture);
	}

	void Entity::update()
	{
		((EntityPrivate*)this)->update();
	}

	Entity* Entity::create()
	{
		return new EntityPrivate;
	}

	static Entity* load_prefab(const std::vector<TypeinfoDatabase*>& dbs, SerializableNode* src)
	{
		auto e = Entity::create();
		e->set_name(src->find_attr("name")->value());

		auto n_cs = src->find_node("components");
		if (n_cs)
		{
			for (auto i_c = 0; i_c < n_cs->node_count(); i_c++)
			{
				auto n_c = n_cs->node(i_c);

				auto udt = find_udt(dbs, H(("c" + n_c->name()).c_str()));
				assert(udt);
				auto dummy = malloc(udt->size());
				auto module = load_module(L"flame_universe.dll");
				{
					auto f = udt->find_function("ctor");
					if (f && f->parameter_count() == 0)
						cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), dummy);
				}
				for (auto i = 0; i < n_c->node_count(); i++)
				{
					auto n = n_c->node(i);

					auto v = udt->find_variable(n->name());
					auto type = v->type();
					unserialize_value(dbs, type->tag(), type->hash(), n->find_attr("v")->value(), (char*)dummy + v->offset());
				}
				void* c;
				{
					auto f = udt->find_function("create");
					assert(f && f->return_type()->equal(TypeTagPointer, cH("Component")) && f->parameter_count() == 0);
					c = cmf(p2f<MF_vp_v>((char*)module + (uint)f->rva()), dummy);
				}
				e->add_component((Component*)c);
				{
					auto f = udt->find_function("dtor");
					if (f)
						cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), dummy);
				}
				free_module(module);
				free(dummy);
			}
		}

		auto n_es = src->find_node("children");
		if (n_es)
		{
			for (auto i_e = 0; i_e < n_es->node_count(); i_e++)
				e->add_child(load_prefab(dbs, n_es->node(i_e)));
		}

		return e;
	}

	Entity* Entity::create_from_file(const std::vector<TypeinfoDatabase*>& dbs, const std::wstring& filename)
	{
		auto file = SerializableNode::create_from_xml_file(filename);
		if (!file || file->name() != "prefab")
			return nullptr;

		return load_prefab(dbs, file->node(0));
	}

	static void save_prefab(const std::vector<TypeinfoDatabase*>& dbs, SerializableNode* dst, EntityPrivate* src)
	{
		auto n = dst->new_node("entity");
		n->new_attr("name", src->name.empty() ? "unnamed" : src->name);
		n->new_attr("visible", src->visible ? "1" : "0");

		if (!src->components.empty())
		{
			auto n_cs = n->new_node("components");
			for (auto& c : src->components)
			{
				auto n_c = n_cs->new_node(c->type_name);

				auto udt = find_udt(dbs, H((std::string("c") + c->type_name).c_str()));
				assert(udt);
				auto dummy = malloc(udt->size());
				auto module = load_module(L"flame_universe.dll");
				{
					auto f = udt->find_function("ctor");
					if (f && f->parameter_count() == 0)
						cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), dummy);
				}
				{
					auto f = udt->find_function("save");
					assert(f && f->return_type()->equal(TypeTagVariable, cH("void")) && f->parameter_count() == 1 && f->parameter_type(0)->equal(TypeTagPointer, cH("Component")));
					cmf(p2f<MF_v_vp>((char*)module + (uint)f->rva()), dummy, c.get());
				}
				for (auto i = 0; i < udt->variable_count(); i++)
				{
					auto v = udt->variable(i);
					auto type = v->type();
					auto tag = type->tag();
					auto hash = type->hash();
					auto p = (char*)dummy + v->offset();
					if ((tag == TypeTagVariable && (hash == cH("std::basic_string(char)") || hash == cH("std::basic_string(wchar_t)"))) || memcmp(p, v->default_value(), v->size()) != 0)
					{
						auto n = n_c->new_node(v->name());
						auto value = serialize_value(dbs, tag, hash, p, 2);
						n->new_attr("v", *value.p);
						delete_mail(value);
					}
				}
				{
					auto f = udt->find_function("dtor");
					if (f)
						cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), dummy);
				}
				free_module(module);
				free(dummy);
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

	void* component_alloc(uint size)
	{
		return malloc(size);
	}

	static std::map<std::string, void*> serialization_datas;

	void universe_serialization_initialize()
	{
		serialization_datas.clear();
	}

	void universe_serialization_set_data(const std::string& name, void* data)
	{
		serialization_datas[name] = data;
	}

	void* universe_serialization_get_data(const std::string& name)
	{
		return serialization_datas[name];
	}

	const std::string& universe_serialization_find_data(void* data)
	{
		for (auto it = serialization_datas.begin(); it != serialization_datas.end(); it++)
		{
			if (it->second == data)
				return it->first;
		}

		return "";
	}
}
