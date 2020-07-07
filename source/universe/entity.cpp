#include <flame/foundation/typeinfo.h>
#include <flame/universe/component.h>
#include "entity_private.h"
#include "world_private.h"

namespace flame
{
	EntityPrivate::EntityPrivate()
	{
		_created_frame = get_looper()->get_frame();

#ifdef _DEBUG
		//_created_stack_ = get_stack_frames(); TODO
#endif
	}

	EntityPrivate::~EntityPrivate()
	{
		for (auto& e : _children)
		{
			for (auto c : e->_local_event_dispatch_list)
				c->on_entity_removed();
		}
		for (auto c : _local_event_dispatch_list)
			c->on_entity_destroyed();
	}

	void EntityPrivate::_release()
	{
		if (_parent)
			_parent->_remove_child(this);
		else
		{
			this->~EntityPrivate();
			_deallocate(this);
		}
	}

	void EntityPrivate::_set_name(const std::string& name)
	{
		_name = name;
		_name_hash = FLAME_HASH(name.c_str());
	}

	void EntityPrivate::_update_visibility()
	{
		auto prev_visibility = _global_visibility;
		if (_parent)
			_global_visibility = _visible && _parent->_global_visibility;
		else
		{
			if (_world->_root.get() == this)
				_global_visibility = true;
			else
				_global_visibility = false;
		}
		if (_global_visibility != prev_visibility)
		{
			for (auto c : _local_event_dispatch_list)
				c->on_entity_visibility_changed();
			if (_parent)
			{
				for (auto c : _parent->_child_event_dispatch_list)
					c->on_entity_child_visibility_changed(this);
			}
		}

		for (auto& e : _children)
			e->_update_visibility();
	}

	void EntityPrivate::_set_visible(bool v)
	{
		if (_visible == v)
			return;
		_visible = v;
		_update_visibility();
	}

	Component* EntityPrivate::_get_component(uint hash) const
	{
		auto it = _components.find(hash);
		if (it != _components.end())
			return it->second.get();
		return nullptr;
	}

	void EntityPrivate::_add_component(Component* c)
	{
		assert(!c->entity);
		assert(_components.find(c->name_hash) == _components.end());

		c->entity = this;

		c->on_added();

		for (auto cc : _local_event_dispatch_list)
			cc->on_entity_component_added(c);
		if (_parent)
		{
			for (auto cc : _parent->_child_event_dispatch_list)
				cc->on_entity_child_component_added(c);
		}

		_components.emplace(c->name_hash, c);

		if (c->_want_local_event)
		{
			_local_event_dispatch_list.push_back(c);
			if (_world)
				c->on_entered_world();
		}
		if (c->_want_child_event)
			_child_event_dispatch_list.push_back(c);
		if (c->_want_local_data_changed)
			_local_data_changed_dispatch_list.push_back(c);
		if (c->_want_child_data_changed)
			_child_data_changed_dispatch_list.push_back(c);
	}

	void EntityPrivate::_remove_component(Component* c, bool destroy)
	{
		auto it = _components.find(c->name_hash);
		if (it == _components.end())
		{
			assert(0);
			return;
		}

		for (auto cc : _local_event_dispatch_list)
			cc->on_entity_component_removed(c);
		if (_parent)
		{
			for (auto cc : _parent->_child_event_dispatch_list)
				cc->on_entity_child_component_removed(c);
		}

		if (c->_want_local_event)
		{
			erase_if(_local_event_dispatch_list, c);
			if (_world)
				c->on_left_world();
		}
		if (c->_want_child_event)
			erase_if(_child_event_dispatch_list, c);
		if (c->_want_local_data_changed)
			erase_if(_local_data_changed_dispatch_list, c);
		if (c->_want_child_data_changed)
			erase_if(_child_data_changed_dispatch_list, c);

		if (!destroy)
			it->second.release();
		_components.erase(it);
	}

	void EntityPrivate::_data_changed(Component* c, uint hash)
	{
		assert(c->entity == this);
		for (auto cc : _local_data_changed_dispatch_list)
		{
			if (cc != c)
				cc->on_entity_component_data_changed(c, hash);
		}
		if (_parent)
		{
			for (auto cc : _parent->_child_data_changed_dispatch_list)
				cc->on_entity_child_component_data_changed(c, hash);
		}
	}

