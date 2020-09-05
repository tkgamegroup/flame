#include <flame/foundation/typeinfo.h>
#include <flame/script/script.h>
#include <flame/universe/component.h>
#include "entity_private.h"
#include "world_private.h"

namespace flame
{
	EntityPrivate::EntityPrivate()
	{
		created_frame = looper().get_frame();

#ifdef _DEBUG
		//_created_stack_ = get_stack_frames(); TODO
#endif
	}

	EntityPrivate::~EntityPrivate()
	{
		for (auto& e : children)
		{
			for (auto c : e->local_message_dispatch_list)
				c->on_local_message(MessageRemoved);
			for (auto& l : e->local_message_listeners)
				l->call(MessageRemoved, nullptr);
		}
		for (auto c : local_message_dispatch_list)
			c->on_local_message(MessageDestroyed);
		for (auto& l : local_message_listeners)
			l->call(MessageDestroyed, nullptr);

		for (auto ev : events)
			looper().remove_event(ev);

		for (auto& s : events_s)
		{
			looper().remove_event(s.second);
			script::Instance::get()->release_slot(s.first);
		}

		for (auto s : local_data_changed_listeners_s)
			script::Instance::get()->release_slot(s);

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
			for (auto c : local_message_dispatch_list)
				c->on_local_message(MessageVisibilityChanged);
			for (auto& l : local_message_listeners)
				l->call(MessageVisibilityChanged, nullptr);
			if (parent)
			{
				for (auto c : parent->child_message_dispatch_list)
					c->on_child_message(MessageVisibilityChanged, this);
				for (auto& l : parent->child_message_listeners)
					l->call(MessageVisibilityChanged, nullptr);
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
		for (auto c : local_message_dispatch_list)
			c->on_local_message(MessageStateChanged);
		for (auto& l : local_message_listeners)
			l->call(MessageStateChanged, nullptr);
	}

	void EntityPrivate::on_message(Message msg, void* p)
	{
		for (auto c : local_message_dispatch_list)
			c->on_local_message(msg, p);
		for (auto& l : local_message_listeners)
			l->call(msg, p);
	}

	Component* EntityPrivate::get_component(uint64 hash) const
	{
		auto it = components.find(hash);
		if (it != components.end())
			return it->second.get();
		return nullptr;
	}

	Component* EntityPrivate::get_component_n(const char* _name) const
	{
		auto name = std::string(_name);
		for (auto& c : components)
		{
			if (c.second->type_name == name)
				return c.second.get();
		}
		name = "flame::" + name;
		for (auto& c : components)
		{
			if (c.second->type_name == name)
				return c.second.get();
		}
		return nullptr;
	}

	void EntityPrivate::Ref::gain(Component* c)
	{
		if (staging == INVALID_POINTER)
			return;
		*dst = staging == INVALID_POINTER ? nullptr : staging;
		staging = INVALID_POINTER;
		if (*dst)
		{
			if (on_gain)
				on_gain(c);

			if (type == RefComponent)
				((ComponentAux*)((Component*)*dst)->aux)->list_ref_by.push_back(c);
		}
	}

	void EntityPrivate::Ref::lost(Component* c)
	{
		if (on_lost)
			on_lost(c);
		*dst = nullptr;
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
		assert(!c->entity);
		assert(components.find(c->type_hash) == components.end());

		ComponentAux aux;

		auto udt = find_underlay_udt(c->type_name);
		if (udt)
		{
			auto vc = udt->get_variables_count();
			for (auto i = 0; i < vc; i++)
			{
				auto v = udt->get_variable(i);
				auto m = v->get_meta();
				if (m->get_token("ref"))
				{
					Ref r;
					r.name = SUS::cut_tail_if(v->get_type()->get_name(), "Private");
					r.hash = std::hash<std::string>()(r.name);
					r.dst = (void**)((char*)c + v->get_offset());
					auto var_name = std::string(v->get_name());
					{
						auto f = udt->find_function(("on_gain_" + var_name).c_str());
						if (f)
							r.on_gain = a2f<void(*)(void*)>(f->get_address());
					}
					{
						auto f = udt->find_function(("on_lost_" + var_name).c_str());
						if (f)
							r.on_lost = a2f<void(*)(void*)>(f->get_address());
					}

					std::string place_str;
					m->get_token("place", &place_str);

					r.optional = m->get_token("optional");

					switch (r.name[(int)r.name.find_last_of(':') + 1])
					{
					case 'c':
					{
						r.type = RefComponent;

						if (place_str.empty() || place_str == "local")
						{
							r.place = PlaceLocal;
							r.staging = get_component(r.hash);
							if (!r.staging)
							{
								if (!r.optional)
								{
									printf("add component %s failed, this component requires local component %s, which do not exist\n", c->type_name, r.name.c_str());
									return;
								}
							}
						}
						else if (place_str == "parent")
						{
							r.place = PlaceParent;
							if (parent)
							{
								r.staging = parent->get_component(r.hash);
								if (!r.staging)
								{
									if (!r.optional)
									{
										printf("add component %s failed, this component requires parent's component %s, which do not exist\n", c->type_name, r.name.c_str());
										return;
									}
								}
							}
						}
						else if (place_str == "ancestor")
						{
							r.place = PlaceAncestor;
							if (parent)
							{
								r.staging = nullptr;
								auto e = parent;
								while (e)
								{
									r.staging = e->get_component(r.hash);
									if (r.staging)
										break;
									e = e->parent;
								}
								if (!r.staging)
								{
									if (!r.optional)
									{
										printf("add component %s failed, this component requires ancestor's component %s, which do not exist\n", c->type_name, r.name.c_str());
										return;
									}
								}
							}
						}
						else
						{
							r.place = PlaceOffspring;
							auto e = find_child(place_str);
							if (e)
								r.staging = e->get_component(r.hash);
							if (!r.staging)
							{
								if (!r.optional)
								{
									printf("add component %s failed, this component requires offspring(%s)'s component %s, which do not exist\n", c->type_name, place_str.c_str(), r.name.c_str());
									return;
								}
							}
						}

						aux.refs.push_back(r);
					}
						break;
					case 's':
						assert(place_str.empty());

						r.type = RefSystem;

						if (world)
						{
							r.staging = world->get_system(r.hash);
							if (!r.staging)
							{
								if (!r.optional)
								{
									printf("add component %s failed, this component requires system %s, which do not exist\n", c->type_name, r.name.c_str());
									return;
								}
							}
						}

						aux.refs.push_back(r);
						break;
					default:
						assert(place_str.empty());

						r.type = RefObject;

						if (world)
						{
							r.staging = world->find_object(r.name);
							if (!r.staging)
							{
								if (!r.optional)
								{
									printf("add component %s failed, this component requires object %s, which do not exist\n", c->type_name, r.name.c_str());
									return;
								}
							}
						}

						aux.refs.push_back(r);
					}
				}
			}
		}

		for (auto& r : aux.refs)
		{
			if (r.staging != INVALID_POINTER)
				*r.dst = r.staging;
		}
		if (!c->check_refs())
		{
			printf("add component %s failed, check refs failed\n", c->type_name);
			return;
		}

		c->entity = this;

		c->aux = new ComponentAux;
		*(ComponentAux*)c->aux = aux;

		if (udt)
		{
			if (udt->find_function("on_local_message"))
				aux.want_local_message = true;
			if (udt->find_function("on_child_message"))
				aux.want_child_message = true;
			if (udt->find_function("on_local_data_changed"))
				aux.want_local_data_changed = true;
			if (udt->find_function("on_child_data_changed"))
				aux.want_child_data_changed = true;
		}

		for (auto& r : ((ComponentAux*)c->aux)->refs)
			r.gain(c);

		c->on_added();

		for (auto cc : local_message_dispatch_list)
			cc->on_local_message(MessageComponentAdded, c);
		for (auto& l : local_message_listeners)
			l->call(MessageComponentAdded, c);
		if (parent)
		{
			for (auto cc : parent->child_message_dispatch_list)
				cc->on_child_message(MessageComponentAdded, c);
			for (auto& l : parent->child_message_listeners)
				l->call(MessageComponentAdded, c);
		}

		components.emplace(c->type_hash, c);

		if (aux.want_local_message)
			local_message_dispatch_list.push_back(c);
		if (aux.want_child_message)
			child_message_dispatch_list.push_back(c);
		if (aux.want_local_data_changed)
			local_data_changed_dispatch_list.push_back(c);
		if (aux.want_child_data_changed)
			child_data_changed_dispatch_list.push_back(c);
	}

	void EntityPrivate::on_component_removed(Component* c)
	{
		auto& aux = *(ComponentAux*)c->aux;

		for (auto& r : aux.refs)
		{
			if (*r.dst)
			{
				if (r.type == RefComponent)
				{
					std::erase_if(((ComponentAux*)((Component*)*r.dst)->aux)->list_ref_by, [&](const auto& i) {
						return i == c;
					});
				}
				r.lost(c);
			}
		}

		delete &aux;
		c->aux = nullptr;

		for (auto cc : local_message_dispatch_list)
			cc->on_local_message(MessageComponentRemoved, c);
		for (auto& l : local_message_listeners)
			l->call(MessageComponentRemoved, c);
		if (parent)
		{
			for (auto cc : parent->child_message_dispatch_list)
				cc->on_child_message(MessageComponentRemoved, c);
			for (auto& l : parent->child_message_listeners)
				l->call(MessageComponentRemoved, c);
		}
	}

	bool EntityPrivate::check_component_removable(Component* c) const
	{
		auto& aux = *(ComponentAux*)c->aux;

		if (!aux.list_ref_by.empty())
		{
			printf("remove component %s failed, this component is referenced by %d component(s)\n", c->type_name, aux.list_ref_by.size());
			return false;
		}

		return true;
	}

	void EntityPrivate::remove_component(Component* c, bool destroy)
	{
		auto it = components.find(c->type_hash);
		if (it == components.end())
		{
			assert(0);
			return;
		}

		if (!check_component_removable(c))
			return;

		auto& aux = *(ComponentAux*)c->aux;

		on_component_removed(c);

		if (aux.want_local_message)
		{
			std::erase_if(local_message_dispatch_list, [&](const auto& i) {
				return i == c;
			});
		}
		if (aux.want_child_message)
		{
			std::erase_if(child_message_dispatch_list, [&](const auto& i) {
				return i == c;
			});
		}
		if (aux.want_local_data_changed)
		{
			std::erase_if(local_data_changed_dispatch_list, [&](const auto& i) {
				return i == c;
			});
		}
		if (aux.want_child_data_changed)
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
		{
			if (!check_component_removable(c.second.get()))
				return;
		}

		for (auto& c : components)
			on_component_removed(c.second.get());

		local_message_dispatch_list.clear();
		child_message_dispatch_list.clear();
		local_data_changed_dispatch_list.clear();
		child_data_changed_dispatch_list.clear();

		if (!destroy)
		{
			for (auto& c : components)
				c.second.release();
		}
		components.clear();
	}

	void EntityPrivate::add_child(EntityPrivate* e, int position)
	{
		assert(e && e != this && !e->parent);

		auto ok = true;
		e->traversal([this, &ok](EntityPrivate* e) {
			for (auto& c : e->components)
			{
				auto& aux = *(ComponentAux*)c.second->aux;
				for (auto& r : aux.refs)
				{
					switch (r.type)
					{
					case RefComponent:
						if (!*r.dst)
						{
							switch (r.place)
							{
							case PlaceParent:
								r.staging = get_component(r.hash);
								if (!r.staging)
								{
									if (!r.optional)
									{
										printf("add child failed, this child contains a component %s that requires parent's component %s, which do not exist\n", c.second->type_name, r.name.c_str());
										ok = false;
										return false;
									}
								}
								break;
							case PlaceAncestor:
								r.staging = nullptr;
								auto e = this;
								while (e)
								{
									r.staging = e->get_component(r.hash);
									if (r.staging)
										break;
									e = e->parent;
								}
								if (!r.staging && !r.optional)
								{
									printf("add child failed, this child contains a component %s that requires ancestor's component %s, which do not exist\n", c.second->type_name, r.name.c_str());
									ok = false;
									return false;
								}
								break;
							}
						}
						break;
					case RefSystem:
						if (world)
						{
							r.staging = world->get_system(r.hash);
							if (!r.staging)
							{
								if (!r.optional)
								{
									printf("add child failed, this child contains a component %s that requires system %s, which do not exist\n", c.second->type_name, r.name.c_str());
									ok = false;
									return false;
								}
							}
						}
						break;
					case RefObject:
						if (world)
						{
							r.staging = world->find_object(r.name);
							if (!r.staging)
							{
								if (!r.optional)
								{
									printf("add child failed, this child contains a component %s that requires object %s, which do not exist\n", c.second->type_name, r.name.c_str());
									ok = false;
									return false;
								}
							}
						}
						break;
					}
				}

				for (auto& r : aux.refs)
				{
					if (r.staging != INVALID_POINTER)
						*r.dst = r.staging;
				}
				if (!c.second->check_refs())
				{
					printf("add child failded, this child contains a component %s that check refs failed\n", c.second->type_name);
					return false;
				}
			}
			return true;
		});

		if (!ok)
			return;

		e->traversal([this](EntityPrivate* e) {
			for (auto& c : e->components)
			{
				auto& aux = *(ComponentAux*)c.second->aux;
				for (auto& r : aux.refs)
					r.gain(c.second.get());

				e->world = world;
				if (world)
				{
					c.second->on_local_message(MessageEnteredWorld);
					for (auto& l : local_message_listeners)
						l->call(MessageEnteredWorld, nullptr);
				}
			}
			return true;
		});

		if (position == -1)
			position = children.size();

		children.emplace(children.begin() + position, e);

		e->parent = this;
		e->depth = depth + 1;
		e->index = position;
		e->update_visibility();

		for (auto c : e->local_message_dispatch_list)
			c->on_local_message(MessageAdded);
		for (auto& l : local_message_listeners)
			l->call(MessageAdded, nullptr);
		for (auto c : child_message_dispatch_list)
			c->on_child_message(MessageAdded, e);
		for (auto& l : child_message_listeners)
			l->call(MessageAdded, e);
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

		for (auto c : a->local_message_dispatch_list)
			c->on_local_message(MessagePositionChanged);
		for (auto& l : a->local_message_listeners)
			l->call(MessagePositionChanged, nullptr);
		for (auto c : b->local_message_dispatch_list)
			c->on_local_message(MessagePositionChanged);
		for (auto& l : b->local_message_listeners)
			l->call(MessagePositionChanged, nullptr);

		for (auto c : child_message_dispatch_list)
			c->on_child_message(MessagePositionChanged, a);
		for (auto& l : child_message_listeners)
			l->call(MessagePositionChanged, a);
		for (auto c : child_message_dispatch_list)
			c->on_child_message(MessagePositionChanged, b);
		for (auto& l : child_message_listeners)
			l->call(MessagePositionChanged, b);
	}

	void EntityPrivate::on_child_removed(EntityPrivate* e) const
	{
		e->parent = nullptr;

		e->traversal([](EntityPrivate* e) {
			for (auto& c : e->components)
			{
				auto& aux = *(ComponentAux*)c.second->aux;
				for (auto& r : aux.refs)
				{
					if (!r.dst)
					{
						if (r.type == RefComponent)
						{
							if (r.place != PlaceLocal)
							{
								std::erase_if(((ComponentAux*)((Component*)*r.dst)->aux)->list_ref_by, [&](const auto& i) {
									return i == c.second.get();
								});
								r.lost(c.second.get());
							}
						}
						else
							r.lost(c.second.get());
					}
				}
			}

			for (auto c : e->local_message_dispatch_list)
				c->on_local_message(MessageLeftWorld);
			for (auto& l : e->local_message_listeners)
				l->call(MessageLeftWorld, nullptr);
			e->world = nullptr;

			return true;
		});

		for (auto c : e->local_message_dispatch_list)
			c->on_local_message(MessageRemoved);
		for (auto& l : e->local_message_listeners)
			l->call(MessageRemoved, nullptr);
		for (auto c : child_message_dispatch_list)
			c->on_child_message(MessageRemoved, e);
		for (auto& l : child_message_listeners)
			l->call(MessageRemoved, e);
	}

	static bool can_remove(EntityPrivate* e)
	{
		for (auto& c : e->components)
		{
			for (auto& r : ((EntityPrivate::ComponentAux*)c.second->aux)->list_ref_by)
			{
				if (((EntityPrivate*)r->entity)->depth < e->depth)
					return false;
			}
		}
		return true;
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

		if (!can_remove(e))
			return;

		on_child_removed(e);

		if (!destroy)
			it->release();
		children.erase(it);
	}

	void EntityPrivate::remove_all_children(bool destroy)
	{
		for (auto& c : children)
		{
			if (!can_remove(c.get()))
				return;
		}

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

	void* EntityPrivate::add_local_message_listener(void (*callback)(Capture& c, Message msg, void* p), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		local_message_listeners.emplace_back(c);
		return c;
	}

	void EntityPrivate::remove_local_message_listener(void* lis)
	{
		std::erase_if(local_message_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* EntityPrivate::add_child_message_listener(void (*callback)(Capture& c, Message msg, void* p), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		child_message_listeners.emplace_back(c);
		return c;
	}

	void EntityPrivate::remove_child_message_listener(void* lis)
	{
		std::erase_if(child_message_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* EntityPrivate::add_local_data_changed_listener(void (*callback)(Capture& c, Component* t, uint64 hash), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		local_data_changed_listeners.emplace_back(c);
		return c;
	}

	void EntityPrivate::remove_local_data_changed_listener(void* lis)
	{
		std::erase_if(local_data_changed_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
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

	void EntityPrivate::add_local_data_changed_listener_s(uint slot)
	{
		local_data_changed_listeners_s.push_back(slot);
	}

	void EntityPrivate::remove_local_data_changed_listener_s(uint slot)
	{
		for (auto it = local_data_changed_listeners_s.begin(); it != local_data_changed_listeners_s.end(); it++)
		{
			if (*it == slot)
			{
				local_data_changed_listeners_s.erase(it);
				script::Instance::get()->release_slot(slot);
			}
		}
	}

	void EntityPrivate::add_event_s(uint slot)
	{
		auto ev = looper().add_event([](Capture& c) {
			script::Instance::get()->call_slot(c.data<uint>(), 0, nullptr);
			c._current = INVALID_POINTER;
		}, Capture().set_data(&slot));
		events_s.emplace_back(slot, ev);
	}

	void EntityPrivate::remove_event_s(uint slot)
	{
		for (auto it = events_s.begin(); it != events_s.end(); it++)
		{
			if (it->first == slot)
			{
				looper().remove_event(it->second);
				events_s.erase(it);
				script::Instance::get()->release_slot(slot);
			}
		}
	}

	static void load_prefab(EntityPrivate* dst, pugi::xml_node src, 
		const std::vector<std::pair<std::string, std::string>>& nss)
	{
		if (src.name() != std::string("entity"))
			return;

		for (auto a : src.attributes())
		{
			auto name = std::string(a.name());
			if (name == "name")
			{
				dst->name = a.value();
				if (dst->name.size() > 1 && dst->name[0] == '@')
					dst->name.erase(dst->name.begin());
			}
			else if (name == "visible")
				dst->visible = a.as_bool();
			else if (name == "src")
			{
				auto path = std::filesystem::path(a.value());
				dst->src = path;
				path.replace_extension(L".prefab");
				dst->load(path);
			}
		}

		auto find_ns = [&](const std::string& n)->std::string {
			for (auto& ns : nss)
			{
				if (ns.first == n)
					return ns.second;
			}
			return "";
		};

		for (auto n_c : src.children())
		{
			auto name = std::string(n_c.name());
			auto attach_address = std::string(n_c.attribute("attach").value());
			auto attach = dst;
			if (!attach_address.empty())
			{
				attach = dst->find_child(attach_address);
				if (!attach)
				{
					printf("cannot find child: %s\n", attach_address.c_str());
					continue;
				}
			}
			if (name == "entity")
			{
				auto e = f_new<EntityPrivate>();
				load_prefab(e, n_c, nss);

				attach->add_child(e);
			}
			else if (name == "debug_break")
				debug_break();
			else
			{
				auto sp = SUS::split(name, ':');
				if (sp.size() == 2)
					name = find_ns(sp[0]) + "::" + sp[1];
				else
					name = "flame::" + name;
				auto udt = find_udt(name.c_str());
				if (udt)
				{
					auto c = attach->get_component(std::hash<std::string>()(name));
					auto isnew = false;
					if (!c)
					{
						auto fc = udt->find_function("create");
						if (fc->get_type()->get_tag() == TypePointer && fc->get_parameters_count() == 0)
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
								void* d = type->create();
								type->unserialize(d, a.value());
								void* parms[] = { d };
								fs->call(c, nullptr, parms);
								type->destroy(d);
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
	}

	std::filesystem::path get_prefab_path(const std::filesystem::path& filename)
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

		if (!doc.load_file(get_prefab_path(filename).c_str()) || (doc_root = doc.first_child()).name() != std::string("prefab"))
		{
			printf("prefab not exist or wrong format: %s\n", filename.string().c_str());
			return;
		}

		std::vector<std::pair<std::string, std::string>> nss;
		for (auto a : doc_root.attributes())
		{
			static std::regex reg_ns(R"(xmlns:(\w+))");
			std::smatch res;
			auto name = std::string(a.name());
			if (std::regex_search(name, res, reg_ns))
				nss.emplace_back(res[1].str(), a.value());
		}
		load_prefab(this, doc_root.first_child(), nss);
	}

	static std::unordered_map<uint64, std::unique_ptr<Component, Delector>> staging_components;

	static void save_prefab(pugi::xml_node n, EntityPrivate* src)
	{
		pugi::xml_document reference_doc;
		pugi::xml_node reference_doc_root;
		pugi::xml_node reference;

		if (!src->src.empty())
		{
			if (!reference_doc.load_file(get_prefab_path(src->src).c_str()) || (reference_doc_root = reference_doc.first_child()).name() != std::string("prefab"))
			{
				printf("prefab not exist or wrong format: %s\n", src->src.string().c_str());
				return;
			}

			reference = reference_doc_root.first_child();

			n.append_attribute("src").set_value(src->src.string().c_str());
		}

		if (src->name != reference.attribute("name").value())
			n.append_attribute("name").set_value(src->name.c_str());
		if (src->visible != reference.attribute("visible").as_bool(true))
			n.append_attribute("visible").set_value(src->visible);

		std::vector<Component*> before_children_components;
		std::vector<Component*> after_children_components;
		for (auto& c : src->components)
		{
			auto after = false;
			for (auto& r : ((EntityPrivate::ComponentAux*)c.second->aux)->refs)
			{
				if (r.type == EntityPrivate::RefComponent && r.place == EntityPrivate::PlaceOffspring)
				{
					after = true;
					break;
				}
			}
			if (after)
				after_children_components.push_back(c.second.get());
			else
				before_children_components.push_back(c.second.get());
		}
		auto sort_components = [](std::vector<Component*>& list) {
			for (auto i = 0; i < (int)list.size() - 1; i++)
			{
				for (auto j = i; j < list.size(); j++)
				{
					auto ok = true;
					for (auto& r : ((EntityPrivate::ComponentAux*)list[j]->aux)->refs)
					{
						if (r.type == EntityPrivate::RefComponent && r.place == EntityPrivate::PlaceLocal)
						{
							auto found = false;
							for (auto k = 0; k < i; k++)
							{
								if (list[k] == *r.dst)
								{
									found = true;
									break;
								}
							}
							if (!found)
							{
								ok = false;
								break;
							}
						}
					}
					if (ok)
					{
						std::swap(list[i], list[j]);
						break;
					}
				}
			}
		};
		sort_components(before_children_components);
		sort_components(after_children_components);

		auto save_component = [&](Component* c) {
			auto c_type = std::string(c->type_name);
			auto c_type_nns = c_type.substr((int)c_type.find_last_of(':') + 1);
			auto reference_c = reference.child(c_type_nns.c_str());

			pugi::xml_node n_c;
			if (!reference || !reference_c)
				n_c = n.append_child(c_type_nns.c_str());

			auto udt = find_udt(c_type.c_str());
			if (udt)
			{
				auto functions_count = udt->get_functions_count();
				for (auto i = 0; i < functions_count; i++)
				{
					auto fg = udt->get_function(i);
					auto name = std::string(fg->get_name());
					if (name.starts_with("get_"))
					{
						name = name.substr(4);
						if (udt->find_function(("set_" + name).c_str()))
						{
							auto type = fg->get_type();
							if (type->get_tag() != TypePointer ||
								type->get_name() == std::string("char") ||
								type->get_name() == std::string("wchar_t"))
							{
								auto d1 = type->create();
								fg->call(c, d1, {});

								auto save_attr = [&]() {
									std::string str;
									type->serialize(&str, d1);
								};

								auto same = false;
								auto d2 = type->create();
								auto a = reference_c.attribute(name.c_str());
								if (a)
								{
									type->unserialize(d2, a.value());
									same = type->compare(d1, d2);
								}
								else
								{
									Component* cc = nullptr;
									auto it = staging_components.find(std::hash<std::string>()(c_type));
									if (it != staging_components.end())
										cc = it->second.get();
									else
									{
										auto fc = udt->find_function("create");
										if (fc->get_type()->get_tag() == TypePointer && fc->get_parameters_count() == 0)
										{
											fc->call(nullptr, &cc, {});
											staging_components.emplace(c->type_hash, cc);
										}
										else
											printf("cannot create component of type: %s\n", name.c_str());
									}
									if (cc)
									{
										fg->call(cc, d2, {});
										same = type->compare(d1, d2);
									}
								}
								if (type->get_tag() == TypePointer)
									*(void**)d2 = nullptr;
								type->destroy(d2);
								if (!same)
								{
									std::string str;
									type->serialize(&str, d1);
									if (!n_c)
										n_c = n.append_child(c_type_nns.c_str());
									n_c.append_attribute(name.c_str()).set_value(str.c_str());
								}
								if (type->get_tag() == TypePointer)
									*(void**)d1 = nullptr;
								type->destroy(d1);
							}
						}
					}
				}
			}
			else
				printf("cannot find udt: %s\n", c_type.c_str());
		};

		for (auto c : before_children_components)
			save_component(c);

		for (auto& c : src->children)
		{
			auto found = false;
			for (auto nn : reference.children("entity"))
			{
				auto name = std::string(nn.attribute("name").value());
				if (name == c->name || name == "@" + c->name)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				auto nn = n.append_child("entity");
				save_prefab(nn, c.get());
			}
		}

		std::vector<EntityPrivate*> attach_points;
		std::function<void(pugi::xml_node)> collect_attach_points;
		collect_attach_points = [&](pugi::xml_node n) {
			for (auto c : n.children())
			{
				if (c.name() == std::string("entity"))
				{
					auto name = std::string(c.attribute("name").value());
					if (name.size() > 1 && name[0] == '@') 
					{
						name.erase(name.begin());
						attach_points.push_back(src->find_child(name));
					}
					collect_attach_points(c);
				}
			}
		};
		collect_attach_points(reference);

		for (auto p : attach_points)
		{
			for (auto& c : p->children)
			{
				auto nn = n.append_child("entity");
				nn.append_attribute("attach").set_value(p->name.c_str());
				save_prefab(nn, c.get());
			}
		}

		for (auto c : after_children_components)
			save_component(c);
	}

	void EntityPrivate::save(const std::filesystem::path& filename)
	{
		pugi::xml_document file;
		auto file_root = file.append_child("prefab");

		save_prefab(file_root, this);

		staging_components.clear();

		file.save_file(filename.c_str());
	}

	void Entity::report_data_changed(Component* c, uint64 hash)
	{
		auto entity = (EntityPrivate*)c->entity;
		if (!entity)
			return;
		for (auto cc : entity->local_data_changed_dispatch_list)
		{
			if (cc != c)
				cc->on_local_data_changed(c, hash);
		}
		for (auto& l : entity->local_data_changed_listeners)
			l->call(c, hash);
		for (auto s : entity->local_data_changed_listeners_s)
		{
			script::Parameter ps[2];
			ps[0].type = script::ScriptTypePointer;
			ps[0].data = &c;
			ps[1].type = script::ScriptTypePointer;
			ps[1].data = &hash;
			script::Instance::get()->call_slot(s, size(ps), ps);
		}
		if (entity->parent)
		{
			for (auto cc : entity->parent->child_data_changed_dispatch_list)
				cc->on_child_data_changed(c, hash);
		}
	}

	Entity* Entity::create()
	{
		return f_new<EntityPrivate>();
	}
}
