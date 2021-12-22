#include "../xml.h"
#include "../foundation/typeinfo.h"
#include "entity_private.h"
#include "world_private.h"

namespace flame
{
	struct ComponentCache
	{
		struct Attribute
		{
			VariableInfo* vi = nullptr;
			TypeInfo* get_type = nullptr;
			TypeInfo* set_type = nullptr;
			FunctionInfo* getter = nullptr;
			FunctionInfo* setter = nullptr;

			//Attribute(Type* ct, const std::string& name, TypeInfo* get_type, TypeInfo* set_type, FunctionInfo* getter, FunctionInfo* setter) :
			//	name(name),
			//	get_type(get_type),
			//	set_type(set_type),
			//	getter(getter),
			//	setter(setter)
			//{
			//	hash = ch(name.c_str());

			//	//if (getter)
			//	//{
			//	//	auto d = get_type->create(false);
			//	//	getter->call(ct->dummy, default_value, nullptr);
			//	//	get_type->destroy(d, false);
			//	//}
			//}

			std::string serialize(void* c, void* ref = nullptr)
			{
				//auto same = false;
				//void* d = get_type->create(false);
				//getter->call(c, d, nullptr);
				//if (ref)
				//{
				//	void* dd = get_type->create(false);
				//	getter->call(ref, dd, nullptr);
				//	if (get_type->compare(d, dd))
				//		same = true;
				//	get_type->destroy(dd, false);
				//}
				//else if (get_type->compare(d, default_value))
				//	same = true;
				std::string str;
				//if (!same)
				//	str = get_type->serialize(d);
				//get_type->destroy(d, false);
				return str;
			}
		};
	};

	//struct Type
	//{

	//	UdtInfo* udt = nullptr;
	//	FunctionInfo* creator = nullptr;
	//	void* dummy = nullptr;

	//	std::map<std::string, Attribute> attributes;

	//	Type(UdtInfo* udt) :
	//		udt(udt)
	//	{
	//		auto fc = udt->find_function("create");
	//		assert(fc && fc->check(TypeInfo::get(TagPointer, udt->name, tidb), { TypeInfo::get(TagPointer, "void", tidb) }));
	//		creator = fc;

	//		dummy = create();
	//		if (dummy)
	//		{
	//			std::vector<std::tuple<std::string, TypeInfo*, FunctionInfo*>> getters;
	//			std::vector<std::tuple<std::string, TypeInfo*, FunctionInfo*>> setters;
	//			for (auto& f : udt->functions)
	//			{
	//				if (f.name.compare(0, 4, "get_") == 0 && f.parameters.empty())
	//				{
	//					if (f.type->name != "void")
	//						getters.emplace_back(f.name.substr(4), f.type, f);
	//				}
	//				else if (f.name.compare(0, 4, "set_") == 0 && f.parameters.size() == 1 && f.type == TypeInfo::get(TagD, "", tidb))
	//				{
	//					auto t = f.parameters[0];
	//					if (t->name != "void")
	//						setters.emplace_back(f.name.substr(4), t, f);
	//				}
	//			}
	//			for (auto& g : getters)
	//			{
	//				for (auto& s : setters)
	//				{
	//					if (std::get<0>(g) == std::get<0>(s) && std::get<1>(g)->name == std::get<1>(s)->name)
	//					{
	//						Attribute a(this, std::get<0>(g), std::get<1>(g), std::get<1>(s), std::get<2>(g), std::get<2>(s));
	//						attributes.emplace(a.name, std::move(a));
	//						break;
	//					}
	//				}
	//			}
	//		}
	//	}

	//	void* create()
	//	{
	//		auto addr = creator->get_address();
	//		if (!addr)
	//			return nullptr;
	//		return a2f<void*(*)(void*)>(addr)(nullptr);
	//	}

	//	Attribute* find_attribute(const std::string& name)
	//	{
	//		auto it = attributes.find(name);
	//		if (it == attributes.end())
	//			return nullptr;
	//		return &it->second;
	//	}
	//};

	//static Type* find_component_type(const std::string& name)
	//{
	//	auto it = component_types.find(name);
	//	if (it == component_types.end())
	//		return nullptr;
	//	return &it->second;
	//}

	//static std::map<std::string, Type> component_types;