	void EntityPrivate::_enter_world()
	{
		for (auto c : _local_event_dispatch_list)
			c->on_entered_world();
		for (auto& e : _children)
		{
			e->_world = _world;
			e->_enter_world();
		}
	}

	void EntityPrivate::_leave_world()
	{
		for (auto it = _children.rbegin(); it != _children.rend(); it++)
			(*it)->_leave_world();
		_world = nullptr;
		for (auto c : _local_event_dispatch_list)
			c->on_left_world();
	}

	void EntityPrivate::_inherit()
	{
		for (auto& e : _children)
		{
			e->_gene = _gene;
			e->_inherit();
		}
	}

	void EntityPrivate::_add_child(EntityPrivate* e, int position)
	{
		if (position == -1)
			position = _children.size();

		for (auto i = position; i < _children.size(); i++)
		{
			auto e = _children[i].get();
			e->_index += 1;
			for (auto c : e->_local_event_dispatch_list)
				c->on_entity_position_changed();
			for (auto c : _child_event_dispatch_list)
				c->on_entity_child_position_changed(e);
		}
		_children.emplace(_children.begin() + position, e);

		if (_gene)
		{
			e->_gene = _gene;
			e->_inherit();
		}
		e->_parent = this;
		e->_depth = _depth + 1;
		e->_index = position;
		e->_update_visibility();

		if (e->_world != _world)
		{
			e->_world = _world;
			if (_world)
				e->_enter_world();
		}

		for (auto c : e->_local_event_dispatch_list)
			c->on_entity_added();
		for (auto c : _local_event_dispatch_list)
			c->on_entity_added_child(e);
	}

	void EntityPrivate::_reposition_child(uint pos1, uint pos2)
	{
		if (pos1 == pos2)
			return;
		assert(pos1 < _children.size() && pos2 < _children.size());

		auto a = _children[pos1].get();
		auto b = _children[pos2].get();
		a->_index = pos2;
		b->_index = pos1;
		std::swap(_children[pos1], _children[pos2]);

		for (auto c : a->_local_event_dispatch_list)
			c->on_entity_position_changed();
		for (auto c : b->_local_event_dispatch_list)
			c->on_entity_position_changed();

		for (auto c : _child_event_dispatch_list)
			c->on_entity_child_position_changed(a);
		for (auto c : _child_event_dispatch_list)
			c->on_entity_child_position_changed(b);
	}

	void EntityPrivate::_remove_child(EntityPrivate* e, bool destroy)
	{
		auto it = std::find_if(_children.begin(), _children.end(), [&](const auto& t) {
			return t.get() == e;
		});
		if (it == _children.end())
		{
			assert(0); // not found!
			return;
		}

		for (auto i = 0; i < e->_index; i++)
		{
			auto ee = _children[i].get();
			ee->_index -= 1;
			for (auto c : ee->_local_event_dispatch_list)
				c->on_entity_position_changed();
			for (auto c : _child_event_dispatch_list)
				c->on_entity_child_position_changed(ee);
		}

		e->_parent = nullptr;
		if (e->_world)
			e->_leave_world();

		for (auto c : e->_local_event_dispatch_list)
			c->on_entity_removed();
		for (auto c : _local_event_dispatch_list)
			c->on_entity_removed_child(e);

		if (!destroy)
			it->release();
		_children.erase(it);
	}

