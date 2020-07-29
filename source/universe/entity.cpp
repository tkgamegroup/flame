#include <flame/foundation/typeinfo.h>
#include <flame/universe/component.h>
#include "entity_private.h"
#include "world_private.h"

namespace flame
{
	EntityPrivate::EntityPrivate()
	{
		created_frame = get_looper()->get_frame();

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
			f_delete(this);
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
			return it->second.p.get();
		return nullptr;
	}

	void EntityPrivate::add_component(Component* c)
	{
		assert(!c->entity);
		assert(components.find(c->type_hash) == components.end());

		ComponentWrapper cw;
		cw.udt[0] = find_udt(c->type_name);
		cw.udt[1] = find_udt((c->type_name + std::string("Private")).c_str());
		if (cw.udt[0])
		{
			auto udt = cw.udt[0];
			if (cw.udt[1])
				udt = cw.udt[1];

			auto vc = udt->get_variables_count();
			for (auto i = 0; i < vc; i++)
			{
				auto v = udt->get_variable(i);
				auto m = v->get_meta();
				if (m->has_token("ref"))
				{
					auto type = std::string(v->get_type()->get_name());
					if (type.ends_with("Private"))
						type = type.substr(0, type.size() - 7);

					auto hash = std::hash<std::string>()(type);
					auto it = components.find(hash);
					if (it == components.end())
					{
						printf("add component %s failed, required component %s do not exist\n", c->type_name, type.c_str());
						return;
					}

					auto name = std::string(v->get_name());

					ComponentReferencing ref;
					ref.t = it->second.p.get();
					ref.dst = (void**)((char*)c + v->get_offset());
					{
						auto f = udt->find_function(("on_gain_" + name).c_str());
						if (f)
							ref.on_gain = a2f<void(*)(void*)>(f->get_address());
						else
							ref.on_gain = nullptr;
					}
					{
						auto f = udt->find_function(("on_lost_" + name).c_str());
						if (f)
							ref.on_lost = a2f<void(*)(void*)>(f->get_address());
						else
							ref.on_lost = nullptr;
					}
					cw.referencings.push_back(ref);
					
					component_monitors[hash].push_back(c);

					*ref.dst = ref.t;
					ref.on_gain(c);
				}
			}

			cw.want_local_event = false;
			if (udt->find_function("on_entered_world") ||
				udt->find_function("on_left_world") ||
				udt->find_function("on_entity_destroyed") ||
				udt->find_function("on_entity_visibility_changed") ||
				udt->find_function("on_entity_state_changed") ||
				udt->find_function("on_entity_added") ||
				udt->find_function("on_entity_removed") ||
				udt->find_function("on_entity_position_changed") ||
				udt->find_function("on_entity_component_added") ||
				udt->find_function("on_entity_component_removed") ||
				udt->find_function("on_entity_added_child") ||
				udt->find_function("on_entity_removed_child"))
				cw.want_local_event = true;
			cw.want_child_event = false;
			if (udt->find_function("on_entity_child_visibility_changed") ||
				udt->find_function("on_entity_child_position_changed") ||
				udt->find_function("on_entity_child_component_added") ||
				udt->find_function("on_entity_child_component_removed"))
				cw.want_child_event = true;
			cw.want_local_data_changed = false;
			if (udt->find_function("on_entity_component_data_changed"))
				cw.want_local_data_changed = true;
			cw.want_child_data_changed = false;
			if (udt->find_function("on_entity_child_component_data_changed"))
				cw.want_child_data_changed = true;
		}
		cw.p.reset(c);

		c->entity = this;
		c->on_added();

		for (auto cc : local_event_dispatch_list)
			cc->on_entity_component_added(c);
		if (parent)
		{
			for (auto cc : parent->child_event_dispatch_list)
				cc->on_entity_child_component_added(c);
		}

		components.emplace(c->type_hash, std::move(cw));
		component_monitors[c->type_hash] = {};

		if (cw.want_local_event)
		{
			local_event_dispatch_list.push_back(c);
			if (world)
				c->on_entered_world();
		}
		if (cw.want_child_event)
			child_event_dispatch_list.push_back(c);
		if (cw.want_local_data_changed)
			local_data_changed_dispatch_list.push_back(c);
		if (cw.want_child_data_changed)
			child_data_changed_dispatch_list.push_back(c);
	}

