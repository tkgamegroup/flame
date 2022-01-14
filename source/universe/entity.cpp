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
		static auto id = 0;
		created_frame = frames;
		created_id = id++;
	}

	EntityPrivate::~EntityPrivate()
	{
		for (auto& l : message_listeners.list)
			l("destroyed"_h, nullptr, nullptr);
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
					l(global_enable ? "active"_h : "inactive"_h, nullptr, nullptr);
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
			printf("cannot add component: %s already exist\n", ui->name.c_str());
			return nullptr;
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

		for (auto& _c : components)
			_c->on_component_added(c);

		component_map.emplace(c->type_hash, c);
		components.emplace_back(c);

		if (world)
			c->on_active();

		return c;
	}

	void EntityPrivate::remove_component(uint hash, bool destroy)
	{
		auto it = component_map.find(hash);
		if (it == component_map.end())
		{
			printf("cannot remove component: component with hash %u does not exist", hash);
			return;
		}

		auto c = it->second;
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
		e->update_enable();

		for (auto& l : e->message_listeners.list)
			l("entity_added"_h, nullptr, nullptr);
		for (auto& c : e->components)
			c->on_entity_added();

		if (world)
		{
			e->forward_traversal([this](EntityPrivate* e) {
				e->world = world;
				if (e->global_enable)
				{
					for (auto& l : e->message_listeners.list)
						l("active"_h, nullptr, nullptr);
					for (auto& c : e->components)
						c->on_active();
				}
			});
		}

		for (auto& l : message_listeners.list)
			l("child_added"_h, e, nullptr);
		for (auto& c : components)
			c->on_child_added(e);
	}

	void EntityPrivate::on_child_removed(EntityPrivate* e) const
	{
		e->parent = nullptr;

		for (auto& l : e->message_listeners.list)
			l("entity_removed"_h, nullptr, nullptr);
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
						l("inactive"_h, nullptr, nullptr);
					e->global_enable = false;
				}
			});
		}

		for (auto& l : message_listeners.list)
			l("child_removed"_h, e, nullptr);
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
		for (auto& c : components)
		{

		}
		for (auto& c : children)
			ret->add_child(c->copy());
		return ret;
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

		UnserializeXmlSpec spec;
		spec.map[TypeInfo::get<Component*>()] = [&](pugi::xml_node src, void* dst_o)->void* {
			auto hash = src.attribute("type_hash").as_uint();
			auto ui = find_udt(hash);
			if (ui)
			{
				auto c = ((EntityPtr)dst_o)->add_component(hash);
				unserialize_xml(*ui, src, c, {});
			}
			return INVALID_POINTER;
		};
		spec.map[TypeInfo::get<Entity*>()] = [&](pugi::xml_node src, void* dst_o)->void* {
			auto e = Entity::create();
			unserialize_xml(src, e, spec);
			((EntityPtr)dst_o)->add_child(e);
			return INVALID_POINTER;
		};
		unserialize_xml(doc_root, this, spec);

		return true;
	}

	bool EntityPrivate::save(const std::filesystem::path& filename)
	{
		pugi::xml_document file;

		SerializeXmlSpec spec;
		spec.map[TypeInfo::get<Component*>()] = [&](void* src, pugi::xml_node dst) {
			auto comp = (Component*)src;
			dst.append_attribute("type_hash").set_value(comp->type_hash);
			auto ui = find_udt(comp->type_hash);
			if (ui)
				serialize_xml(*ui, comp, dst);
		};
		serialize_xml(this, file.append_child("prefab"), spec);

		file.save_file(filename.c_str());

		return true;
	}

	struct EntityCreatePrivate : Entity::Create
	{
		EntityPtr operator()() override
		{
			auto ret = new EntityPrivate();
			ret->guid = generate_guid();
			return ret;
		}
	}Entity_create_private;
	Entity::Create& Entity::create = Entity_create_private;
}