	static std::map<std::string, std::filesystem::path> name_to_prefab_path;

	struct _Initializer
	{
		_Initializer()
		{
			//for (auto& ui : tidb.udts)
			//{
			//	static auto reg_com = std::regex("^(flame::c\\w+)$");
			//	std::smatch res;
			//	if (std::regex_search(ui.second.name, res, reg_com))
			//	{
			//		Type t(&ui.second);
			//		if (t.dummy)
			//			component_types.emplace(res[1].str(), std::move(t));
			//	}
			//}

			//auto engine_path = getenv("FLAME_PATH");
			//if (engine_path)
			//{
			//	auto path = std::filesystem::path(engine_path) / L"default_assets/prefabs";
			//	if (std::filesystem::exists(path))
			//	{
			//		for (auto& it : std::filesystem::directory_iterator(path))
			//		{
			//			if (it.path().extension() == L".prefab")
			//			{
			//				auto name = it.path().stem().string();
			//				name[0] = std::toupper(name[0]);
			//				for (auto i = 0; i < name.size(); i++)
			//				{
			//					if (name[i] == '_')
			//						name[i + 1] = std::toupper(name[i + 1]);
			//				}
			//				SUS::remove_char(name, '_');
			//				name = 'e' + name;
			//				name_to_prefab_path[name] = it.path();
			//			}
			//		}
			//	}
			//}
		}
	};
	static _Initializer _initializer;

	EntityPrivate::EntityPrivate()
	{
		static auto id = 0;
		created_frame = frames;
		created_id = id++;
	}

	EntityPrivate::~EntityPrivate()
	{
		for (auto& l : message_listeners.list)
			l(S<"destroyed"_h>, nullptr, nullptr);
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
			for (auto& l : message_listeners.list)
				l(S<"visibility_changed"_h>, global_visibility ? (void*)1 : nullptr, nullptr);
			for (auto c : component_list)
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

		for (auto& l : message_listeners.list)
			l(S<"state_changed"_h>, (void*)state, (void*)last_state);
		for (auto c : component_list)
			c->on_state_changed(state);
	}

	void EntityPrivate::add_component(Component* c)
	{
		assert(!parent);
		assert(!c->entity);
		assert(components.find(c->type_hash) == components.end());

		c->entity = this;

		for (auto& _c : component_list)
			_c->on_component_added(c);

		components.emplace(c->type_hash, c);
		component_list.push_back(c);

		c->on_added();
		if (world)
			c->on_entered_world();
	}

	void EntityPrivate::remove_component(Component* c, bool destroy)
	{
		auto it = components.find(c->type_hash);
		if (it == components.end())
		{
			assert(0);
			return;
		}

		it->second.release();
		components.erase(it);

		for (auto it = component_list.begin(); it != component_list.end(); it++)
		{
			if (*it == c)
			{
				component_list.erase(it);
				break;
			}
		}

		for (auto& _c : component_list)
			_c->on_component_removed(c);

		c->on_removed();
		if (world)
			c->on_left_world();

		if (destroy)
			delete c;
	}

	void EntityPrivate::add_child(EntityPrivate* e, int position)
	{
		assert(e && e != this && !e->parent);

		uint pos;
		if (position == -1)
			pos = children.size();
		else
			pos = position;

		children.emplace(children.begin() + pos, e);

		e->parent = this;
		e->backward_traversal([this](EntityPrivate* e) {
			e->depth = e->parent->depth + 1;
		});
		e->index = pos;
		e->update_visibility();

		for (auto& l : e->message_listeners.list)
			l(S<"entity_added"_h>, nullptr, nullptr);
		for (auto c : e->component_list)
			c->on_entity_added();

		e->backward_traversal([this](EntityPrivate* e) {
			if (!e->world && world)
				e->on_entered_world(world);
		});

		for (auto& l : message_listeners.list)
			l(S<"child_added"_h>, e, nullptr);
		for (auto c : component_list)
			c->on_child_added(e);
	}

