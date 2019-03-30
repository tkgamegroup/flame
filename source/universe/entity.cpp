// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <flame/foundation/foundation.h>
#include <flame/universe/entity.h>
#include <flame/universe/component.h>

namespace flame
{
	struct EntityPrivate : Entity
	{
		std::string name;
		std::map<uint, std::vector<std::unique_ptr<Component>>> components;
		Entity* parent;
		std::vector<std::unique_ptr<Entity>> children;

		inline Component* get_component(uint type_hash)
		{
			for (auto& cl : components)
			{
				if (cl.first == type_hash)
					return cl.second[0].get();
			}
			return nullptr;
		}

		inline Array<Component*> get_components(uint type_hash)
		{
			Array<Component*> ret;
			for (auto& cl : components)
			{
				if (cl.first == type_hash)
					ret.push_back(cl.second[0].get());
			}
			return ret;
		}

		inline void add_component(Component* c)
		{
			components[c->type_hash()].emplace_back(c);
			c->entity = this;
		}

		inline void add_child(Entity* e)
		{
			children.emplace_back(e);
			e->parent = this;
		}

		inline void update(float delta_time)
		{
			for (auto& cl : components)
			{
				for (auto& c : cl.second)
					c->update(delta_time);
			}
			for (auto& e : children)
				e->update(delta_time);
		}
	};

	const char* Entity::name() const
	{
		return ((EntityPrivate*)this)->name.c_str();
	}

	void Entity::set_name(const char* name) const
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

	Array<Component*> Entity::components(uint type_hash) const
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
		((EntityPrivate*)this)->add_child(e);
	}

	void Entity::update(float delta_time)
	{
		((EntityPrivate*)this)->update(delta_time);
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
