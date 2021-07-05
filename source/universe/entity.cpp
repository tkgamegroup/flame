#include "../foundation/typeinfo.h"
#include "entity_private.h"
#include "world_private.h"

#include <pugixml.hpp>

namespace flame
{
	struct Type
	{
		struct Attribute
		{
			std::string name;
			uint hash;
			TypeInfo* get_type;
			TypeInfo* set_type;
			FunctionInfo* getter;
			FunctionInfo* setter;
			void* default_value;

			Attribute(Type* ct, const std::string& name, TypeInfo* get_type, TypeInfo* set_type, FunctionInfo* getter, FunctionInfo* setter) :
				name(name),
				get_type(get_type),
				set_type(set_type),
				getter(getter),
				setter(setter)
			{
				hash = ch(name.c_str());

				default_value = get_type->create(false);
				getter->call(ct->dummy, default_value, nullptr);
			}

			~Attribute()
			{
				get_type->destroy(default_value, false);
			}

			std::string serialize(void* c, void* ref = nullptr)
			{
				auto same = false;
				void* d = get_type->create(false);
				getter->call(c, d, nullptr);
				if (ref)
				{
					void* dd = get_type->create(false);
					getter->call(ref, dd, nullptr);
					if (get_type->compare(d, dd))
						same = true;
					get_type->destroy(dd, false);
				}
				else if (get_type->compare(d, default_value))
					same = true;
				std::string str;
				if (!same)
					str = get_type->serialize(d);
				get_type->destroy(d, false);
				return str;
			}
		};

		UdtInfo* udt;
		FunctionInfo* creator;
		void* dummy;

		std::map<std::string, std::unique_ptr<Attribute>> attributes;

		Type(UdtInfo* udt) :
			udt(udt)
		{
			auto fc = udt->find_function("create");
			{
				TypeInfo* parms[] = { TypeInfo::get(TypePointer, "void") };
				fassert(fc && fc->check(TypeInfo::get(TypePointer, udt->get_name()), 1, parms));
			}
			creator = fc;

			dummy = create();
			std::vector<std::tuple<std::string, TypeInfo*, FunctionInfo*>> getters;
			std::vector<std::tuple<std::string, TypeInfo*, FunctionInfo*>> setters;
			auto num_funs = udt->get_functions_count();
			for (auto i = 0; i < num_funs; i++)
			{
				auto f = udt->get_function(i);
				auto name = std::string(f->get_name());
				auto ft = f->get_type();
				auto pc = f->get_parameters_count();
				if (name.compare(0, 4, "get_") == 0 && pc == 0)
				{
					if (ft->get_name() != std::string("void"))
						getters.emplace_back(name.substr(4), ft, f);
				}
				else if (name.compare(0, 4, "set_") == 0 && pc == 1 && ft == TypeInfo::get(TypeData, ""))
				{
					auto t = f->get_parameter(0);
					if (t->get_name() != std::string("void"))
						setters.emplace_back(name.substr(4), t, f);
				}
			}
			for (auto& g : getters)
			{
				for (auto& s : setters)
				{
					if (std::get<0>(g) == std::get<0>(s) && std::string(std::get<1>(g)->get_name()) == std::string(std::get<1>(s)->get_name()))
					{
						attributes.emplace(std::get<0>(g), new Attribute(this, std::get<0>(g), std::get<1>(g), std::get<1>(s), std::get<2>(g), std::get<2>(s)));
						break;
					}
				}
			}
		}

		void* create()
		{
			return a2f<void*(*)(void*)>(creator->get_address())(nullptr);
		}

		Attribute* find_attribute(const std::string& name)
		{
			auto it = attributes.find(name);
			if (it == attributes.end())
				return nullptr;
			return it->second.get();
		}
	};

	static std::map<std::string, Type> component_types;

	Type* find_component_type(const std::string& name)
	{
		auto it = component_types.find(name);
		if (it == component_types.end())
			return nullptr;
		return &it->second;
	}

	Type* find_component_type(const std::string& udt_name, std::string* name)
	{
		for (auto& t : component_types)
		{
			if (t.second.udt->get_name() == udt_name)
			{
				if (name)
					*name = t.first;
				return &t.second;
			}
		}
		return nullptr;
	}

	static std::map<std::string, Type> driver_types;

