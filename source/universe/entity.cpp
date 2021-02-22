#include <flame/foundation/typeinfo.h>
#include <flame/script/script.h>
#include "entity_private.h"
#include <flame/universe/component.h>
#include <flame/universe/driver.h>
#include "world_private.h"

namespace flame
{
	static bool debug = false;

	struct Type
	{
		struct Attribute
		{
			TypeInfo* get_type;
			TypeInfo* set_type;
			FunctionInfo* getter;
			FunctionInfo* setter;
			std::string default_value;

			Attribute(Type* ct, TypeInfo* get_type, TypeInfo* set_type, FunctionInfo* getter, FunctionInfo* setter) :
				get_type(get_type),
				set_type(set_type),
				getter(getter),
				setter(setter)
			{
				void* d = get_type->create(false);
				getter->call(ct->dummy, d, nullptr);
				get_type->serialize(d, &default_value, [](void* _str, uint size) {
					auto& str = *(std::string*)_str;
					str.resize(size);
					return str.data();
				});
				get_type->destroy(d, false);
			}

			std::string serialize(void* c)
			{
				void* d = get_type->create(false);
				getter->call(c, d, nullptr);
				std::string str;
				get_type->serialize(d, &str, [](void* _str, uint size) {
					auto& str = *(std::string*)_str;
					str.resize(size);
					return str.data();
				});
				get_type->destroy(d, false);
				if (str == default_value)
					str = "";
				return str;
			}
		};

		UdtInfo* udt;
		FunctionInfo* creator;
		void* dummy;

		std::map<std::string, Attribute> attributes;

