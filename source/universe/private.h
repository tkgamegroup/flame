#include <flame/universe/component.h>
#include <flame/universe/entity.h>
#include <flame/universe/world.h>

namespace flame
{
	inline void mark_dying(Entity* e)
	{
		e->dying_ = true;
		for (auto c : e->children)
			mark_dying(c);
	}
}