	Type* find_driver_type(const std::string& name)
	{
		auto it = driver_types.find(name);
		if (it == driver_types.end())
			return nullptr;
		return &it->second;
	}

	Type* find_driver_type(const std::string& udt_name, std::string* name)
	{
		for (auto& t : driver_types)
		{
			if (t.second.udt->get_name() == udt_name)
			{
				if (name)
					*name = t.first;
				return &t.second;
			}
		}
		return nullptr;
	}

	static std::map<std::string, std::filesystem::path> prefabs_map;

	StateRule::~StateRule()
	{
		for (auto& v : values)
			type->destroy(v.second);
	}

	EntityPrivate::EntityPrivate()
	{
		static auto id = 0;
		created_frame = looper().get_frame();
		created_id = id++;
	}

	EntityPrivate::~EntityPrivate()
	{
		for (auto& l : message_listeners)
			l->call(S<"destroyed"_h>, nullptr, nullptr);
		for (auto& c : components)
			c->on_destroyed();
		for (auto ev : events)
			looper().remove_event(ev);
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
			if (!world)
				global_visibility = true;
			else
				global_visibility = false;
		}
		if (global_visibility != prev_visibility)
		{
			for (auto& l : message_listeners)
				l->call(S<"visibility_changed"_h>, global_visibility ? (void*)1 : nullptr, nullptr);
			for (auto& c : components)
				c->on_visibility_changed(global_visibility);
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
		last_state = state;
		state = s;

		for (auto& r : state_rules)
		{
			void* ps[] = { nullptr };
			for (auto& v : r->values)
			{
				if (v.first == StateNone)
				{
					if (!ps[0])
						ps[0] = v.second;
				}
				else if ((v.first & state) == v.first)
				{
					ps[0] = v.second;
					break;
				}
			}
			r->setter->call(r->o, nullptr, ps);
		}

		for (auto& l : message_listeners)
			l->call(S<"state_changed"_h>, (void*)state, (void*)last_state);
		for (auto& c : components)
			c->on_state_changed(state);
	}

	void EntityPrivate::add_src(const std::filesystem::path& p)
	{
		srcs.insert(srcs.begin(), p);
		srcs_str += p.wstring() + L";";
	}

	const wchar_t* EntityPrivate::get_srcs() const
	{
		return srcs_str.c_str();
	}

	Component* EntityPrivate::get_component(uint hash) const
	{
		auto it = components_map.find(hash);
		if (it != components_map.end())
			return it->second.first;
		return nullptr;
	}

	Component* EntityPrivate::find_component(const std::string& _name) const
	{
		Component* ret = nullptr;
		auto name = _name;
		for (auto& c : components)
		{
			if (c->type_name == _name)
			{
				ret = c.get();
				break;
			}
		}
		name = "flame::" + _name;
		for (auto& c : components)
		{
			if (c->type_name == name)
			{
				ret = c.get();
				break;
			}
		}
		if (ret)
		{
			auto script = script::Instance::get_default();
			script->push_string(name.c_str());
			script->set_global_name("__type__");
		}
		return ret;
	}

	void EntityPrivate::get_components(void (*callback)(Capture& c, Component* cmp), const Capture& capture) const
	{
		if (!callback)
		{
			auto scr_ins = script::Instance::get_default();
			scr_ins->get_global("callbacks");
			scr_ins->get_member(nullptr, (uint)&capture);
			for (auto& c : components)
			{
				scr_ins->get_member("f");
				scr_ins->push_object();
				scr_ins->set_object_type("flame::Component", c.get());
				scr_ins->call(1);
			}
			scr_ins->pop(2);
		}
		else
		{
			for (auto& c : components)
				callback((Capture&)capture, c.get());
			f_free(capture._data);
		}
	}

	void EntityPrivate::add_component(Component* c)
	{
		fassert(!parent);
		fassert(!c->entity);
		fassert(components_map.find(c->type_hash) == components_map.end());

		c->entity = this;

		for (auto& _c : components)
			_c->on_component_added(c);
		c->on_added();
		if (world)
			c->on_entered_world();

		components.emplace_back(c);
		components_map.emplace(c->type_hash, std::make_pair(c, DataListeners()));
	}

