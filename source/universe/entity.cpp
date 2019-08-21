#include <flame/foundation/foundation.h>
#include <flame/foundation/serialize.h>
#include <flame/universe/entity.h>
#include <flame/universe/component.h>

namespace flame
{
	struct EntityPrivate : Entity
	{
		std::string name;
		std::vector<std::unique_ptr<Component>> components;
		EntityPrivate* parent;
		std::vector<std::unique_ptr<EntityPrivate>> children;

		EntityPrivate() :
			parent(nullptr)
		{
			visible = true;
			global_visible = false;
		}

		Component* find_component(uint type_hash)
		{
			for (auto& c : components)
			{
				if (c->type_hash == type_hash)
					return c.get();
			}
			return nullptr;
		}

		Mail<std::vector<Component*>> find_components(uint type_hash)
		{
			auto ret = new_mail<std::vector<Component*>>();
			for (auto& c : components)
			{
				if (c->type_hash == type_hash)
					ret.p->push_back(c.get());
			}
			return ret;
		}

		void add_component(Component* c)
		{
			components.emplace_back(c);
			c->entity = this;
			c->on_add_to_parent();
		}

		void add_child(EntityPrivate* e)
		{
			children.emplace_back(e);
			e->parent = this;
			e->on_add_to_parent();
		}

		void on_add_to_parent()
		{
			for (auto& c : components)
				c->on_add_to_parent();
			for (auto& e : children)
				e->on_add_to_parent();
		}

		void traverse_forward(void (*callback)(void* c, Entity* n), const Mail<>& capture)
		{
			callback(capture.p, this);
			for (auto& c : children)
			{
				if (c->global_visible)
					c->traverse_forward(callback, capture);
			}
		}

		void traverse_backward(void (*callback)(void* c, Entity* n), const Mail<>& capture)
		{
			for (auto& c : children)
			{
				if (c->global_visible)
					c->traverse_backward(callback, capture);
			}
			callback(capture.p, this);
		}

		void update()
		{
			if (!parent)
				global_visible = visible;
			else
				global_visible = visible && parent->global_visible;
			if (!global_visible)
				return;
			for (auto& c : components)
				c->update();
			for (auto& e : children)
				e->update();
		}
	};

	const std::string& Entity::name() const
	{
		return ((EntityPrivate*)this)->name;
	}

	void Entity::set_name(const std::string& name) const
	{
		((EntityPrivate*)this)->name = name;
	}

	uint Entity::component_count() const
	{
		return ((EntityPrivate*)this)->components.size();
	}

	Component* Entity::component(uint index) const
	{
		return ((EntityPrivate*)this)->components[index].get();
	}

	Component* Entity::find_component(uint type_hash) const
	{
		return ((EntityPrivate*)this)->find_component(type_hash);
	}

	Mail<std::vector<Component*>> Entity::find_components(uint type_hash) const
	{
		return ((EntityPrivate*)this)->find_components(type_hash);
	}

	void Entity::add_component(Component* c)
	{
		((EntityPrivate*)this)->add_component(c);
	}

	Entity* Entity::parent() const
	{
		return ((EntityPrivate*)this)->parent;
	}

	uint Entity::child_count() const
	{
		return ((EntityPrivate*)this)->children.size();
	}

	Entity* Entity::child(uint index) const
	{
		return ((EntityPrivate*)this)->children[index].get();
	}

	Entity* Entity::find_child(const std::string& name) const
	{
		for (auto& c : ((EntityPrivate*)this)->children)
		{
			if (c->name == name)
				return c.get();
		}
		return nullptr;
	}

	void Entity::add_child(Entity* e)
	{
		((EntityPrivate*)this)->add_child((EntityPrivate*)e);
	}

	void Entity::traverse_forward(void (*callback)(void* c, Entity* n), const Mail<>& capture)
	{
		((EntityPrivate*)this)->traverse_forward(callback, capture);
		delete_mail(capture);
	}

	void Entity::traverse_backward(void (*callback)(void* c, Entity* n), const Mail<>& capture)
	{
		((EntityPrivate*)this)->traverse_backward(callback, capture);
		delete_mail(capture);
	}

	void Entity::update()
	{
		((EntityPrivate*)this)->update();
	}

	//static void serialize(EntityPrivate* src, SerializableNode* dst)
	//{
	//	dst->new_attr("name", src->name);
	//	dst->new_attr("visible", std::to_string((int)src->visible));
	//	auto n_components = dst->new_node("components");
	//	for (auto& c : src->components)
	//	{
	//		std::string type_name = c->type_name;
	//		auto udt = find_udt(H((type_name + "A").c_str()));
	//		if (udt)
	//		{
	//			auto create_func = udt->find_function("create");
	//			if (create_func)
	//				int cut = 1;
	//		}
	//	}
	//}

	//static void unserialize(EntityPrivate* dst, SerializableNode* src)
	//{

	//}

	Entity* Entity::create()
	{
		return new EntityPrivate;
	}

	Entity* Entity::create_from_file(const std::wstring& filename)
	{
		auto file = SerializableNode::create_from_xml_file(filename);
		if (!file || file->name() != "node")
			return nullptr;
		return nullptr;
	}

	void Entity::save_to_file(Entity* e, const std::wstring& filename)
	{

	}

	void Entity::destroy(Entity* w)
	{
		delete (EntityPrivate*)w;
	}
}