		Type(UdtInfo* udt) :
			udt(udt)
		{
			auto fc = udt->find_function("create");
			fassert(fc && fc->check(TypeInfo::get(TypePointer, udt->get_name())));
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
						attributes.emplace(std::get<0>(g), Attribute(this, std::get<1>(g), std::get<1>(s), std::get<2>(g), std::get<2>(s)));
						break;
					}
				}
			}
		}

		void* create()
		{
			void* ret;
			creator->call(nullptr, &ret, nullptr);
			return ret;
		}

		Attribute* find_attribute(const std::string& name)
		{
			auto it = attributes.find(name);
			if (it == attributes.end())
				return nullptr;
			return &it->second;
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

		if (debug)
		{

		}
	}

	EntityPrivate::~EntityPrivate()
	{
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
			if (!world || world->root.get() == this)
				global_visibility = true;
			else
				global_visibility = false;
		}
		if (global_visibility != prev_visibility)
		{
			for (auto& c : components)
				c.second.c->on_visibility_changed(global_visibility);
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

		for (auto& r : state_rules)
		{
			void* d = nullptr;
			for (auto& v : r->values)
			{
				if (v.first == StateNone)
				{
					if (!d)
						d = v.second;
				}
				else if ((v.first & state) == v.first)
				{
					d = v.second;
					break;
				}
			}

			r->setter->call(r->o, nullptr, d);
		}

		for (auto& c : components)
			c.second.c->on_state_changed(state);
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

	Component* EntityPrivate::get_component(uint64 hash) const
	{
		auto it = components.find(hash);
		if (it != components.end())
			return it->second.c.get();
		return nullptr;
	}

	Component* EntityPrivate::find_component(const std::string& _name) const
	{
		Component* ret = nullptr;
		auto name = _name;
		for (auto& c : components)
		{
			if (c.second.c->type_name == _name)
			{
				ret = c.second.c.get();
				break;
			}
		}
		name = "flame::" + _name;
		for (auto& c : components)
		{
			if (c.second.c->type_name == name)
			{
				ret = c.second.c.get();
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

	Component* EntityPrivate::find_first_dfs_component(const std::string& name) const
	{
		auto ret = find_component(name);
		if (ret)
			return ret;
		for (auto& c : children)
		{
			auto ret = c->find_first_dfs_component(name);
			if (ret)
				return ret;
		}
		return nullptr;
	}

	void EntityPrivate::add_component(Component* c)
	{
		fassert(!parent || c->type_hash == S<"cSpy"_h>);
		fassert(!c->entity);
		fassert(components.find(c->type_hash) == components.end());

		c->entity = this;

		c->on_added();
		if (world)
			c->on_entered_world();

		ComponentSlot slot;
		slot.c.reset(c);
		slot.id = component_id++;
		components.emplace(c->type_hash, std::move(slot));
	}

	void EntityPrivate::remove_component(Component* c, bool destroy)
	{
		fassert(!parent || c->type_hash == S<"cSpy"_h>);

		auto it = components.find(c->type_hash);
		if (it == components.end())
		{
			fassert(0);
			return;
		}

		if (!destroy)
			it->second.c.release();
		components.erase(it);
	}

	void EntityPrivate::add_child(EntityPrivate* e, int position)
	{
		fassert(e && e != this && !e->parent);

		if (e->redirectable && position == -1)
		{
			for (auto i = (int)drivers.size() - 1; i >= 0; i--)
			{
				if (drivers[i].d->on_child_added(e))
					return;
			}
		}

		if (position == -1)
			position = children.size();

		children.emplace(children.begin() + position, e);

		e->parent = this;
		e->redirectable = false;
		e->depth = depth + 1;
		e->index = position;
		e->update_visibility();

		for (auto& c : e->components)
			c.second.c->on_self_added();

		e->traversal([this](EntityPrivate* e) {
			if (!e->world && world)
			{
				e->world = world;
				for (auto& c : e->components)
					c.second.c->on_entered_world();
			}
			return true;
		});

		for (auto& c : components)
			c.second.c->on_child_added(e);
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

		for (auto& c : a->components)
			c.second.c->on_reposition(pos1, pos2);
		for (auto& c : b->components)
			c.second.c->on_reposition(pos2, pos1);
	}

	void EntityPrivate::on_child_removed(EntityPrivate* e) const
	{
		e->parent = nullptr;
		e->redirectable = true;

		for (auto& c : e->components)
			c.second.c->on_self_removed();

		e->traversal([](EntityPrivate* e) {
			if (e->world)
			{
				for (auto& c : e->components)
					c.second.c->on_left_world();
				e->world = nullptr;
			}

			return true;
		});

		for (auto& c : components)
			c.second.c->on_child_removed(e);
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

	void EntityPrivate::traversal(const std::function<bool(EntityPrivate*)>& callback)
	{
		if (!callback(this))
			return;
		for (auto& c : children)
			c->traversal(callback);
	}

	Driver* EntityPrivate::get_driver(uint64 hash, int idx) const
	{
		if (idx == -1)
		{
			for (auto& d : drivers)
			{
				if (d.d->type_hash == hash)
					return d.d.get();
			}
		}
		else
		{
			idx = max(0, (int)drivers.size() - 1 - idx);
			return drivers[idx].d.get();
		}
		return nullptr;
	}

	Driver* EntityPrivate::find_driver(const std::string& _name) const
	{
		Driver* ret = nullptr;
		auto name = _name;
		for (auto& d : drivers)
		{
			if (d.d->type_name == _name)
			{
				ret = d.d.get();
				break;
			}
		}
		name = "flame::" + _name;
		for (auto& d : drivers)
		{
			if (d.d->type_name == name)
			{
				ret = d.d.get();
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
		EntityPrivate::DriverSlot slot;
		slot.d.reset(d);
		drivers.push_back(std::move(slot));
	}

	void EntityPrivate::pop_driver()
	{
		if (!drivers.empty())
			drivers.pop_back();
	}

	void EntityPrivate::component_data_changed(Component* c, uint64 h)
	{
		auto it = components.find(c->type_hash);
		if (it != components.end())
		{
			for (auto& l : it->second.data_listeners)
				l->call(h);
		}
	}

	void* EntityPrivate::add_component_data_listener(void (*callback)(Capture& c, uint64 h), const Capture& capture, Component* c)
	{
		auto it = components.find(c->type_hash);
		if (it != components.end())
		{
			if (!callback)
			{
				auto slot = (uint)&capture;
				callback = [](Capture& c, uint64 h) {
					auto scr_ins = script::Instance::get_default();
					scr_ins->get_global("callbacks");
					scr_ins->get_member(nullptr, c.data<uint>());
					scr_ins->get_member("f");
					scr_ins->push_pointer((void*)h);
					scr_ins->call(1);
					scr_ins->pop(2);
				};
				auto c = new Closure(callback, Capture().set_data(&slot));
				it->second.data_listeners.emplace_back(c);
				return c;
			}
			auto c = new Closure(callback, capture);
			it->second.data_listeners.emplace_back(c);
			return c;
		}
		return nullptr;
	}

	void EntityPrivate::remove_component_data_listener(void* lis, Component* c)
	{
		auto it = components.find(c->type_hash);
		if (it != components.end())
		{
			std::erase_if(it->second.data_listeners, [&](const auto& i) {
				return i == (decltype(i))lis;
			});
		}
	}

	void EntityPrivate::driver_data_changed(Driver* _d, uint64 h)
	{
		for (auto& d : drivers)
		{
			if (d.d->type_hash == _d->type_hash)
			{
				for (auto& l : d.data_listeners)
					l->call(h);
			}
		}
	}

	void* EntityPrivate::add_driver_data_listener(void (*callback)(Capture& c, uint64 h), const Capture& capture, Driver* _d)
	{
		for (auto& d : drivers)
		{
			if (d.d->type_hash == _d->type_hash)
			{
				if (!callback)
				{
					auto slot = (uint)&capture;
					callback = [](Capture& c, uint64 h) {
						auto scr_ins = script::Instance::get_default();
						scr_ins->get_global("callbacks");
						scr_ins->get_member(nullptr, c.data<uint>());
						scr_ins->get_member("f");
						scr_ins->push_pointer((void*)h);
						scr_ins->call(1);
						scr_ins->pop(2);
					};
					auto c = new Closure(callback, Capture().set_data(&slot));
					d.data_listeners.emplace_back(c);
					return c;
				}
				auto c = new Closure(callback, capture);
				d.data_listeners.emplace_back(c);
				return c;
			}
		}
		return nullptr;
	}

	void EntityPrivate::remove_driver_data_listener(void* lis, Driver* _d)
	{
		for (auto& d : drivers)
		{
			if (d.d->type_hash == _d->type_hash)
			{
				std::erase_if(d.data_listeners, [&](const auto& i) {
					return i == (decltype(i))lis;
				});
				return;
			}
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
			}
		}
	}

	static void load_prefab(EntityPrivate* e_dst, pugi::xml_node n_src, 
		const std::filesystem::path& fn, EntityPrivate* first_e, const std::vector<uint>& los)
	{
		auto set_attribute = [&](void* o, Type* ot, const std::string& name, const std::string& value) {
			auto sp = SUS::split(name, '.');
			std::string vname;
			std::string rule;
			vname = sp[0];
			if (sp.size() == 2)
			{
				vname = sp[0];
				rule = sp[1];
			}
			auto att = ot->find_attribute(vname);
			if (att)
			{
				auto type = att->set_type;
				auto fs = att->setter;
				if (rule == "state_rules")
				{
					auto rule = new StateRule;
					rule->o = o;
					rule->vname = vname;
					rule->type = type;
					rule->setter = fs;
					for (auto& s : SUS::split_with_spaces(value))
					{
						auto pair = SUS::split(s, ':');
						auto state = StateNone;
						void* d = type->create();
						if (pair.size() == 1)
						{
							type->unserialize(d, pair[0].c_str());
							fs->call(o, nullptr, d);
						}
						else if (pair.size() == 2)
						{
							TypeInfo::get(TypeEnumMulti, "flame::StateFlags")
								->unserialize(&state, pair[0].c_str());
							type->unserialize(d, pair[1].c_str());
						}
						else
							fassert(0);
						rule->values.emplace_back(state, d);
					}
					first_e->state_rules.emplace_back(rule);
				}
				else
				{
					void* d = type->create();
					type->unserialize(d, value.c_str());
					fs->call(o, nullptr, d);
					type->destroy(d);
				}
				return true;
			}
			return false;
		};
		auto set_content = [&](Component* c, Type* ct, const std::string& value) {
			auto att = ct->find_attribute("content");
			if (att)
			{
				auto type = att->set_type;
				auto fs = att->setter;
				void* d = type->create();
				type->unserialize(d, value.c_str());
				fs->call(c, nullptr, d);
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
				auto name = std::string(a.name());
				auto value = std::string(a.value());
				for (auto i = (int)e_dst->drivers.size() - 1; i >= 0; i--)
				{
					auto d = e_dst->drivers[i].d.get();
					auto dt = find_driver_type(d->type_name, nullptr);
					if (dt && set_attribute(d, dt, name, value))
					{
						ok = true;
						break;
					}
				}
				if (!ok)
				{
					for (auto& c : e_dst->components)
					{
						auto ct = find_component_type(c.second.c->type_name, nullptr);
						if (ct && set_attribute(c.second.c.get(), ct, name, value))
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
						if (!set_attribute(c, ct, a.name(), a.value()))
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
				auto e = f_new<EntityPrivate>();
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
			auto d = e_dst->drivers.back().d.get();
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

	static void save_prefab(pugi::xml_node n_dst, EntityPrivate* e_src, EntityPrivate* first_e)
	{
		if (!e_src->name.empty())
			n_dst.append_attribute("name").set_value(e_src->name.c_str());
		if (!e_src->visible)
			n_dst.append_attribute("visible").set_value("false");
		if (!e_src->drivers.empty())
		{
			auto d = e_src->drivers.back().d.get();
			//if (d->src_id == ??)
			//{
			//	std::string name;
			//	find_driver_type(d->type_name, &name);
			//	n_dst.append_attribute("driver").set_value(name.c_str());
			//}
		}
		std::filesystem::path src;
		if (!e_src->srcs.empty())
			src = e_src->srcs.back();
		if (!src.empty())
		{
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

		std::vector<std::pair<uint, Component*>> components;
		for (auto& c : e_src->components)
			components.emplace_back(c.second.id, c.second.c.get());
		std::sort(components.begin(), components.end(), [](const auto& a, const auto& b) {
			return a.first < b.first;
		});
		for (auto& c : components)
		{
			std::string cname;
			auto ct = find_component_type(c.second->type_name, &cname);
			fassert(ct);
			auto n_c = n_dst.append_child(cname.c_str());
			for (auto& a : ct->attributes)
			{
				auto isrule = false;
				if (e_src == first_e)
				{
					for (auto& r : e_src->state_rules)
					{
						if (r->vname == a.first)
						{
							isrule = true;
							std::string value;
							for (auto& v : r->values)
							{
								std::string str;
								auto str_alloc = [](void* _str, uint size) {
									auto& str = *(std::string*)_str;
									str.resize(size);
									return str.data();
								};
								if (v.first)
								{
									TypeInfo::get(TypeEnumMulti, "flame::StateFlags")
										->serialize(&v.first, &str, str_alloc);
									value += str;
									value += ":";
								}
								r->type->serialize(v.second, &str, str_alloc);
								value += str;
								value += "  ";
							}
							value.erase(value.end() - 1);
							n_c.append_attribute((a.first + "state_rules").c_str()).set_value(value.c_str());
							break;
						}
					}
				}
				if (!isrule)
				{
					auto value = a.second.serialize(c.second);
					if (!value.empty())
						n_c.append_attribute(a.first.c_str()).set_value(value.c_str());
				}
			}
		}
	}

	bool EntityPrivate::save(const std::filesystem::path& filename)
	{
		pugi::xml_document file;
		auto file_root = file.append_child("prefab");

		save_prefab(file_root.append_child("entity"), this, this);

		file.save_file(filename.c_str());

		return true;
	}

	Entity* Entity::create()
	{
		return f_new<EntityPrivate>();
	}

	void Entity::initialize()
	{
		traverse_udts([](Capture&, UdtInfo* ui) {
			static auto reg_com = std::regex(R"(^flame::(c[\w]+)$)");
			static auto reg_dri = std::regex(R"(^flame::(d[\w]+)$)");
			std::smatch res;
			auto name = std::string(ui->get_name());
			if (std::regex_search(name, res, reg_com))
				component_types.emplace(res[1].str(), ui);
			else if (std::regex_search(name, res, reg_dri))
				driver_types.emplace(res[1].str(), ui);
		}, Capture());

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

	void Entity::set_debug(bool v)
	{
		debug = v;
	}
}