	void EntityPrivate::remove_component(Component* c, bool destroy)
	{
		fassert(!parent);

		auto it = components_map.find(c->type_hash);
		if (it == components_map.end())
		{
			fassert(0);
			return;
		}
		components_map.erase(it);

		for (auto it = components.begin(); it != components.end(); it++)
		{
			if (it->get() == c)
			{
				for (auto& _c : components)
					_c->on_component_removed(c);
				c->on_removed();
				if (!destroy)
					it->release();
				components.erase(it);
				break;
			}
		}
	}

	void EntityPrivate::add_child(EntityPrivate* e, int position)
	{
		fassert(e && e != this && !e->parent);

		uint pos;
		if (position == -1)
			pos = children.size();
		else
			pos = position;

		if (position == -1)
		{
			for (auto it = drivers.rbegin(); it != drivers.rend(); it++)
			{
				if ((*it)->on_child_added(e, pos))
					return;
			}
		}

		children.emplace(children.begin() + pos, e);

		e->parent = this;
		e->depth = depth + 1;
		e->index = pos;
		e->update_visibility();

		for (auto& l : e->message_listeners)
			l->call(S<"self_added"_h>, nullptr, nullptr);
		for (auto& c : e->components)
			c->on_entity_added();

		e->traversal([this](EntityPrivate* e) {
			if (!e->world && world)
				e->on_entered_world(world);
			return true;
		});

		for (auto& l : message_listeners)
			l->call(S<"child_added"_h>, e, nullptr);
		for (auto& c : components)
			c->on_child_added(e);
	}

	void EntityPrivate::reposition_child(uint pos1, uint pos2)
	{
		if (pos1 == pos2)
			return;
		fassert(pos1 < children.size() && pos2 < children.size());

		auto a = children[pos1].get();
		auto b = children[pos2].get();
		a->index = pos2;
		b->index = pos1;
		std::swap(children[pos1], children[pos2]);

		for (auto& l : message_listeners)
			l->call(S<"reposition"_h>, (void*)pos1, (void*)pos1);
		for (auto& c : a->components)
			c->on_reposition(pos1, pos2);
		for (auto& c : b->components)
			c->on_reposition(pos2, pos1);
	}

	void EntityPrivate::on_child_removed(EntityPrivate* e) const
	{
		e->parent = nullptr;

		for (auto& l : e->message_listeners)
			l->call(S<"self_removed"_h>, nullptr, nullptr);
		for (auto& c : e->components)
			c->on_entity_removed();

		e->traversal([](EntityPrivate* e) {
			if (e->world)
				e->on_left_world();

			return true;
		});

		for (auto& l : message_listeners)
			l->call(S<"child_removed"_h>, e, nullptr);
		for (auto& c : components)
			c->on_child_removed(e);
	}

	void EntityPrivate::remove_child(EntityPrivate* e, bool destroy)
	{
		fassert(e && e != this);

		auto it = std::find_if(children.begin(), children.end(), [&](const auto& t) {
			return t.get() == e;
		});
		if (it == children.end())
		{
			fassert(0);
			return;
		}

		on_child_removed(e);

		if (!destroy)
			it->release();
		children.erase(it);
	}

	void EntityPrivate::remove_all_children(bool destroy)
	{
		for (auto i = (int)children.size() - 1; i >= 0; i--)
		{
			auto e = children[i].get();
			on_child_removed(e);
			if (!destroy)
				children[i].release();
		}
		children.clear();
	}

	EntityPrivate* EntityPrivate::find_child(const std::string& name) const
	{
		for (auto& c : children)
		{
			if (c->name == name)
				return c.get();
			auto res = c->find_child(name);
			if (res)
				return res;
		}
		return nullptr;
	}

	void EntityPrivate::on_entered_world(WorldPrivate* _world)
	{
		world = _world;
		for (auto& l : message_listeners)
			l->call(S<"entered_world"_h>, nullptr, nullptr);
		for (auto& c : components)
			c->on_entered_world();
	}

	void EntityPrivate::on_left_world()
	{
		for (auto& l : message_listeners)
			l->call(S<"left_world"_h>, nullptr, nullptr);
		for (auto& c : components)
			c->on_left_world();
		world = nullptr;
	}

