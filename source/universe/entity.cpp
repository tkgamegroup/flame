#include "../xml.h"
#include "../foundation/typeinfo_serialize.h"
#include "../foundation/system.h"
#include "entity_private.h"
#include "world_private.h"

namespace flame
{
	void Component::set_enable(bool v)
	{
		if (enable == v)
			return;

		if (!enable)
		{
			if (entity->global_enable)
				on_active();
		}
		else
		{
			if (entity->global_enable)
				on_inactive();
		}

		enable = v;

		data_changed("enable"_h);
	}

	EntityPrivate::EntityPrivate()
	{
		instance_id = generate_guid();

#ifdef FLAME_UNIVERSE_DEBUG
		static auto id = 0;
		created_frame = frames;
		created_id = id++;
#endif
	}

	EntityPrivate::~EntityPrivate()
	{
		message_listeners.call("destroyed"_h, nullptr, nullptr);

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
			if (depth == (ushort)-1)
				global_enable = true;
			else
				global_enable = false;
		}
		if (global_enable != prev_enable)
		{
			if (depth != (ushort)-1)
			{
				for (auto& c : components)
					global_enable && c->enable ? c->on_active() : c->on_inactive();
				message_listeners.call(global_enable ? "active"_h : "inactive"_h, nullptr, nullptr);
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

		if (find_component(hash))
		{
			printf("cannot add component: %s already existed\n", ui->name.c_str());
			return nullptr;
		}

		std::vector<std::pair<Component*, uint>> require_comps;
		for (auto& vi : ui->variables)
		{
			if (vi.metas.get("auto_requires"_h))
			{
				if (vi.type->tag == TagPU)
				{
					auto hash = sh(vi.type->name.c_str());
					auto comp = get_component(hash);
					if (!comp)
						comp = add_component(hash);
					if (comp)
						require_comps.emplace_back(comp, vi.offset);
				}
			}
			else if (vi.metas.get("requires"_h))
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
			_c.first->ref++;
			*(void**)((char*)c + _c.second) = _c.first;
		}

		c->on_init();

		for (auto& _c : components)
			_c->on_component_added(c);

		components.emplace_back(c);

		if (global_enable && c->enable)
			c->on_active();

		return c;
	}

	bool EntityPrivate::remove_component(uint hash)
	{
		auto c = find_component(hash);
		if (!c)
		{
			printf("cannot remove component: component with hash %u does not exist\n", hash);
			return false;
		}

		if (c->ref != 0)
		{
			printf("cannot remove component: component is strongly referenced by other compoent(s)\n");
			return false;
		}

		{
			auto ui = find_udt(hash);
			for (auto& vi : ui->variables)
			{
				if (vi.metas.get("requires"_h) || vi.metas.get("auto_requires"_h))
				{
					auto comp = get_component(sh(vi.type->name.c_str()));
					if (comp)
						comp->ref--;
					else
						assert(0);
				}
			}
		}

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

		if (global_enable && c->enable)
			c->on_inactive();

		delete c;

		return true;
	}

	void EntityPrivate::remove_all_components()
	{
		for (auto i = (int)components.size() - 1; i >= 0; i--)
		{
			auto c = components[i].release();
			if (global_enable && c->enable)
				c->on_inactive();
			delete c;
		}
		components.clear();
	}

	bool EntityPrivate::reposition_component(Component* comp)
	{
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

		e->parent = this;
		for (auto i = pos; i < children.size(); i++)
			children[i]->index++;
		children.emplace(children.begin() + pos, e);

		e->index = pos;
		e->update_enable();

		e->message_listeners.call("entity_added"_h, nullptr, nullptr);
		for (auto& c : e->components)
			c->on_entity_added();

		if (depth != (ushort)-1)
		{
			e->forward_traversal([this](EntityPrivate* e) {
				e->depth = e->parent->depth + 1;
				if (e->global_enable)
				{
					e->message_listeners.call("active"_h, nullptr, nullptr);
					for (auto& c : e->components)
					{
						if (c->enable)
							c->on_active();
					}
				}
			});
		}

		message_listeners.call("child_added"_h, e, nullptr);
		for (auto& c : components)
			c->on_child_added(e);
	}

	void EntityPrivate::on_child_removed(EntityPrivate* e) const
	{
		e->parent = nullptr;

		e->message_listeners.call("entity_removed"_h, nullptr, nullptr);
		for (auto& c : e->components)
			c->on_entity_removed();

		if (depth != (ushort)-1)
		{
			e->backward_traversal([](EntityPrivate* e) {
				e->depth = (ushort)-1;
				if (e->global_enable)
				{
					for (auto& c : e->components)
					{
						if (c->enable)
							c->on_inactive();
					}
					e->message_listeners.call("inactive"_h, nullptr, nullptr);
					e->global_enable = false;
				}
			});
		}

		message_listeners.call("child_removed"_h, e, nullptr);
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
		for (auto it2 = it + 1; it2 != children.end(); it2++)
			(*it2)->index--;
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

	EntityPtr EntityPrivate::duplicate(EntityPtr dst)
	{
		if (!dst)
		{
			dst = Entity::create();
			dst->name = name;
			dst->tag = tag;
			dst->set_enable(enable);
		}
		else
		{
			dst->remove_all_children();
			dst->remove_all_components();
		}
		dst->file_id = file_id;
		for (auto& c : components)
		{
			auto cc = dst->add_component(c->type_hash);
			cc->enable = c->enable;
			auto& ui = *find_udt(c->type_hash);
			for (auto& a : ui.attributes)
			{
				if (a.var_idx != -1)
				{
					auto& metas = a.var()->metas;
					if (metas.get("requires"_h) || metas.get("auto_requires"_h))
						continue;
				}
				auto v = a.get_value(c.get());
				a.set_value(cc, v);
			}
		}
		for (auto& c : children)
		{
			if (c->tag & TagNotSerialized)
				continue;
			dst->add_child(c->duplicate());
		}
		return dst;
	}

	ModificationType EntityPrivate::parse_modification_target(const std::string& target, ModificationParsedData& out, voidptr& obj) 
	{
		auto sp = SUS::split(target, '|');

		GUID guid; guid.from_string(std::string(sp.front()));
		auto te = find_with_file_id(guid);
		if (!te)
			return ModificationWrong;

		obj = te;
		auto ui = TypeInfo::get<Entity>()->retrive_ui();

		ModificationType type = ModificationAttributeModify;

		auto name = sp.back();
		if (name == "add_child")
		{
			type = ModificationEntityAdd;
			if (sp.size() == 3)
				out.d.guid.from_string(std::string(sp[1]));
		}
		else if (name == "add")
		{
			type = ModificationComponentAdd;
			out.d.hash = sh(sp[1].data());
		}
		else if (name == "remove")
		{
			type = ModificationComponentRemove;
			out.d.hash = sh(sp[1].data());
		}
		else if (sp.size() == 3)
		{
			auto name = std::string(sp[1]);
			auto hash = sh(name.data());
			obj = te->get_component(hash);
			ui = find_udt(hash);
			if (!obj)
				return ModificationWrong;
			if (!ui)
				return ModificationWrong;
		}

		if (type == ModificationAttributeModify)
		{
			out.d.attr = ui->find_attribute(sp.back());
			if (!out.d.attr)
				return ModificationWrong;
		}

		return type;
	}

	bool EntityPrivate::load(const std::filesystem::path& _filename, bool only_root)
	{
		pugi::xml_document doc;
		pugi::xml_node doc_root;

		auto filename = Path::get(_filename);
		if (!std::filesystem::exists(filename))
		{
			wprintf(L"prefab does not exist: %s\n", _filename.c_str());
			return false;
		}
		if (!doc.load_file(filename.c_str()) || (doc_root = doc.first_child()).name() != std::string("prefab"))
		{
			wprintf(L"prefab is wrong format: %s\n", _filename.c_str());
			return false;
		}

		auto base_path = Path::reverse(filename).parent_path();
		auto path_delegate = [&](const std::string& str, void* dst) {
			auto sp = SUS::split(str, '#');
			auto p = Path::combine(base_path, sp.front());
			if (sp.size() > 1)
			{
				p += L'#';
				p += sp.back();
			}
			*(std::filesystem::path*)dst = p;
		};

		UnserializeXmlSpec spec;
		spec.typed_delegates[TypeInfo::get<std::filesystem::path>()] = path_delegate;
		spec.typed_obj_delegates[TypeInfo::get<Component*>()] = [&](pugi::xml_node src, void* dst_o)->void* {
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
		if (only_root)
			spec.excludes.emplace_back("flame::Entity"_h, "children"_h);
		else
		{
			spec.typed_obj_delegates[TypeInfo::get<Entity*>()] = [&](pugi::xml_node src, void* dst_o)->void* {
				auto e = new EntityPrivate();

				if (auto a = src.attribute("filename"); a)
				{
					auto path = Path::combine(base_path, a.value());
					e->load(path, false);
					new PrefabInstance(e, path);

					for (auto n : src.child("modifications"))
					{
						std::string target = n.attribute("target").value();

						ModificationParsedData data; void* obj;
						auto type = e->parse_modification_target(target, data, obj);
						if (type == ModificationWrong)
						{
							printf("wrong prefab modification target: cannot find target: %s\n", target.c_str());
							continue;
						}

						if (type == ModificationEntityAdd)
						{
							auto new_one = Entity::create();
							((EntityPtr)obj)->add_child(new_one);
							if (auto a = n.attribute("filename"); a)
							{
								auto path = Path::combine(base_path, a.value());
								new_one->load(path, false);
								new PrefabInstance(new_one, path);
							}
							else
								new_one->file_id = data.d.guid;
						}
						else if (type == ModificationComponentAdd)
							((EntityPtr)obj)->add_component(data.d.hash);
						else if (type == ModificationComponentRemove)
						{
							auto comp = (Component*)obj;
							comp->entity->remove_component(comp->type_hash);
						}
						else if (type == ModificationAttributeModify)
						{
							UnserializeXmlSpec spec;
							spec.typed_delegates[TypeInfo::get<std::filesystem::path>()] = path_delegate;
							unserialize_xml(*data.d.attr->ui, data.d.attr->var_off(), data.d.attr->type, "value", 0, data.d.attr->setter_idx, n, obj, spec);
						}
						e->prefab_instance->modifications.push_back(target);
					}
				}
				else
				{
					if (auto a = src.attribute("file_id"); a)
						e->file_id.from_string(a.value());
					else
						e->file_id = e->instance_id;
					unserialize_xml(src, e, spec);
				}

				((EntityPtr)dst_o)->add_child(e);

				return INVALID_POINTER;
			};
		}

		if (auto a = doc_root.attribute("file_id"); a)
			file_id.from_string(a.value());
		else
			file_id = instance_id;

		unserialize_xml(doc_root, this, spec);

		return true;
	}

	bool EntityPrivate::save(const std::filesystem::path& _filename, bool only_root)
	{
		pugi::xml_document doc;
		pugi::xml_node doc_root;
		auto filename = Path::get(_filename);

		if (only_root)
		{
			if (!std::filesystem::exists(filename))
			{
				wprintf(L"prefab does not exist: %s\n", _filename.c_str());
				return false;
			}
			if (!doc.load_file(filename.c_str()) || (doc_root = doc.first_child()).name() != std::string("prefab"))
			{
				wprintf(L"prefab is wrong format: %s\n", _filename.c_str());
				return false;
			}
		}
		else
		{
			doc_root = doc.append_child("prefab");
			std::vector<pugi::xml_attribute> old_attributes;
			for (auto a : doc_root.attributes())
				old_attributes.push_back(a);
			for (auto a : old_attributes)
				doc_root.remove_attribute(a);
			if (auto c = doc_root.child("components"); c)
				doc_root.remove_child(c);
		}

		auto base_path = Path::reverse(filename).parent_path();
		auto path_delegate = [&](void* src)->std::string {
			auto& path = *(std::filesystem::path*)src;
			if (path.native().starts_with(L"0x"))
				return "";
			return Path::rebase(base_path, path).string();
		};

		SerializeXmlSpec spec;
		spec.excludes.emplace_back("flame::cNode"_h, "eul"_h);
		spec.typed_delegates[TypeInfo::get<std::filesystem::path>()] = path_delegate;
		spec.typed_obj_delegates[TypeInfo::get<Component*>()] = [&](void* src, pugi::xml_node dst) {
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
		if (only_root)
			spec.excludes.emplace_back("flame::Entity"_h, "children"_h);
		else
		{
			spec.typed_obj_delegates[TypeInfo::get<Entity*>()] = [&](void* src, pugi::xml_node dst) {
				auto e = (EntityPtr)src;
				if (e->tag & TagNotSerialized)
				{
					dst.parent().remove_child(dst);
					return;
				}
				if (e->prefab_instance)
				{
					dst.append_attribute("filename").set_value(Path::rebase(base_path, e->prefab_instance->filename).string().c_str());
					auto n_mod = dst.append_child("modifications");
					for (auto& target : e->prefab_instance->modifications)
					{
						ModificationParsedData data; void* obj;
						auto type = e->parse_modification_target(target, data, obj);
						if (type == ModificationWrong)
						{
							printf("wrong prefab modification target: cannot find target: %s\n", target.c_str());
							continue;
						}

						auto n = n_mod.append_child("item");
						n.append_attribute("target").set_value(target.c_str());
						if (type == ModificationAttributeModify)
						{
							SerializeXmlSpec spec;
							spec.typed_delegates[TypeInfo::get<std::filesystem::path>()] = path_delegate;
							serialize_xml(*data.d.attr->ui, data.d.attr->var_off(), data.d.attr->type, "value", 0, "", data.d.attr->getter_idx, obj, n, spec);
						}
					}
				}
				else
				{
					dst.append_attribute("file_id").set_value(e->file_id.to_string().c_str());
					serialize_xml(e, dst, spec);
				}
			};
		}

		doc_root.append_attribute("file_id").set_value(file_id.to_string().c_str());
		serialize_xml(this, doc_root, spec);

		doc.save_file(filename.c_str());

		return true;
	}

	struct EntityCreate : Entity::Create
	{
		EntityPtr operator()(GUID* file_id) override
		{
			auto ret = new EntityPrivate();
			if (file_id)
				ret->file_id = *file_id;
			else
				ret->file_id = ret->instance_id;
			return ret;
		}

		EntityPtr operator()(const std::filesystem::path& filename) override
		{
			auto ret = new EntityPrivate();
			ret->load(filename, false);
			return ret;
		}
	}Entity_create;
	Entity::Create& Entity::create = Entity_create;
}
