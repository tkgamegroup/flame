#include <flame/foundation/window.h>
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
		std::vector<std::pair<Entity*, bool>> trashbin;

		EntityPrivate() :
			parent(nullptr)
		{
			created_frame = app_frame();

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
			c->on_added();
		}

		EntityPrivate* find_child(const std::string& name) const
		{
			for (auto& e : children)
			{
				if (e->name == name)
					return e.get();
			}
			return nullptr;
		}

		void add_child(EntityPrivate* e)
		{
			children.emplace_back(e);
			e->parent = this;
		}

		void remove_child(EntityPrivate* e)
		{
			for (auto it = children.begin(); it != children.end(); it++)
			{
				if (it->get() == e)
				{
					children.erase(it);
					return;
				}
			}
		}

		void take_child(EntityPrivate* e)
		{
			for (auto it = children.begin(); it != children.end(); it++)
			{
				if (it->get() == e)
				{
					e->parent = nullptr;
					it->release();
					children.erase(it);
					return;
				}
			}
		}

		void remove_all_children()
		{
			children.clear();
		}

		void take_all_children()
		{
			for (auto& e : children)
			{
				e->parent = nullptr;
				e.release();
			}
			children.clear();
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
			for (auto it = children.rbegin(); it != children.rend(); it++)
			{
				auto c = it->get();
				if (c->global_visible)
					c->traverse_backward(callback, capture);
			}
			callback(capture.p, this);
		}

		void update()
		{
			for (auto& e : trashbin)
			{
				if (!e.second)
					e.first->parent()->remove_child(e.first);
				else
					e.first->parent()->take_child(e.first);
			}
			trashbin.clear();
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
		return ((EntityPrivate*)this)->find_child(name);
	}

	void Entity::add_child(Entity* e)
	{
		((EntityPrivate*)this)->add_child((EntityPrivate*)e);
	}

	void Entity::remove_child(Entity* e)
	{
		((EntityPrivate*)this)->remove_child((EntityPrivate*)e);
	}

	void Entity::take_child(Entity* e)
	{
		((EntityPrivate*)this)->take_child((EntityPrivate*)e);
	}

	void Entity::remove_all_children()
	{
		((EntityPrivate*)this)->remove_all_children();
	}

	void Entity::take_all_children()
	{
		((EntityPrivate*)this)->take_all_children();
	}

	void Entity::add_to_trashbin(Entity* root, bool is_take)
	{
		((EntityPrivate*)root)->trashbin.emplace_back(this, is_take);
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

	void* component_alloc(uint size)
	{
		return malloc(size);
	}
}
