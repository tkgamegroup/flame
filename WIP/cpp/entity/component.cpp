#include <flame/engine/entity/node.h>
#include <flame/engine/entity/component.h>

namespace flame
{
	Component::Component(ComponentType _type) :
		type(_type)
	{
	}

	Component::~Component() {}

	ComponentType Component::get_type() const
	{
		return type;
	}

	Node *Component::get_parent() const
	{
		return parent;
	}
}
