#include <flame/foundation/typeinfo.h>
#include "private.h"

namespace flame
{
	Entity::Entity()
	{
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
		created_stack_ = get_stack_frames();
#endif
	}

	Entity::~Entity()
	{
		mark_dying(this);

		for (auto c : children)
		{
			for (auto cc : c->components.get_all())
				cc->on_event(EntityRemoved, nullptr);
			c->event_listeners.call(EntityRemoved, nullptr);
		}
		for (auto c : components.get_all())
			c->on_event(EntityDestroyed, nullptr);
		event_listeners.call(EntityDestroyed, nullptr);

		event_listeners.impl->release();

		for (auto c : components.get_all())
			f_delete(c);
		for (auto c : children)
			f_delete(c);
	}

	static void update_visibility(Entity* e)
	{
		auto prev_visibility = e->global_visibility;
		e->global_visibility = e->parent ? e->visible_ && e->parent->global_visibility : false;
		if (e->global_visibility != prev_visibility)
		{
			for (auto c : e->components.get_all())
				c->on_event(EntityVisibilityChanged, nullptr);
			e->event_listeners.call(EntityVisibilityChanged, nullptr);
			auto p = e->parent;
			if (p)
			{
				for (auto c : p->components.get_all())
					c->on_event(EntityChildVisibilityChanged, nullptr);
				p->event_listeners.call(EntityChildVisibilityChanged, e);
			}
		}

		for (auto c : e->children)
			update_visibility(c);
	}

	void Entity::set_visible(bool v)
	{
		if (visible_ == v)
			return;
		visible_ = v;
		update_visibility(this);
	}

	void Entity::add_component(Component* c)
	{
		auto hash = c->name_hash;
		if (c->id)
			hash = hash_update(hash, c->id);
		assert(!components.find(hash));

		c->entity = this;
		components.add(hash, c);
		if (world)
			c->on_event(EntityEnteredWorld, nullptr);
		for (auto cc : components.get_all())
			cc->on_event(EntityComponentAdded, c);
		event_listeners.call(EntityComponentAdded, c);
		if (parent)
		{
			for (auto cc : parent->components.get_all())
				cc->on_event(EntityChildComponentAdded, c);
			parent->event_listeners.call(EntityChildComponentAdded, c);
		}
	}

	void Entity::remove_component(Component* c)
	{
		auto hash = c->name_hash;
		if (c->id)
			hash = hash_update(hash, c->id);
		if (components.remove(hash))
		{
			for (auto cc : components.get_all())
				cc->on_event(EntityComponentRemoved, c);
			event_listeners.call(EntityComponentRemoved, c);
			if (parent)
			{
				for (auto cc : parent->components.get_all())
					cc->on_event(EntityChildComponentRemoved, c);
				parent->event_listeners.call(EntityChildComponentRemoved, c);
			}
			f_delete(c);
		}
		else
			assert(0);
	}

	static void enter_world(World* w, Entity* e)
	{
		e->world = w;
		for (auto c : e->components.get_all())
			c->on_event(EntityEnteredWorld, nullptr);
		e->event_listeners.call(EntityEnteredWorld, nullptr);
		for (auto c : e->children)
			enter_world(w, c);
	}

	static void leave_world(Entity* e)
	{
		for (auto i = (int)e->children.s - 1; i >= 0; i--)
			leave_world(e->children[i]);
		e->world = nullptr;
		for (auto c : e->components.get_all())
			c->on_event(EntityLeftWorld, nullptr);
		e->event_listeners.call(EntityLeftWorld, nullptr);
	}

	static void inherit(Entity* e, void* gene)
	{
		if (!gene)
			return;
		e->gene = gene;
		for (auto c : e->children)
			inherit(c, gene);
	}

