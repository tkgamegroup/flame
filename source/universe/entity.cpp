#include "../xml.h"
#include "../foundation/typeinfo.h"
#include "../foundation/typeinfo_serialize.h"
#include "../foundation/system.h"
#include "entity_private.h"
#include "world_private.h"

namespace flame
{
	static std::map<std::string, std::filesystem::path> name_to_prefab_path;

	struct _Initializer
	{
		_Initializer()
		{

		}
	};
	static _Initializer _initializer;

	EntityPrivate::EntityPrivate()
	{
		instance_id = generate_guid();

		static auto id = 0;
		created_frame = frames;
		created_id = id++;
	}

	EntityPrivate::~EntityPrivate()
	{
		for (auto& l : message_listeners.list)
			l.first("destroyed"_h, nullptr, nullptr);

		for (auto it = components.rbegin(); it != components.rend(); it++)
		{
			auto comp = it->release();
			delete comp;
		}
	}

	void EntityPrivate::update_enable()
	{
		auto prev_enable = global_enable;
		if (parent)
			global_enable = enable && parent->global_enable;
		else
		{
			if (!world)
				global_enable = true;
			else
				global_enable = false;
		}
		if (global_enable != prev_enable)
		{
			if (world)
			{
				for (auto& c : components)
					global_enable ? c->on_active() : c->on_inactive();
				for (auto& l : message_listeners.list)
					l.first(global_enable ? "active"_h : "inactive"_h, nullptr, nullptr);
			}

			for (auto& e : children)
				e->update_enable();
		}
	}

	void EntityPrivate::set_enable(bool v)
	{
		if (enable == v)
			return;
		enable = v;
		update_enable();
	}

	Component* EntityPrivate::add_component(uint hash)
	{
		auto ui = find_udt(hash);
		if (!ui)
		{
			printf("cannot add component: cannot find udt of hash %u\n", hash);
			return nullptr;
		}

		if (component_map.find(hash) != component_map.end())
		{
			printf("cannot add component: %s already existed\n", ui->name.c_str());
			return nullptr;
		}

		std::vector<std::pair<Component*, uint>> require_comps;
		for (auto& vi : ui->variables)
		{
			if (vi.metas.get("requires"_h))
			{
				auto ok = false;
				if (vi.type->tag == TagPU)
				{
					auto comp = get_component(sh(vi.type->name.c_str()));
					if (comp)
					{
						require_comps.emplace_back(comp, vi.offset);
						ok = true;
					}
				}
				if (!ok)
				{
					printf("cannot add component: %s requires %s, which doesn't exist\n", ui->name.c_str(), vi.type->name.c_str());
					return nullptr;
				}
			}
		}

		auto fi = ui->find_function("create");
		if (!fi)
		{
			printf("cannot add component: cannot find create function of %s\n", ui->name.c_str());
			return nullptr;
		}

		if (fi->return_type != TypeInfo::get(TagPU, ui->name) || fi->parameters.size() != 1 || fi->parameters[0] != TypeInfo::get<Entity*>())
		{
			printf("cannot add component: %s's create function format does not match\n", ui->name.c_str());
			return nullptr;
		}

		auto c = fi->call<Component*, Entity*>(nullptr, this);
		if (!c)
		{
			printf("cannot add component: %s's create funcion returned nullptr\n", ui->name.c_str());
			return nullptr;
		}

		c->type_hash = hash;
		c->entity = this;

		for (auto _c : require_comps)
		{
			_c.first->n_strong_ref++;
			*(void**)((char*)c + _c.second) = _c.first;
		}

		c->on_init();

		for (auto& _c : components)
			_c->on_component_added(c);

		component_map.emplace(c->type_hash, c);
		components.emplace_back(c);

		if (world)
			c->on_active();

		return c;
	}

