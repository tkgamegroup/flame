#include "../entity_private.h"
#include "list_private.h"

namespace flame
{
	void cListPrivate::set_selected(EntityPtr v)
	{
		if (selected == v)
			return;
		if (selected)
			selected->set_state((StateFlags)(selected->state & (~StateSelected)));
		if (v)
			v->set_state((StateFlags)(v->state | StateSelected));
		selected = v;
		entity->component_data_changed(this, S<"selected"_h>);
	}
	
	cList* cList::create(void* parms)
	{
		return new cListPrivate();
	}
}
