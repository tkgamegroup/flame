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
		for (auto& e : children)
		{
			for (auto c : e->local_event_dispatch_list)
				c->on_entity_removed();
		}
		for (auto c : local_event_dispatch_list)
			c->on_entity_destroyed();
	}

	void EntityPrivate::release()
	{
		if (parent)
			parent->remove_child(this);
		else
		{
			this->~EntityPrivate();
			_deallocate(this);
		}
	}

	void EntityPrivate::update_visibility()
	{
		auto prev_visibility = global_visibility;
		if (parent)
			global_visibility = visible && parent->global_visibility;
		else
		{
			if (world->root.get() == this)
				global_visibility = true;
			else
				global_visibility = false;
		}
		if (global_visibility != prev_visibility)
		{
			for (auto c : local_event_dispatch_list)
				c->on_entity_visibility_changed();
			if (parent)
			{
				for (auto c : parent->child_event_dispatch_list)
					c->on_entity_child_visibility_changed(this);
			}
		}

		for (auto& e : children)
			e->update_visibility();
	}

	void EntityPrivate::set_visible(bool v)
	{
		if (visible == v)
			return;
		visible = v;
		update_visibility();
	}

	void EntityPrivate::set_state(StateFlags s)
	{
		if (state == s)
			return;
		state = s;
		for (auto c : local_event_dispatch_list)
			c->on_entity_state_changed();
	}

	Component* EntityPrivate::get_component(uint64 hash) const
	{
		auto it = components.find(hash);
		if (it != components.end())
			return it->second.get();
		return nullptr;
	}

	void EntityPrivate::add_component(Component* c)
	{
		assert(!c->entity);
		assert(components.find(c->type_hash) == components.end());

		c->entity = this;

		c->on_added();

		for (auto cc : local_event_dispatch_list)
			cc->on_entity_component_added(c);
		if (parent)
		{
			for (auto cc : parent->child_event_dispatch_list)
				cc->on_entity_child_component_added(c);
		}

		components.emplace(c->type_hash, c);

		if (c->_want_local_event)
		{
			local_event_dispatch_list.push_back(c);
			if (world)
				c->on_entered_world();
		}
		if (c->_want_child_event)
			child_event_dispatch_list.push_back(c);
		if (c->_want_local_data_changed)
			local_data_changed_dispatch_list.push_back(c);
		if (c->_want_child_data_changed)
			child_data_changed_dispatch_list.push_back(c);
	}

	void EntityPrivate::info_component_removed(Component* c) const
	{
		for (auto cc : local_event_dispatch_list)
			cc->on_entity_component_removed(c);
		if (parent)
		{
			for (auto cc : parent->child_event_dispatch_list)
				cc->on_entity_child_component_removed(c);
		}

		if (c->_want_local_event)
		{
			if (world)
				c->on_left_world();
		}
	}

	void EntityPrivate::remove_component(Component* c, bool destroy)
	{
		auto it = components.find(c->type_hash);
		if (it == components.end())
		{
			assert(0);
			return;
		}

		info_component_removed(c);

		if (c->_want_local_event)
		{
			std::erase_if(local_event_dispatch_list, [&](const auto& i) {
				return i == c;
			});
		}
		if (c->_want_child_event)
		{
			std::erase_if(child_event_dispatch_list, [&](const auto& i) {
				return i == c;
			});
		}
		if (c->_want_local_data_changed)
		{
			std::erase_if(local_data_changed_dispatch_list, [&](const auto& i) {
				return i == c;
			});
		}
		if (c->_want_child_data_changed)
		{
			std::erase_if(child_data_changed_dispatch_list, [&](const auto& i) {
				return i == c;
			});
		}

		if (!destroy)
			it->second.release();
		components.erase(it);
	}

	void EntityPrivate::remove_all_components(bool destroy)
	{
		for (auto& c : components)
			info_component_removed(c.second.get());

		local_event_dispatch_list.clear();
		child_event_dispatch_list.clear();
		local_data_changed_dispatch_list.clear();
		child_data_changed_dispatch_list.clear();

		if (!destroy)
		{
			for (auto& c : components)
				c.second.release();
		}
		components.clear();
	}

	void EntityPrivate::data_changed(Component* c, uint hash)
	{
		assert(c->entity == this);
		for (auto cc : local_data_changed_dispatch_list)
		{
			if (cc != c)
				cc->on_entity_component_data_changed(c, hash);
		}
		if (parent)
		{
			for (auto cc : parent->child_data_changed_dispatch_list)
				cc->on_entity_child_component_data_changed(c, hash);
		}
	}

	void EntityPrivate::enter_world()
	{
		for (auto c : local_event_dispatch_list)
			c->on_entered_world();
		for (auto& e : children)
		{
			e->world = world;
			e->enter_world();
		}
	}

	void EntityPrivate::leave_world()
	{
		for (auto it = children.rbegin(); it != children.rend(); it++)
			(*it)->leave_world();
		world = nullptr;
		for (auto c : local_event_dispatch_list)
			c->on_left_world();
	}

	void EntityPrivate::inherit()
	{
		for (auto& e : children)
		{
			e->gene = gene;
			e->inherit();
		}
	}

	void EntityPrivate::add_child(EntityPrivate* e, int position)
	{
		if (position == -1)
			position = children.size();

		for (auto i = position; i < children.size(); i++)
		{
			auto e = children[i].get();
			e->index += 1;
			for (auto c : e->local_event_dispatch_list)
				c->on_entity_position_changed();
			for (auto c : child_event_dispatch_list)
				c->on_entity_child_position_changed(e);
		}
		children.emplace(children.begin() + position, e);

		if (gene)
		{
			e->gene = gene;
			e->inherit();
		}
		e->parent = this;
		e->depth = depth + 1;
		e->index = position;
		e->update_visibility();

		if (e->world != world)
		{
			e->world = world;
			if (world)
				e->enter_world();
		}

		for (auto c : e->local_event_dispatch_list)
			c->on_entity_added();
		for (auto c : local_event_dispatch_list)
			c->on_entity_added_child(e);
	}

	void EntityPrivate::reposition_child(uint pos1, uint pos2)
	{
		if (pos1 == pos2)
			return;
		assert(pos1 < children.size() && pos2 < children.size());

		auto a = children[pos1].get();
		auto b = children[pos2].get();
		a->index = pos2;
		b->index = pos1;
		std::swap(children[pos1], children[pos2]);

		for (auto c : a->local_event_dispatch_list)
			c->on_entity_position_changed();
		for (auto c : b->local_event_dispatch_list)
			c->on_entity_position_changed();

		for (auto c : child_event_dispatch_list)
			c->on_entity_child_position_changed(a);
		for (auto c : child_event_dispatch_list)
			c->on_entity_child_position_changed(b);
	}

	void EntityPrivate::info_child_removed(EntityPrivate* e) const
	{
		for (auto i = 0; i < e->index; i++)
		{
			auto ee = children[i].get();
			ee->index -= 1;
			for (auto c : ee->local_event_dispatch_list)
				c->on_entity_position_changed();
			for (auto c : child_event_dispatch_list)
				c->on_entity_child_position_changed(ee);
		}

		e->parent = nullptr;
		if (e->world)
			e->leave_world();

		for (auto c : e->local_event_dispatch_list)
			c->on_entity_removed();
		for (auto c : local_event_dispatch_list)
			c->on_entity_removed_child(e);
	}

	void EntityPrivate::remove_child(EntityPrivate* e, bool destroy)
	{
		auto it = std::find_if(children.begin(), children.end(), [&](const auto& t) {
			return t.get() == e;
		});
		if (it == children.end())
		{
			assert(0); // not found!
			return;
		}

		info_child_removed(e);

		if (!destroy)
			it->release();
		children.erase(it);
	}

	void EntityPrivate::remove_all_children(bool destroy)
	{
		for (auto& c : children)
			info_child_removed(c.get());

		if (!destroy)
		{
			for (auto& c : children)
				c.release();
		}
		children.clear();
	}

	EntityPrivate* EntityPrivate::find_child(const std::string& name) const
	{
		for (auto& c : children)
		{
			if (c->name == name)
				return c.get();
		}
		return nullptr;
	}

	static void set_attribute(Component* c, UdtInfo* udt, const std::string& name, const std::string& value)
	{
		auto fs = udt->find_function(("set_" + name).c_str());
		if (fs->get_type() == TypeInfo::get(TypeData, "void") && fs->get_parameters_count() == 1)
		{
			auto type = fs->get_parameter(0);
			if (type->get_tag() == TypePointer)
			{
				auto type_name = std::string(type->get_name());
				if (type_name == "char")
				{
					fs->call(c, nullptr, value.c_str());
					return;
				}
				if (type_name == "wchar_t")
				{
					fs->call(c, nullptr, s2w(value).c_str());
					return;
				}
				type = TypeInfo::get(TypeData, type->get_name());
			}
			void* d = new char[type->get_size()];
			type->unserialize(value.c_str(), d);
			fs->call(c, nullptr, d);
			delete[]d;
		}
	}

	static std::stack<std::filesystem::path> prefab_paths;

	static void load_prefab(EntityPrivate* dst, pugi::xml_node src)
	{
		if (src.name() != std::string("entity"))
			return;

		for (auto a : src.attributes())
		{
			auto name = std::string(a.name());
			if (name == "name")
				dst->name = a.value();
			else if (name == "visible")
				dst->visible = a.as_bool();
			else if (name == "src")
			{
				auto path = std::filesystem::path(a.value());
				path.replace_extension(L".prefab");
				dst->load(prefab_paths.empty() ? path : prefab_paths.top() / path);
			}
			else
			{
				auto sp = SUS::split(name, '.');
				if (sp.size() > 1)
				{
					auto c = dst->get_component(std::hash<std::string>()(sp[0]));
					if (c)
					{
						auto udt = find_udt(("flame::" + sp[0]).c_str());
						set_attribute(c, udt, sp[1], a.value());
					}
				}
			}
		}

		for (auto n_c : src.children())
		{
			auto name = std::string(n_c.name());
			if (name == "entity")
			{
				auto e = EntityPrivate::create();
				dst->add_child(e);
				load_prefab(e, n_c);
			}
			else
			{
				auto udt = find_udt(("flame::" + name).c_str());
				auto fc = udt->find_function("create");
				if (fc->get_type()->get_tag() == TypePointer && fc->get_parameters_count() == 0)
				{
					Component* c = nullptr;
					fc->call(nullptr, &c);
					for (auto a : n_c.attributes())
						set_attribute(c, udt, a.name(), a.value());
					for (auto i : n_c.children())
					{
						auto fa = udt->find_function((std::string("add_") + i.name()).c_str());

					}
					dst->add_component((Component*)c);
				}
			}
		}
	}

	void EntityPrivate::load(const std::filesystem::path& filename)
	{
		pugi::xml_document file;
		pugi::xml_node file_root;

		if (!file.load_file(filename.c_str()) || (file_root = file.first_child()).name() != std::string("prefab"))
			return;

		auto parent_path = filename.parent_path();
		if (!parent_path.empty())
			prefab_paths.push(parent_path);
		load_prefab(this, file_root.first_child());
		if (!parent_path.empty())
			prefab_paths.pop();
	}

	static void save_prefab(pugi::xml_node dst, EntityPrivate* src)
	{
		auto n = dst.append_child("entity");
		n.append_attribute("name").set_value(src->name.empty() ? "unnamed" : src->name.c_str());
		n.append_attribute("visible").set_value(src->visible);

		//if (!src->components.empty())
		//{
		//	auto n_cs = n.append_child("components");
		//	for (auto& c : src->components)
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

		//if (!src->children.empty())
		//{
		//	auto n_es = n.append_child("children");
		//	for (auto& e : src->children)
		//		save_prefab(n_es, e.get());
		//}
	}

	void EntityPrivate::save(const std::filesystem::path& filename)
	{
		pugi::xml_document file;
		auto file_root = file.append_child("prefab");

		save_prefab(file_root, this);

		file.save_file(filename.c_str());
	}

	EntityPrivate* EntityPrivate::create()
	{
		auto ret = _allocate(sizeof(EntityPrivate));
		new (ret) EntityPrivate;
		return (EntityPrivate*)ret;
	}

	Entity* Entity::create() { return EntityPrivate::create(); }
}
