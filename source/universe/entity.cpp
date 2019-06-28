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
		}

		Component* get_component(uint type_hash)
		{
			for (auto& c : components)
			{
				if (c->type_hash() == type_hash)
					return c.get();
			}
			return nullptr;
		}

		DynamicArray<Component*> get_components(uint type_hash)
		{
			DynamicArray<Component*> ret;
			for (auto& c : components)
			{
				if (c->type_hash() == type_hash)
					ret.push_back(c.get());
			}
			return ret;
		}

		void add_component(Component* c)
		{
			components.emplace_back(c);
			c->entity = this;
			c->on_attach();
		}

		void add_child(EntityPrivate* e)
		{
			children.emplace_back(e);
			e->parent = this;
			e->on_attach();
		}

		void on_attach()
		{
			for (auto& c : components)
				c->on_attach();
			for (auto& e : children)
				e->on_attach();
		}

		void update(float delta_time)
		{
			if (!parent)
			{
				if (visible.frame > global_visible.frame)
					global_visible = visible;
			}
			else
			{
				if (visible.frame > global_visible.frame || parent->global_visible.frame > global_visible.frame)
					global_visible = visible && parent->global_visible;
			}
			if (!global_visible)
				return;
			for (auto& c : components)
				c->update(delta_time);
			for (auto& e : children)
				e->update(delta_time);
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

	int Entity::component_count() const
	{
		return ((EntityPrivate*)this)->components.size();
	}

	Component* Entity::component(uint type_hash) const
	{
		return ((EntityPrivate*)this)->get_component(type_hash);
	}

	Mail<std::vector<Component*>> Entity::components(uint type_hash) const
	{
		return ((EntityPrivate*)this)->get_components(type_hash);
	}

	void Entity::add_component(Component* c)
	{
		((EntityPrivate*)this)->add_component(c);
	}

	Entity* Entity::parent() const
	{
		return ((EntityPrivate*)this)->parent;
	}

	int Entity::children_count() const
	{
		return ((EntityPrivate*)this)->children.size();
	}

	Entity* Entity::child(int index) const
	{
		return ((EntityPrivate*)this)->children[index].get();
	}

	void Entity::add_child(Entity* e)
	{
		((EntityPrivate*)this)->add_child((EntityPrivate*)e);
	}

	void Entity::update(float delta_time)
	{
		((EntityPrivate*)this)->update(delta_time);
	}

	static void serialize(EntityPrivate* src, SerializableNode* dst)
	{
		dst->new_attr("name", src->name);
		dst->new_attr("visible", to_stdstring(src->visible));
		auto n_components = dst->new_node("components");
		for (auto& c : src->components)
		{
			std::string type_name = c->type_name();
			auto u_c = find_udt(H(type_name.c_str()));
			auto u_a = find_udt(H((type_name + "Archive").c_str()));
			if (u_c)
			{
				auto create_func = u_c->find_function("create");
				if (create_func)
					int cut = 1;
			}
		}
	}

	static void unserialize(EntityPrivate* dst, SerializableNode* src)
	{

	}

	void Entity::load(const std::wstring& filename)
	{
		auto file = SerializableNode::create_from_xml_file(filename);
		if (!file || file->name() != "node")
			return;


	}

	void Entity::save(const std::wstring& filename)
	{

	}

	Entity* Entity::create()
	{
		return new EntityPrivate;
	}

	void Entity::destroy(Entity* w)
	{
		delete (EntityPrivate*)w;
	}
}
