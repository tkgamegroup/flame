#pragma once

#include <flame/engine/core/object.h>

namespace flame
{
	enum ComponentType
	{
		ComponentTypeController,
		ComponentTypeCamera,
		ComponentTypeLight,
		ComponentTypeModelInstance,
		ComponentTypeTerrain,
		ComponentTypeWater
	};

	class Node;
	struct XMLNode;

	class Component : public Object
	{
	private:
		ComponentType type;
		Node *parent;
		friend class Node;
	public:
		Component(ComponentType _type);
		virtual ~Component();

		ComponentType get_type() const;
		Node *get_parent() const;

		virtual void serialize(XMLNode *dst) {};
		virtual void unserialize(XMLNode *src) {};
	protected:
		virtual void on_update() {};
	};
}