	void EntityPrivate::on_child_removed(EntityPrivate* e) const
	{
		e->parent = nullptr;

		for (auto& l : e->message_listeners.list)
			l(S<"entity_removed"_h>, nullptr, nullptr);
		for (auto c : e->component_list)
			c->on_entity_removed();

		e->backward_traversal([](EntityPrivate* e) {
			if (e->world)
				e->on_left_world();
		});

		for (auto& l : message_listeners.list)
			l(S<"child_removed"_h>, e, nullptr);
		for (auto c : component_list)
			c->on_child_removed(e);
	}

	void EntityPrivate::remove_child(EntityPrivate* e, bool destroy)
	{
		assert(e && e != this);

		auto it = std::find_if(children.begin(), children.end(), [&](const auto& t) {
			return t.get() == e;
		});
		if (it == children.end())
		{
			assert(0);
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

	void EntityPrivate::on_entered_world(WorldPrivate* _world)
	{
		world = _world;
		for (auto& l : message_listeners.list)
			l(S<"entered_world"_h>, nullptr, nullptr);
		for (auto c : component_list)
			c->on_entered_world();
	}

	void EntityPrivate::on_left_world()
	{
		for (auto& l : message_listeners.list)
			l(S<"left_world"_h>, nullptr, nullptr);
		for (auto c : component_list)
			c->on_left_world();
		world = nullptr;
	}

	EntityPtr EntityPrivate::copy()
	{
		auto ret = new EntityPrivate();
		ret->name = name;
		ret->tag = tag;
		ret->visible = visible;
		ret->sources = sources;
		ret->state = state;
		for (auto c : component_list)
		{
		//	std::string type_name = c->type_name;
		//	SUS::cut_head_if(type_name, "flame::");
		//	auto ct = find_component_type(type_name);
		//	assert(ct);
		//	auto cc = (Component*)ct->create();
		//	cc->src_id = c->src_id;
		//	for (auto& a : ct->attributes)
		//	{
		//		auto& attr = a.second;
		//		if (!attr.getter->metas.get(MetaSecondaryAttribute, nullptr))
		//		{
		//			auto d = attr.get_type->create(false);
		//			attr.getter->call(c.get(), d, nullptr);
		//			if (attr.get_type->tag == TagD && attr.set_type->tag == TagPointer)
		//			{
		//				void* ps[] = { &d };
		//				attr.setter->call(cc, nullptr, ps);
		//			}
		//			else
		//			{
		//				void* ps[] = { d };
		//				attr.setter->call(cc, nullptr, ps);
		//			}
		//			attr.get_type->destroy(d, false);
		//		}
		//	}
		//	ret->add_component(cc);
		}
		for (auto& c : children)
			ret->add_child(c->copy());
		return ret;
	}

	static void load_prefab(EntityPrivate* e_dst, pugi::xml_node n_src)
	{
	//	auto set_attribute = [&](void* o, Type* ot, const std::string& vname, const std::string& value) {
	//		auto att = ot->find_attribute(vname);
	//		if (!att)
	//			return false;

	//		auto type = att->set_type;
	//		auto fs = att->setter;
	//		void* d = type->create();
	//		type->unserialize(value.c_str(), d);
	//		void* ps[] = { d };
	//		fs->call(o, nullptr, ps);
	//		type->destroy(d);
	//		return true;
	//	};

		auto ename = std::string(n_src.name());
		if (ename != "entity")
		{
	//		auto it = name_to_prefab_path.find(ename);
	//		if (it != name_to_prefab_path.end())
	//			e_dst->load(it->second);
	//		else
	//			printf("cannot find prefab: %s\n", ename.c_str());
		}

		for (auto a : n_src.attributes())
		{
			auto name = std::string(a.name());
			if (name == "name")
				e_dst->name = a.value();
			else if (name == "visible")
				e_dst->visible = a.as_bool();
			else if (name == "src")
			{
				std::filesystem::path fn(a.value());
	//			path.replace_extension(L".prefab");
	//			if (!path.is_absolute())
	//			{
	//				auto temp = fn.parent_path() / path;
	//				if (std::filesystem::exists(temp))
	//					path = temp;
	//			}
	//			e_dst->load(path);
			}
		}

		for (auto n_c : n_src.children())
		{
			auto name = std::string(n_c.name());
			if (name[0] == 'c')
			{
	//			Type* ct = find_component_type(name);
	//			if (ct)
	//			{
	//				auto c = e_dst->get_component(std::hash<std::string>()(ct->udt->name));
	//				auto isnew = false;
	//				if (!c)
	//				{
	//					c = (Component*)ct->create();
	//					auto& srcs = e_dst->srcs;
	//					for (auto i = (int)srcs.size() - 1; i >= 0; i--)
	//					{
	//						if (srcs[i] == fn)
	//						{
	//							c->src_id = srcs.size() - i - 1;
	//							break;
	//						}
	//					}
	//					isnew = true;
	//				}
	//				for (auto a : n_c.attributes())
	//				{
	//					if (!set_attribute(c, ct, a.name(), a.value()))
	//						printf("cannot find attribute: %s\n", a.name());
	//				}
	//				if (isnew)
	//					e_dst->add_component(c);
	//			}
	//			else
	//				printf("cannot find component: %s\n", name.c_str());
			}
			else if (name[0] == 'e')
			{
				auto e = new EntityPrivate;
				e->sources.push_back(e_dst->sources.back());
				load_prefab(e, n_c);
				e_dst->add_child(e);
			}
		}

	}

	bool EntityPrivate::load(const std::filesystem::path& filename)
	{
		pugi::xml_document doc;
		pugi::xml_node doc_root;

		if (!doc.load_file(filename.c_str()) || (doc_root = doc.first_child()).name() != std::string("prefab"))
		{
			printf("prefab do not exist or wrong format: %s\n", filename.string().c_str());
			return false;
		}

		sources.push_back(filename);
		load_prefab(this, doc_root.first_child());

		return true;
	}

	//static void save_prefab(pugi::xml_node n_dst, EntityPrivate* e_src)
	//{
	//	auto& srcs = e_src->srcs;
	//	if (srcs.empty() || srcs.back() != fn)
	//	{
	//		n_dst.parent().remove_child(n_dst);
	//		return;
	//	}

	//	EntityPrivate* reference = nullptr;
	//	if (srcs.size() >= 2)
	//	{
	//		auto src = srcs[srcs.size() - 2];

	//		for (auto& r : references)
	//		{
	//			if (r.first == src)
	//			{
	//				reference = r.second.get();
	//				break;
	//			}
	//		}
	//		if (!reference)
	//		{
	//			reference = new EntityPrivate;
	//			reference->load(src);
	//			references.emplace_back(src, reference);
	//		}

	//		auto found = false;
	//		for (auto& p : name_to_prefab_path)
	//		{
	//			if (p.second == src)
	//			{
	//				n_dst.set_name(p.first.c_str());
	//				found = true;
	//				break;
	//			}
	//		}
	//		if (!found)
	//			n_dst.append_attribute("src").set_value(src.string().c_str());
	//	}

	//	if (!e_src->name.empty())
	//		n_dst.append_attribute("name").set_value(e_src->name.c_str());
	//	if (!e_src->visible)
	//		n_dst.append_attribute("visible").set_value("false");

	//	for (auto& c : e_src->components)
	//	{
	//		std::string cname;
	//		auto ct = find_component_type(c->type_name, &cname);
	//		assert(ct);
	//		auto ref = reference ? reference->get_component(c->type_hash) : nullptr;
	//		auto put_attributes = [&](pugi::xml_node n) {
	//			for (auto& a : ct->attributes)
	//			{
	//				if (!a.second.getter->metas.get(MetaSecondaryAttribute, nullptr))
	//				{
	//					auto value = a.second.serialize(c.get(), ref);
	//					if (!value.empty())
	//						n.append_attribute(a.first.c_str()).set_value(value.c_str());
	//				}
	//			}
	//		};
	//		if (srcs[srcs.size() - c->src_id - 1] == fn)
	//			put_attributes(n_dst.append_child(cname.c_str()));
	//		else
	//			put_attributes(n_dst);

	//	}

	//	for (auto& c : e_src->children)
	//		save_prefab(n_dst.append_child("entity"), c.get());
	//}

	bool EntityPrivate::save(const std::filesystem::path& filename)
	{
		//pugi::xml_document file;
		//auto file_root = file.append_child("prefab");

		//save_prefab(file_root.append_child("entity"), this);

		//file.save_file(filename.c_str());

		return true;
	}

	Entity* Entity::create()
	{
		return new EntityPrivate();
	}
}