	bool EntityPrivate::remove_component(uint hash)
	{
		auto it = component_map.find(hash);
		if (it == component_map.end())
		{
			printf("cannot remove component: component with hash %u does not exist\n", hash);
			return false;
		}

		auto c = it->second;
		if (c->n_strong_ref != 0)
		{
			printf("cannot remove component: component is strongly referenced by other compoent(s)\n", hash);
			return false;
		}

		{
			auto ui = find_udt(hash);
			for (auto& vi : ui->variables)
			{
				if (vi.metas.get("requires"_h))
				{
					auto comp = get_component(sh(vi.type->name.c_str()));
					if (comp)
						comp->n_strong_ref--;
					else
						assert(0);
				}
			}
		}

		component_map.erase(it);

		for (auto it = components.begin(); it != components.end(); it++)
		{
			if (it->get() == c)
			{
				it->release();
				components.erase(it);
				break;
			}
		}

		for (auto& _c : components)
			_c->on_component_removed(c);

		if (world)
			c->on_active();

		delete c;

		return true;
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
		e->update_enable();

		for (auto& l : e->message_listeners.list)
			l.first("entity_added"_h, nullptr, nullptr);
		for (auto& c : e->components)
			c->on_entity_added();

		if (world)
		{
			e->forward_traversal([this](EntityPrivate* e) {
				e->world = world;
				if (e->global_enable)
				{
					for (auto& l : e->message_listeners.list)
						l.first("active"_h, nullptr, nullptr);
					for (auto& c : e->components)
						c->on_active();
				}
			});
		}

		for (auto& l : message_listeners.list)
			l.first("child_added"_h, e, nullptr);
		for (auto& c : components)
			c->on_child_added(e);
	}