	void EntityPrivate::info_component_removed(ComponentWrapper& cw)
	{
		for (auto& r : cw.referencings)
		{
			auto& vec = ((EntityPrivate*)r.t->entity)->component_monitors[r.t->type_hash];
			std::erase_if(vec, [&](const auto& i) {
				return i == cw.p.get();
			});
			r.on_lost(cw.p.get());
			*r.dst = nullptr;
		}
		cw.referencings.clear();

		for (auto cc : local_event_dispatch_list)
			cc->on_entity_component_removed(cw.p.get());
		if (parent)
		{
			for (auto cc : parent->child_event_dispatch_list)
				cc->on_entity_child_component_removed(cw.p.get());
		}

		if (cw.want_local_event)
		{
			if (world)
				cw.p->on_left_world();
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

		auto& vec = component_monitors[c->type_hash];
		if (!vec.empty())
		{
			printf("remove component %s failed, this component is referenced by ", c->type_name);
			for (auto& r : vec)
				printf("%s ", r->type_name);
			printf("\n");
			return;
		}

		auto& cw = it->second;

		info_component_removed(cw);

		if (cw.want_local_event)
		{
			std::erase_if(local_event_dispatch_list, [&](const auto& i) {
				return i == c;
			});
		}
		if (cw.want_child_event)
		{
			std::erase_if(child_event_dispatch_list, [&](const auto& i) {
				return i == c;
			});
		}
		if (cw.want_local_data_changed)
		{
			std::erase_if(local_data_changed_dispatch_list, [&](const auto& i) {
				return i == c;
			});
		}
		if (cw.want_child_data_changed)
		{
			std::erase_if(child_data_changed_dispatch_list, [&](const auto& i) {
				return i == c;
			});
		}

		if (!destroy)
			it->second.p.release();
		components.erase(it);
	}

	void EntityPrivate::remove_all_components(bool destroy)
	{
		for (auto& c : components)
			info_component_removed(c.second);

		local_event_dispatch_list.clear();
		child_event_dispatch_list.clear();
		local_data_changed_dispatch_list.clear();
		child_data_changed_dispatch_list.clear();

		if (!destroy)
		{
			for (auto& c : components)
				c.second.p.release();
		}
		components.clear();
	}

	void EntityPrivate::report_data_changed(Component* c, uint hash)
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
		assert(e && e != this);

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
		assert(e && e != this);

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

	struct LoadState
	{
		std::vector<std::pair<std::string, std::string>> nss;
		std::filesystem::path path;

		std::string find_ns(const std::string& n)
		{
			for (auto& ns : nss)
			{
				if (ns.first == n)
					return ns.second;
			}
			return "";
		}
	};
	static std::stack<LoadState> load_states;

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
				dst->load(load_states.empty() ? path : load_states.top().path / path);
			}
		}

		for (auto n_c : src.children())
		{
			auto name = std::string(n_c.name());
			if (name == "entity")
			{
				auto e = f_new<EntityPrivate>();
				dst->add_child(e);
				load_prefab(e, n_c);
			}
			else
			{
				auto sp = SUS::split(name, ':');
				if (sp.size() == 2)
				{
					auto name = load_states.top().find_ns(sp[0]) + "::" + sp[1];
					auto udt = find_udt(name.c_str());
					if (udt)
					{
						auto c = dst->get_component(std::hash<std::string>()(name));
						if (!c)
						{
							auto fc = udt->find_function("create");
							if (fc->get_type()->get_tag() == TypePointer && fc->get_parameters_count() == 0)
							{
								fc->call(nullptr, &c, {});
								dst->add_component((Component*)c);
							}
						}
						if (c)
						{
							for (auto a : n_c.attributes())
							{
								auto fs = udt->find_function((std::string("set_") + a.name()).c_str());
								if (fs->get_type() == TypeInfo::get(TypeData, "void") && fs->get_parameters_count() == 1)
								{
									auto type = fs->get_parameter(0);
									void* d = type->create();
									type->unserialize(d, a.value());
									void* parms[] = { type->get_tag() == TypePointer ? *(void**)d : d };
									fs->call(c, nullptr, parms);
									type->destroy(d);
								}
							}
						}
					}
				}
			}
		}
	}

	void EntityPrivate::load(const std::filesystem::path& filename)
	{
		pugi::xml_document file;
		pugi::xml_node file_root;

		if (!file.load_file(filename.c_str()) || (file_root = file.first_child()).name() != std::string("prefab"))
		{
			printf("prefab not exist or wrong format: %s\n", filename.string().c_str());
			return;
		}

		LoadState state;
		for (auto a : file_root.attributes())
		{
			static std::regex reg_ns(R"(xmlns:(\w+))");
			std::smatch res;
			auto name = std::string(a.name());
			if (std::regex_search(name, res, reg_ns))
				state.nss.emplace_back(res[1].str(), a.value());
		}
		state.path = filename.parent_path();
		load_states.push(state);
		load_prefab(this, file_root.first_child());
		load_states.pop();
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

		//		auto udt = find_udt(component->name);
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
		//					n_c.append_child(v->get_name()).append_attribute("v").set_value(type->serialize(p).c_str());
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

	Entity* Entity::create()
	{
		return f_new<EntityPrivate>();
	}
}