	static void load_prefab(EntityPrivate* dst, pugi::xml_node src)
	{
		dst->_set_name(src.attribute("name").value());
		dst->_visible = src.attribute("visible").as_bool();

		for (auto n_c : src.children())
		{
			auto udt = find_udt(("flame::" + std::string(n_c.name())).c_str());
			auto fc = udt->find_function("create");
			if (fc->get_type()->get_tag() == TypePointer && fc->get_parameters_count() == 0)
			{
				Component* c = nullptr;
				fc->call(nullptr, &c);
				for (auto a : n_c.attributes())
				{
					auto fs = udt->find_function(("set_" + std::string(a.name())).c_str());
					if (fs->get_type() == TypeInfo::get(TypeData, "void") && fs->get_parameters_count() == 1)
					{
						auto type = fs->get_parameter(0);
						void* d = new char[type->get_size()];
						type->unserialize(a.value(), d);
						fs->call(c, nullptr, d);
					}
				}
				dst->add_component((Component*)c);
			}
		}

		//for (auto n_c : src.child("components"))
		//{
		//	auto udt = find_udt((std::string("flame::") + n_c.name()).c_str());
		//	assert(udt && udt->get_base_name() == std::string("Component"));
		//	auto library_address = udt->get_library()->get_address();
		//	auto f = udt->find_function("create");
		//	assert(f);
		//	auto component = cf(p2f<F_vp_v>((char*)library_address + (uint)f->get_rva()));
		//	for (auto n_v : n_c)
		//	{
		//		auto v = udt->find_variable(n_v.name());
		//		auto type = v->get_type();
		//		auto p = (char*)component + v->get_offset();
		//		//if (type->tag == TypePointer) // TODO
		//		//{
		//		//	auto& s = type->base_name;
		//		//	auto name = s.v;
		//		//	auto len = s.s;
		//		//	for (auto i = len - 1; i >= 0; i--)
		//		//	{
		//		//		if (name[i] == ':')
		//		//		{
		//		//			name = name + i + 1;
		//		//			break;
		//		//		}
		//		//	}
		//		//	*(Object**)p = w->find_object(FLAME_HASH(name), n_v.attribute("v").as_uint());
		//		//}
		//		//else
		//		type->unserialize(n_v.attribute("v").value(), p);
		//	}
		//	dst->_add_component((Component*)component);
		//}

		//for (auto n_e : src.child("children"))
		//{
		//	auto e = EntityPrivate::_create();
		//	dst->_add_child(e);
		//	load_prefab(e, n_e);
		//}
	}

	void EntityPrivate::_load(const std::filesystem::path& filename)
	{
		pugi::xml_document file;
		pugi::xml_node file_root;
		if (!file.load_file(filename.c_str()) || (file_root = file.first_child()).name() != std::string("prefab"))
			return;

		load_prefab(this, file_root.first_child());
	}

	static void save_prefab(pugi::xml_node dst, EntityPrivate* src)
	{
		auto n = dst.append_child("entity");
		n.append_attribute("name").set_value(src->_name.empty() ? "unnamed" : src->_name.c_str());
		n.append_attribute("visible").set_value(src->_visible);

		//if (!src->_components.empty())
		//{
		//	auto n_cs = n.append_child("components");
		//	for (auto& c : src->_components)
		//	{
		//		auto component = c.second.get();

		//		auto n_c = n_cs.append_child(component->name);

		//		auto udt = find_udt((std::string("flame::") + component->name).c_str());
		//		assert(udt && udt->get_base_name() == std::string("Component"));
		//		auto variables_count = udt->get_variables_count();
		//		for (auto i = 0; i < variables_count; i++)
		//		{
		//			auto v = udt->get_variable(i);
		//			auto type = v->get_type();
		//			auto p = (char*)component + v->get_offset();
		//			if (type->get_tag() == TypePointer)
		//				//n_c.append_child(v->name.v).append_attribute("v").set_value((*(Object**)p)->id); TODO
		//				;
		//			else
		//			{
		//				auto dv = v->get_default_value();
		//				if (!dv || memcmp(dv, p, type->get_size()) != 0)
		//					n_c.append_child(v->get_name()).append_attribute("v").set_value(type->serialize_s(p).c_str());
		//			}
		//		}
		//	}
		//}

		//if (!src->_children.empty())
		//{
		//	auto n_es = n.append_child("children");
		//	for (auto& e : src->_children)
		//		save_prefab(n_es, e.get());
		//}
	}

	void EntityPrivate::_save(const std::filesystem::path& filename)
	{
		pugi::xml_document file;
		auto file_root = file.append_child("prefab");

		save_prefab(file_root, this);

		file.save_file(filename.c_str());
	}

	EntityPrivate* EntityPrivate::_create()
	{
		auto ret = _allocate(sizeof(EntityPrivate));
		new (ret) EntityPrivate;
		return (EntityPrivate*)ret;
	}

	Entity* Entity::create() { return EntityPrivate::_create(); }
}