	void EntityPrivate::on_child_removed(EntityPrivate* e) const
	{
		e->parent = nullptr;

		for (auto& l : e->message_listeners.list)
			l.first("entity_removed"_h, nullptr, nullptr);
		for (auto& c : e->components)
			c->on_entity_removed();

		if (world)
		{
			e->backward_traversal([](EntityPrivate* e) {
				e->world = nullptr;
				if (e->global_enable)
				{
					for (auto& c : e->components)
						c->on_inactive();
					for (auto& l : e->message_listeners.list)
						l.first("inactive"_h, nullptr, nullptr);
					e->global_enable = false;
				}
			});
		}

		for (auto& l : message_listeners.list)
			l.first("child_removed"_h, e, nullptr);
		for (auto& c : components)
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

	EntityPtr EntityPrivate::copy()
	{
		auto ret = Entity::create();
		ret->name = name;
		ret->tag = tag;
		ret->set_enable(enable);
		for (auto& c : components)
		{
			auto cc = ret->add_component(c->type_hash);
			auto& ui = *find_udt(c->type_hash);
			for (auto& a : ui.attributes)
			{
				if (a.var_idx != -1)
				{
					if (a.var()->metas.get("requires"_h))
						continue;
				}
				auto v = a.get_value(c.get());
				a.set_value(cc, v);
			}
		}
		for (auto& c : children)
			ret->add_child(c->copy());
		return ret;
	}

	static EntityPtr find_with_file_id(EntityPtr e, const std::string& id)
	{
		if (e->file_id == id)
			return e;
		for (auto& c : e->children)
		{
			auto ret = find_with_file_id(c.get(), id);
			if (ret)
				return ret;
		}
		return nullptr;
	}

	bool get_modification_target(const std::string& target, EntityPtr e, void*& obj, const Attribute*& attr)
	{
		auto sp = SUS::split(target, '|');

		auto te = find_with_file_id(e, sp.front());
		if (!te)
		{
			printf("prefab modification: cannot find target: %s\n", sp.front().c_str());
			return false;
		}

		obj = nullptr;
		UdtInfo* ui = nullptr;

		if (sp.size() == 2)
		{
			obj = te;
			ui = TypeInfo::get<Entity>()->retrive_ui();
		}
		else
		{
			auto hash = sh(sp[1].c_str());
			obj = te->get_component(hash);
			ui = find_udt(hash);
			if (!obj)
			{
				printf("prefab modification: cannot find component %s of target %s\n", sp[1].c_str(), sp.front().c_str());
				return false;
			}
			if (!ui)
			{
				printf("prefab modification: cannot find UdtInfo of %s\n", sp[1].c_str());
				return false;
			}
		}

		attr = ui->find_attribute(sp.back());
		if (!attr)
		{
			if (obj == te)
				printf("prefab modification: cannot find attribute %s of target %s\n", sp.back().c_str(), sp.front().c_str());
			else
				printf("prefab modification: cannot find attribute %s of component %s of target %s\n", sp.back().c_str(), sp[1].c_str(), sp.front().c_str());
			return false;
		}

		return true;
	}

	bool EntityPrivate::load(const std::filesystem::path& _filename)
	{
		pugi::xml_document doc;
		pugi::xml_node doc_root;

		auto filename = Path::get(_filename);
		if (!doc.load_file(filename.c_str()) || (doc_root = doc.first_child()).name() != std::string("prefab"))
		{
			wprintf(L"prefab does not exist or wrong format: %s\n", _filename.c_str());
			return false;
		}

		auto base_path = Path::reverse(filename).parent_path();

		UnserializeXmlSpec spec;
		spec.data_delegates[TypeInfo::get<std::filesystem::path>()] = [&](const std::string& str, void* dst) {
			*(std::filesystem::path*)dst = Path::combine(base_path, str);
		};
		spec.obj_delegates[TypeInfo::get<Component*>()] = [&](pugi::xml_node src, void* dst_o)->void* {
			std::string name = src.attribute("type_name").value();
			auto hash = sh(name.c_str());
			auto ui = find_udt(hash);
			if (ui)
			{
				auto c = ((EntityPtr)dst_o)->add_component(hash);
				if (c)
				{
					if (auto a = src.attribute("enable"); a)
						c->enable = a.as_bool();
					unserialize_xml(*ui, src, c, spec);
				}
			}
			else
				printf("cannot find component with name %s\n", name.c_str());
			return INVALID_POINTER;
		};
		spec.obj_delegates[TypeInfo::get<Entity*>()] = [&](pugi::xml_node src, void* dst_o)->void* {
			auto e = new EntityPrivate();

			if (auto a = src.attribute("filename"); a)
			{
				e->load(Path::combine(base_path, a.value()));
				new PrefabInstance(e, a.value());

				auto n_mod = src.child("modifications");
				for (auto n : n_mod)
				{
					std::string target = n.attribute("target").value();

					void* obj;
					const Attribute* attr;
					if (!get_modification_target(target, e, obj, attr))
						continue;

					unserialize_xml(*attr->ui, attr->var_off(), attr->type, "value", attr->setter_idx, n, obj);
					e->prefab_instance->modifications.push_back(target);
				}
			}
			else
			{
				if (auto a = src.attribute("file_id"); a)
					e->file_id = a.value();
				else
					e->file_id = e->instance_id;
				unserialize_xml(src, e, spec);
			}

			((EntityPtr)dst_o)->add_child(e);

			return INVALID_POINTER;
		};

		if (auto a = doc_root.attribute("file_id"); a)
			file_id = a.value();
		else
			file_id = instance_id;

		unserialize_xml(doc_root, this, spec);

		return true;
	}

	bool EntityPrivate::save(const std::filesystem::path& filename)
	{
		pugi::xml_document file;

		auto base_path = Path::reverse(filename).parent_path();

		SerializeXmlSpec spec;
		spec.excludes.emplace_back("flame::cNode"_h, "eul"_h);
		spec.data_delegates[TypeInfo::get<std::filesystem::path>()] = [&](void* src) {
			auto& path = *(std::filesystem::path*)src;
			return Path::rebase(base_path, path).string();
		};
		spec.obj_delegates[TypeInfo::get<Component*>()] = [&](void* src, pugi::xml_node dst) {
			auto comp = (Component*)src;
			auto ui = find_udt(comp->type_hash);
			if (ui)
			{
				dst.append_attribute("type_name").set_value(ui->name.c_str());
				if (!comp->enable)
					dst.append_attribute("enable").set_value("false");
				serialize_xml(*ui, comp, dst, spec);
			}
		};
		spec.obj_delegates[TypeInfo::get<Entity*>()] = [&](void* src, pugi::xml_node dst) {
			auto e = (EntityPtr)src;
			if (e->prefab_instance)
			{
				dst.append_attribute("filename").set_value(Path::rebase(base_path, e->prefab_instance->filename).string().c_str());
				auto n_mod = dst.append_child("modifications");
				for (auto& target : e->prefab_instance->modifications)
				{
					void* obj;
					const Attribute* attr;
					if (!get_modification_target(target, e, obj, attr))
						continue;

					auto n = n_mod.append_child("item");
					n.append_attribute("target").set_value(target.c_str());
					serialize_xml(*attr->ui, attr->var_off(), attr->type, "value", "", attr->getter_idx, obj, n);
				}
			}
			else
			{
				dst.append_attribute("file_id").set_value(e->file_id.c_str());
				serialize_xml(e, dst, spec);
			}
		};

		auto doc_root = file.append_child("prefab");
		doc_root.append_attribute("file_id").set_value(file_id.c_str());
		serialize_xml(this, doc_root, spec);

		file.save_file(filename.c_str());

		return true;
	}

	struct EntityCreate : Entity::Create
	{
		EntityPtr operator()(std::string* file_id) override
		{
			auto ret = new EntityPrivate();
			if (file_id)
				ret->file_id = *file_id;
			else
				ret->file_id = ret->instance_id;
			return ret;
		}
	}Entity_create;
	Entity::Create& Entity::create = Entity_create;
}