	void Entity::add_child(Entity* e, int position)
	{
		if (position == -1)
			position = children.s;
		for (auto i = position; i < children.s; i++)
			children[i]->index_ += 1;
		children.insert(position, e);
		inherit(e, gene);
		e->depth_ = depth_ + 1;
		e->index_ = position;
		e->parent = this;
		update_visibility(e);
		if (!e->world && world)
			enter_world(world, e);
		for (auto c : e->components.get_all())
			c->on_event(EntityAdded, nullptr);
		e->event_listeners.call(EntityAdded, nullptr);
		for (auto c : components.get_all())
			c->on_event(EntityChildAdded, e);
		event_listeners.call(EntityChildAdded, e);
	}

	void Entity::reposition_child(Entity* e, int position)
	{
		if (position == -1)
			position = children.s - 1;
		assert(position < children.s);
		auto old_position = e->index_;
		if (old_position == position)
			return;
		auto dst = children[position];
		std::swap(children[old_position], children[position]);
		dst->index_ = old_position;
		e->index_ = position;
		for (auto c : e->components.get_all())
			c->on_event(EntityPositionChanged, nullptr);
		e->event_listeners.call(EntityPositionChanged, nullptr);
		for (auto c : dst->components.get_all())
			c->on_event(EntityPositionChanged, nullptr);
		dst->event_listeners.call(EntityPositionChanged, nullptr);
		for (auto c : components.get_all())
		{
			c->on_event(EntityChildPositionChanged, e);
			c->on_event(EntityChildPositionChanged, dst);
		}
		event_listeners.call(EntityChildPositionChanged, e);
		event_listeners.call(EntityChildPositionChanged, dst);
	}

	void Entity::remove_child(Entity* e, bool destroy)
	{
		for (auto i = 0; i < children.s; i++)
		{
			auto ee = children[i];
			if (ee == e)
			{
				remove_children(i, i, destroy);
				return;
			}
		}
	}

	void Entity::remove_children(int from, int to, bool destroy)
	{
		if (to == -1)
			to = children.s - 1;
		std::vector<Entity*> es;
		for (auto i = from; i <= to; i++)
			es.push_back(children[i]);
		for (auto i = to + 1; i < children.s; i++)
			children[i]->index_ -= 1;
		children.remove(from, es.size());
		for (auto e : es)
		{
			if (e->world)
				leave_world(e);
			for (auto c : e->components.get_all())
				c->on_event(EntityRemoved, nullptr);
			e->event_listeners.call(EntityRemoved, nullptr);
			for (auto c : components.get_all())
				c->on_event(EntityChildRemoved, e);
			event_listeners.call(EntityChildRemoved, e);
			if (destroy)
			{
				mark_dying(e);
				f_delete(e);
			}
			else
				e->parent = nullptr;
		}
	}

	static Entity* load_prefab(World* w, pugi::xml_node src)
	{
		auto e = new Entity;
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

	static void save_prefab(pugi::xml_node dst, Entity* src)
	{
		auto n = dst.append_child("entity");
		n.append_attribute("name").set_value(src->name.s ? "unnamed" : src->name.v);
		n.append_attribute("visible").set_value(src->visible_ );

		auto components = src->components.get_all();
		if (!components.empty())
		{
			auto n_cs = n.append_child("components");
			for (auto component : components)
			{
				auto n_c = n_cs.append_child(component->name);

				auto udt = find_udt(FLAME_HASH((std::string("flame::") + component->name).c_str()));
				assert(udt && udt->base_name.str() == "Component");
				for (auto v : udt->variables)
				{
					auto type = v->type;
					auto p = (char*)component + v->offset;
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

		if (src->children.s > 0)
		{
			auto n_es = n.append_child("children");
			for (auto e : src->children)
				save_prefab(n_es, e);
		}
	}

	void Entity::save_to_file(Entity* e, const wchar_t* filename)
	{
		pugi::xml_document file;
		auto file_root = file.append_child("prefab");

		save_prefab(file_root, (Entity*)e);

		file.save_file(filename);
	}
}
