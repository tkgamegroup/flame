#include <flame/foundation/typeinfo.h>
#include <flame/script/script.h>
#include "entity_private.h"
#include <flame/universe/component.h>
#include <flame/universe/driver.h>
#include "world_private.h"

namespace flame
{
	static bool debug = false;

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

			void* parms[] = { d };
			r->setter->call(r->c, nullptr, parms);
		}

		for (auto& c : components)
			c.second.c->on_state_changed(state);
	}

	Component* EntityPrivate::get_component(uint64 hash) const
	{
		auto it = components.find(hash);
		if (it != components.end())
			return it->second.c.get();
		return nullptr;
	}

	Component* EntityPrivate::get_component_n(const char* _name) const
	{
		auto name = std::string(_name);
		for (auto& c : components)
		{
			if (c.second.c->type_name == name)
				return c.second.c.get();
		}
		name = "flame::" + name;
		for (auto& c : components)
		{
			if (c.second.c->type_name == name)
				return c.second.c.get();
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

	void EntityPrivate::add_component(Component* c)
	{
		fassert(!parent);
		fassert(!c->entity);
		fassert(components.find(c->type_hash) == components.end());

		c->entity = this;

		c->on_added();

		ComponentSlot slot;
		slot.c.reset(c);
		components.emplace(c->type_hash, std::move(slot));
	}

	void EntityPrivate::remove_component(Component* c, bool destroy)
	{
		fassert(!parent);

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

	void EntityPrivate::remove_all_components(bool destroy)
	{
		if (!destroy)
		{
			for (auto& c : components)
				c.second.c.release();
		}
		components.clear();
	}

	void EntityPrivate::add_child(EntityPrivate* e, int position)
	{
		fassert(e && e != this && !e->parent);

		if (position == -1)
			position = children.size();

		children.emplace(children.begin() + position, e);

		e->parent = this;
		e->depth = depth + 1;
		e->index = position;
		e->update_visibility();

		for (auto& c : e->components)
			c.second.c->on_self_added();

		e->traversal([this](EntityPrivate* e) {
			e->world = world;

			for (auto& c : e->components)
			{
				if (world)
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
			fassert(0); // not found!
			return;
		}

		on_child_removed(e);

		if (!destroy)
			it->release();
		children.erase(it);
	}

	void EntityPrivate::remove_all_children(bool destroy)
	{
		for (auto& c : children)
			on_child_removed(c.get());

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
			auto res = c->find_child(name);
			if (res)
				return res;
		}
		return nullptr;
	}

	void EntityPrivate::data_changed(Component* c, uint64 h)
	{
		auto it = components.find(c->type_hash);
		if (it != components.end())
		{
			for (auto& l : it->second.data_listeners)
				l->call(h);
		}
	}

	void* EntityPrivate::add_data_listener(Component* c, void (*callback)(Capture& c, uint64 h), const Capture& capture)
	{
		auto it = components.find(c->type_hash);
		if (it != components.end())
		{
			auto c = new Closure(callback, capture);
			it->second.data_listeners.emplace_back(c);
			return c;
		}
		return nullptr;
	}

	void EntityPrivate::remove_data_listener(Component* c, void* lis)
	{
		auto it = components.find(c->type_hash);
		if (it != components.end())
		{
			std::erase_if(it->second.data_listeners, [&](const auto& i) {
				return i == (decltype(i))lis;
			});
		}
	}

	void* EntityPrivate::add_event(void (*callback)(Capture& c), const Capture& capture)
	{
		auto ev = looper().add_event(callback, capture);
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
		const std::filesystem::path& filename,
		const std::vector<uint>& los,
		std::vector<std::unique_ptr<StateRule>>& state_rules)
	{
		std::string src;

		if (n_src.name() != std::string("entity"))
			return;

		for (auto a : n_src.attributes())
		{
			auto name = std::string(a.name());
			if (name == "name")
				e_dst->name = a.value();
			else if (name == "visible")
				e_dst->visible = a.as_bool();
			else if (name == "driver")
			{

			}
			else if (name == "src")
				src = a.value();
		}
		if (!src.empty())
		{
			if (e_dst->src.empty())
				e_dst->src = src;
			auto path = std::filesystem::path(src);
			if (path.extension().empty())
				path.replace_extension(L".prefab");
			e_dst->load(path);
		}

		for (auto n_c : n_src.children())
		{
			auto name = std::string(n_c.name());
			auto attach_address = std::string(n_c.attribute("attach").value());
			auto attach = e_dst;
			if (!attach_address.empty())
			{
				attach = e_dst->find_child(attach_address);
				if (!attach)
				{
					printf("cannot find child: %s\n", attach_address.c_str());
					continue;
				}
			}
			if (name == "entity")
			{
				auto e = f_new<EntityPrivate>();
				e->path = e_dst->path;
				e->created_location.first = filename;
				auto offset = n_c.offset_debug();
				for (auto i = 0; i < los.size(); i++)
				{
					if (offset < los[i])
					{
						e->created_location.second = i + 1;
						break;
					}
				}
				load_prefab(e, n_c, filename, los, state_rules);

				attach->add_child(e);
			}
			else
			{
				name = "flame::" + name;
				auto udt = find_udt(name.c_str());
				if (udt)
				{
					auto c = attach->get_component(std::hash<std::string>()(name));
					auto isnew = false;
					if (!c)
					{
						auto fc = udt->find_function("create");
						if (fc && fc->get_type()->get_tag() == TypePointer && fc->get_parameters_count() == 0)
						{
							fc->call(nullptr, &c, {});
							isnew = true;
						}
						else
							printf("cannot create component of type: %s\n", name.c_str());
					}
					if (c)
					{
						for (auto a : n_c.attributes())
						{
							auto fs = udt->find_function((std::string("set_") + a.name()).c_str());
							if (fs && fs->get_parameters_count() == 1)
							{
								auto type = fs->get_parameter(0);
								auto value = std::string(a.value());
								auto sp = SUS::split(value, ';');
								if (sp.size() == 1)
								{
									void* d = type->create();
									type->unserialize(d, value.c_str());
									void* parms[] = { d };
									fs->call(c, nullptr, parms);
									type->destroy(d);
								}
								else
								{
									auto ei = TypeInfo::get(TypeEnumMulti, "flame::StateFlags");
									fassert(ei);
									auto rule = new StateRule;
									rule->c = c;
									rule->type = type;
									rule->setter = fs;
									for (auto& s : sp)
									{
										auto pair = SUS::split(s, ':');
										auto state = StateNone;
										void* d = type->create();
										if (pair.size() == 1)
										{
											type->unserialize(d, pair[0].c_str());
											void* parms[] = { d };
											fs->call(c, nullptr, parms);
										}
										else if (pair.size() == 2)
										{
											ei->unserialize(&state, pair[0].c_str());
											type->unserialize(d, pair[1].c_str());
										}
										else
											fassert(0);
										rule->values.emplace_back(state, d);
									}
									state_rules.emplace_back(rule);
								}
							}
							else
								printf("cannot find setter: %s\n", a.name());
						}
						if (isnew)
							attach->add_component((Component*)c);
					}
					else
						printf("cannot create component: %s\n", name.c_str());
				}
				else
					printf("cannot find udt: %s\n", name.c_str());
			}
		}

		if (e_dst->driver)
			e_dst->driver->on_load_finished();
	}

	static std::filesystem::path get_prefab_path(const std::filesystem::path& filename)
	{
		auto fn = filename;

		if (fn.extension().empty())
			fn.replace_extension(L".prefab");
		if (!std::filesystem::exists(fn))
		{
			fn = L"assets" / fn;
			if (!std::filesystem::exists(fn))
			{
				auto engine_path = getenv("FLAME_PATH");
				if (engine_path)
					fn = engine_path / fn;
			}
		}

		return fn;
	}

	void EntityPrivate::load(const std::filesystem::path& filename)
	{
		pugi::xml_document doc;
		pugi::xml_node doc_root;

		auto fn = get_prefab_path(filename);
		if (!doc.load_file(fn.c_str()) || (doc_root = doc.first_child()).name() != std::string("prefab"))
		{
			printf("prefab do not exist or wrong format: %s\n", filename.string().c_str());
			return;
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
		path = fn;
		load_prefab(this, doc_root.first_child(), fn, los, state_rules);
	}

	static void save_prefab(pugi::xml_node n, EntityPrivate* src)
	{
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

	void Entity::set_debug(bool v)
	{
		debug = v;
	}
}