	void EntityPrivate::traversal(const std::function<bool(EntityPrivate*)>& callback)
	{
		if (!callback(this))
			return;
		for (auto& c : children)
			c->traversal(callback);
	}

	Driver* EntityPrivate::get_driver(uint hash, int idx) const
	{
		if (idx == -1)
		{
			for (auto& d : drivers)
			{
				if (d->type_hash == hash)
					return d.get();
			}
		}
		else
		{
			idx = max(0, (int)drivers.size() - 1 - idx);
			return drivers[idx].get();
		}
		return nullptr;
	}

	Driver* EntityPrivate::find_driver(const std::string& _name) const
	{
		Driver* ret = nullptr;
		auto name = _name;
		for (auto& d : drivers)
		{
			if (d->type_name == _name)
			{
				ret = d.get();
				break;
			}
		}
		name = "flame::" + _name;
		for (auto& d : drivers)
		{
			if (d->type_name == name)
			{
				ret = d.get();
				break;
			}
		}
		if (ret)
		{
			auto script = script::Instance::get_default();
			script->push_string(name.c_str());
			script->set_global_name("__type__");
		}
		return ret;
	}

	void EntityPrivate::push_driver(Driver* d)
	{
		fassert(!d->entity && d->entity != this);
		fassert(!get_driver(d->type_hash));

		d->entity = this;
		drivers.emplace_back(d);
		drivers_map.emplace(d->type_hash, std::make_pair(d, DataListeners()));
	}

	void EntityPrivate::pop_driver()
	{
		if (!drivers.empty())
			drivers.pop_back();
	}

	void* EntityPrivate::add_message_listener(void (*callback)(Capture& c, uint msg, void* parm1, void* parm2), const Capture& capture)
	{
		if (!callback)
		{
			auto slot = (uint)&capture;
			callback = [](Capture& c, uint msg, void* parm1, void* parm2) {
				auto scr_ins = script::Instance::get_default();
				scr_ins->get_global("callbacks");
				scr_ins->get_member(nullptr, c.data<uint>());
				scr_ins->get_member("f");
				scr_ins->push_uint(msg);
				scr_ins->push_pointer(parm1);
				scr_ins->push_pointer(parm2);
				scr_ins->call(3);
				scr_ins->pop(2);
			};
			auto c = new Closure(callback, Capture().set_data(&slot));
			message_listeners.emplace_back(c);
			return c;
		}
		auto c = new Closure(callback, capture);
		message_listeners.emplace_back(c);
		return c;
	}

	void EntityPrivate::remove_message_listener(void* lis)
	{
		std::erase_if(message_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void EntityPrivate::component_data_changed(Component* c, uint h)
	{
		auto it = components_map.find(c->type_hash);
		if (it != components_map.end())
		{
			for (auto& l : it->second.second)
				l->call(h);
		}
	}

	void* EntityPrivate::add_component_data_listener(void (*callback)(Capture& c, uint h), const Capture& capture, Component* c)
	{
		auto it = components_map.find(c->type_hash);
		if (it != components_map.end())
		{
			if (!callback)
			{
				auto slot = (uint)&capture;
				callback = [](Capture& c, uint h) {
					auto scr_ins = script::Instance::get_default();
					scr_ins->get_global("callbacks");
					scr_ins->get_member(nullptr, c.data<uint>());
					scr_ins->get_member("f");
					scr_ins->push_uint(h);
					scr_ins->call(1);
					scr_ins->pop(2);
				};
				auto c = new Closure(callback, Capture().set_data(&slot));
				it->second.second.emplace_back(c);
				return c;
			}
			auto c = new Closure(callback, capture);
			it->second.second.emplace_back(c);
			return c;
		}
		return nullptr;
	}

	void EntityPrivate::remove_component_data_listener(void* lis, Component* c)
	{
		auto it = components_map.find(c->type_hash);
		if (it != components_map.end())
		{
			std::erase_if(it->second.second, [&](const auto& i) {
				return i == (decltype(i))lis;
			});
		}
	}

	void EntityPrivate::driver_data_changed(Driver* d, uint h)
	{
		auto it = drivers_map.find(d->type_hash);
		if (it != drivers_map.end())
		{
			for (auto& l : it->second.second)
				l->call(h);
		}
	}

	void* EntityPrivate::add_driver_data_listener(void (*callback)(Capture& c, uint h), const Capture& capture, Driver* d)
	{
		auto it = drivers_map.find(d->type_hash);
		if (it != drivers_map.end())
		{
			if (!callback)
			{
				auto slot = (uint)&capture;
				callback = [](Capture& c, uint h) {
					auto scr_ins = script::Instance::get_default();
					scr_ins->get_global("callbacks");
					scr_ins->get_member(nullptr, c.data<uint>());
					scr_ins->get_member("f");
					scr_ins->push_uint(h);
					scr_ins->call(1);
					scr_ins->pop(2);
				};
				auto c = new Closure(callback, Capture().set_data(&slot));
				it->second.second.emplace_back(c);
				return c;
			}
			auto c = new Closure(callback, capture);
			it->second.second.emplace_back(c);
			return c;
		}
		return nullptr;
	}

	void EntityPrivate::remove_driver_data_listener(void* lis, Driver* d)
	{
		auto it = drivers_map.find(d->type_hash);
		if (it != drivers_map.end())
		{
			std::erase_if(it->second.second, [&](const auto& i) {
				return i == (decltype(i))lis;
			});
		}
	}

	void* EntityPrivate::add_event(void (*callback)(Capture& c), const Capture& capture, float interval)
	{
		CountDown cd;
		if (interval != 0.f)
		{
			cd.is_frame = false;
			cd.v.time = interval;
		}
		if (!callback)
		{
			auto slot = (uint)&capture;
			callback = [](Capture& c) {
				auto scr_ins = script::Instance::get_default();
				scr_ins->get_global("callbacks");
				scr_ins->get_member(nullptr, c.data<uint>());
				scr_ins->get_member("f");
				scr_ins->call(0);
				scr_ins->pop(2);
				c._current = nullptr;
			};
			auto ev = looper().add_event(callback, Capture().set_data(&slot), cd);
			events.push_back(ev);
			return ev;
		}
		auto ev = looper().add_event(callback, capture, cd);
		events.push_back(ev);
		return ev;
	}

	void EntityPrivate::remove_event(void* ev)
	{
		for (auto it = events.begin(); it != events.end(); it++)
		{
			if (*it == ev)
			{
				events.erase(it);
				looper().remove_event(ev);
				return;
			}
		}
	}

	static void load_prefab(EntityPrivate* e_dst, pugi::xml_node n_src, 
		const std::filesystem::path& fn, EntityPrivate* first_e, const std::vector<uint>& los)
	{
		auto ti_stateflags = TypeInfo::get(TypeEnumMulti, "flame::StateFlags");
		auto set_attribute = [&](void* o, Type* ot, const std::string& vname, const std::string& value, bool is_state_rule) {
			auto att = ot->find_attribute(vname);
			if (!att)
				return false;

			auto type = att->set_type;
			auto fs = att->setter;
			if (is_state_rule)
			{
				StateRule* rule = nullptr;
				for (auto& r : first_e->state_rules)
				{
					if (r->o == o && r->vname == vname)
					{
						rule = r.get();
						rule->values.clear();
						break;
					}
				}
				if (!rule)
				{
					rule = new StateRule;
					rule->o = o;
					rule->vname = vname;
					rule->type = type;
					rule->setter = fs;
					first_e->state_rules.emplace_back(rule);
				}
				for (auto& s : SUS::split_with_spaces(value))
				{
					auto sp = SUS::split(s, ':');
					fassert(sp.size() <= 2);

					auto state = StateNone;
					void* d = type->create();
					if (sp.size() == 1)
					{
						type->unserialize(d, sp[0].c_str());
						void* ps[] = { d };
						fs->call(o, nullptr, ps);
					}
					else if (sp.size() == 2)
					{
						ti_stateflags->unserialize(&state, sp[0].c_str());
						type->unserialize(d, sp[1].c_str());
					}
					rule->values.emplace_back(state, d);
				}
			}
			else
			{
				void* d = type->create();
				type->unserialize(d, value.c_str());
				void* ps[] = { d };
				fs->call(o, nullptr, ps);
				type->destroy(d);
			}
			return true;
		};
		auto set_content = [&](Component* c, Type* ct, const std::string& value) {
			auto att = ct->find_attribute("content");
			if (att)
			{
				auto type = att->set_type;
				auto fs = att->setter;
				void* d = type->create();
				type->unserialize(d, value.c_str());
				void* ps[] = { d };
				fs->call(c, nullptr, ps);
				type->destroy(d);
				return true;
			}
			return false;
		};
		auto ename = std::string(n_src.name());
		if (ename != "entity")
		{
			auto it = prefabs_map.find(ename);
			if (it != prefabs_map.end())
				e_dst->load(it->second);
			else
				printf("cannot find prefab: %s\n", ename.c_str());
		}

		for (auto a : n_src.attributes())
		{
			auto name = std::string(a.name());
			if (name == "name")
				e_dst->name = a.value();
			else if (name == "visible")
				e_dst->visible = a.as_bool();
			else if (name == "src")
				e_dst->load(a.value());
			else if (name == "driver")
			{
				fassert(first_e == e_dst);
				Type* dt = find_driver_type(a.value());
				if (dt)
				{
					auto d = (Driver*)dt->create();
					auto& srcs = e_dst->srcs;
					for (auto i = (int)srcs.size() - 1; i >= 0; i--)
					{
						if (srcs[i] == fn)
						{
							d->src_id = srcs.size() - i - 1;
							break;
						}
					}
					e_dst->push_driver(d);
				}
				else
					printf("cannot find driver type: %s\n", a.value());
			}
			else
			{
				auto ok = false;
				auto sp = SUS::split(std::string(a.name()), '.');
				auto value = std::string(a.value());
				auto is_state_rule = false;
				if (sp.size() >= 2 && sp.back() == "state_rule")
				{
					is_state_rule = true;
					sp.pop_back();
				}
				fassert(sp.size() <= 2);
				auto vname = sp.back();
				auto e = e_dst;
				if (sp.size() > 1)
				{
					e = e_dst->find_child(sp[0]);
					if (!e)
						printf("cannot find child: %s\n", sp[0].c_str());
				}
				if (e)
				{
					for (auto it = e->drivers.rbegin(); it != e->drivers.rend(); it++)
					{
						auto d = (*it).get();
						auto dt = find_driver_type(d->type_name, nullptr);
						if (dt && set_attribute(d, dt, vname, value, is_state_rule))
						{
							ok = true;
							break;
						}
					}
					if (!ok)
					{
						for (auto& c : e->components)
						{
							auto ct = find_component_type(c->type_name, nullptr);
							if (ct && set_attribute(c.get(), ct, vname, value, is_state_rule))
							{
								ok = true;
								break;
							}
						}
					}
					if (!ok)
						printf("cannot find attribute: %s\n", a.name());
				}
			}
		}

		for (auto n_c : n_src.children())
		{
			auto name = std::string(n_c.name());
			if (name[0] == 'c')
			{
				Type* ct = find_component_type(name);
				if (ct)
				{
					auto c = e_dst->get_component(std::hash<std::string>()(ct->udt->get_name()));
					auto isnew = false;
					if (!c)
					{
						c = (Component*)ct->create();
						auto& srcs = e_dst->srcs;
						for (auto i = (int)srcs.size() - 1; i >= 0; i--)
						{
							if (srcs[i] == fn)
							{
								c->src_id = srcs.size() - i - 1;
								break;
							}
						}
						isnew = true;
					}
					for (auto a : n_c.attributes())
					{
						auto sp = SUS::split(std::string(a.name()), '.');
						if (!set_attribute(c, ct, sp.front(), a.value(), sp.size() == 2 && sp.back() == "state_rule"))
							printf("cannot find attribute: %s\n", a.name());
					}
					auto content = std::string(n_c.child_value());
					if (!content.empty())
					{
						if (!set_content(c, ct, content))
							printf("cannot find attribute: content\n");
					}
					if (isnew)
						e_dst->add_component(c);
				}
				else
					printf("cannot find component: %s\n", name.c_str());
			}
			else if (name[0] == 'e')
			{
				auto e = new EntityPrivate();
				e->add_src(fn);
				auto offset = n_c.offset_debug();
				for (auto i = 0; i < los.size(); i++)
				{
					if (offset < los[i])
					{
						e->created_location = i + 1;
						break;
					}
				}
				load_prefab(e, n_c, fn, first_e, los);
				e_dst->add_child(e);
			}
		}

		if (first_e == e_dst && !e_dst->drivers.empty())
		{
			auto d = e_dst->drivers.back().get();
			if (!d->load_finished)
			{
				d->load_finished = true;
				d->on_load_finished();
			}
		}
	}

	static std::filesystem::path get_prefab_path(const std::filesystem::path& filename)
	{
		auto fn = filename;

		if (fn.extension().empty())
			fn.replace_extension(L".prefab");
		if (!std::filesystem::exists(fn))
		{
			auto engine_path = getenv("FLAME_PATH");
			if (engine_path)
				fn = std::filesystem::path(engine_path) / L"assets" / fn;
		}

		return fn;
	}

	EntityPtr EntityPrivate::copy()
	{
		auto ret = new EntityPrivate();
		ret->name = name;
		ret->visible = visible;
		ret->srcs = srcs;
		ret->srcs_str = srcs_str;
		for (auto& c : components)
		{
			std::string type_name = c->type_name;
			SUS::cut_head_if(type_name, "flame::");
			auto ct = find_component_type(type_name);
			fassert(ct);
			auto cc = (Component*)ct->create();
			cc->src_id = c->src_id;
			for (auto& a : ct->attributes)
			{
				auto attr = a.second.get();
				if (!attr->getter->get_meta(MetaSecondaryAttribute, nullptr))
				{
					auto d = attr->get_type->create(false);
					attr->getter->call(c.get(), d, nullptr);
					if (!attr->get_type->compare(d, attr->default_value))
					{
						if (attr->get_type->get_tag() == TypeData && attr->set_type->get_tag() == TypePointer)
						{
							void* ps[] = { &d };
							attr->setter->call(cc, nullptr, ps);
						}
						else
						{
							void* ps[] = { d };
							attr->setter->call(cc, nullptr, ps);
						}
					}
					attr->get_type->destroy(d, false);
				}
			}
			ret->add_component(cc);
		}
		for (auto& c : children)
		{
			auto cc = c->copy();
			ret->add_child(cc);
		}
		return ret;
	}

	bool EntityPrivate::load(const std::filesystem::path& filename)
	{
		pugi::xml_document doc;
		pugi::xml_node doc_root;

		auto fn = get_prefab_path(filename);
		if (!doc.load_file(fn.c_str()) || (doc_root = doc.first_child()).name() != std::string("prefab"))
		{
			printf("prefab do not exist or wrong format: %s\n", filename.string().c_str());
			return false;
		}

		std::vector<uint> los;
		{
			std::ifstream file(fn);
			while (!file.eof())
			{
				std::string line;
				std::getline(file, line);
				los.push_back(file.tellg());
			}
			file.close();
		}

		if (!fn.is_absolute())
			fn = std::filesystem::canonical(fn);
		fn.make_preferred();
		add_src(fn);
		load_prefab(this, doc_root.first_child(), fn, this, los);

		return true;
	}

	static void save_prefab(pugi::xml_node n_dst, EntityPrivate* e_src, const std::filesystem::path& fn, EntityPrivate* first_e, 
		std::vector<std::pair<std::filesystem::path, std::unique_ptr<EntityPrivate>>>& references)
	{
		auto& srcs = e_src->srcs;
		if (srcs.empty() || srcs.back() != fn)
		{
			n_dst.parent().remove_child(n_dst);
			return;
		}

		EntityPrivate* reference = nullptr;
		if (srcs.size() >= 2)
		{
			auto src = srcs[srcs.size() - 2];

			for (auto& r : references)
			{
				if (r.first == src)
				{
					reference = r.second.get();
					break;
				}
			}
			if (!reference)
			{
				reference = new EntityPrivate;
				reference->load(src);
				references.emplace_back(src, reference);
			}

			auto found = false;
			for (auto& pair : prefabs_map)
			{
				if (pair.second == src)
				{
					n_dst.set_name(pair.first.c_str());
					found = true;
					break;
				}
			}
			if (!found)
				n_dst.append_attribute("src").set_value(src.string().c_str());
		}

		if (!e_src->name.empty())
			n_dst.append_attribute("name").set_value(e_src->name.c_str());
		if (!e_src->visible)
			n_dst.append_attribute("visible").set_value("false");
		if (!e_src->drivers.empty())
		{
			auto d = e_src->drivers.back().get();
			std::string dname;
			find_driver_type(d->type_name, &dname);
			if (srcs[srcs.size() - d->src_id - 1] == fn)
				n_dst.append_attribute("driver").set_value(dname.c_str());
			for (auto& d : e_src->drivers)
			{
				auto dt = find_driver_type(d->type_name, nullptr);
				auto ref = reference ? reference->get_driver(d->type_hash) : nullptr;
				for (auto& a : dt->attributes)
				{
					auto value = a.second->serialize(d.get(), ref);
					if (!value.empty())
						n_dst.append_attribute(a.first.c_str()).set_value(value.c_str());
				}
			}
		}

		auto ti_stateflags = TypeInfo::get(TypeEnumMulti, "flame::StateFlags");
		for (auto& c : e_src->components)
		{
			std::string cname;
			auto ct = find_component_type(c->type_name, &cname);
			fassert(ct);
			auto ref = reference ? reference->get_component(c->type_hash) : nullptr;
			auto put_attributes = [&](pugi::xml_node n) {
				for (auto& a : ct->attributes)
				{
					auto find_rule = [&](const std::string& name)->StateRule* {
						for (auto& r : e_src->state_rules)
						{
							if (r->vname == name)
								return r.get();
						}
						return nullptr;
					};
					auto rule = find_rule(a.first);
					if (rule && e_src == first_e)
					{
						std::string value;
						for (auto& v : rule->values)
						{
							if (v.first)
							{
								value += ti_stateflags->serialize(&v.first);
								value += ":";
							}
							value += rule->type->serialize(v.second);
							value += "  ";
						}
						value.pop_back();
						n.append_attribute((a.first + "state_rules").c_str()).set_value(value.c_str());
					}
					else
					{
						if (!a.second->getter->get_meta(MetaSecondaryAttribute, nullptr))
						{
							auto value = a.second->serialize(c.get(), ref);
							if (!value.empty())
							{
								if (a.first == "content")
									n.append_child(pugi::node_pcdata).set_value(value.c_str());
								else
									n.append_attribute(a.first.c_str()).set_value(value.c_str());
							}
						}
					}
				}
			};
			if (srcs[srcs.size() - c->src_id - 1] == fn)
				put_attributes(n_dst.append_child(cname.c_str()));
			else
				put_attributes(n_dst);

		}

		for (auto& c : e_src->children)
			save_prefab(n_dst.append_child("entity"), c.get(), fn, first_e, references);
	}

	bool EntityPrivate::save(const std::filesystem::path& filename)
	{
		pugi::xml_document file;
		auto file_root = file.append_child("prefab");

		std::vector<std::pair<std::filesystem::path, std::unique_ptr<EntityPrivate>>> references;
		save_prefab(file_root.append_child("entity"), this, srcs.empty() ? L"" : srcs.back(), this, references);

		file.save_file(filename.c_str());

		return true;
	}

	Entity* Entity::create()
	{
		return new EntityPrivate();
	}

	void Entity::initialize()
	{
		std::vector<UdtInfo*> udts;
		{
			uint len;
			get_udts(nullptr, &len);
			udts.resize(len);
			get_udts(udts.data(), nullptr);
		}
		for (auto ui : udts)
		{
			static auto reg_com = std::regex(R"(^flame::(c\w+)$)");
			static auto reg_dri = std::regex(R"(^flame::(d\w+)$)");
			std::smatch res;
			auto name = std::string(ui->get_name());
			if (std::regex_search(name, res, reg_com))
				component_types.emplace(res[1].str(), ui);
			else if (std::regex_search(name, res, reg_dri))
				driver_types.emplace(res[1].str(), ui);
		}

		auto engine_path = getenv("FLAME_PATH");
		if (engine_path)
		{
			auto path = std::filesystem::path(engine_path) / L"assets/prefabs";
			for (std::filesystem::directory_iterator end, it(path); it != end; it++)
			{
				if (it->path().extension() == L".prefab")
				{
					std::string name = "e";
					auto sp = SUS::split(it->path().filename().stem().string(), '_');
					for (auto& t : sp)
					{
						t[0] = std::toupper(t[0]);
						name += t;
					}
					prefabs_map[name] = it->path();
				}
			}
		}
	}
}
