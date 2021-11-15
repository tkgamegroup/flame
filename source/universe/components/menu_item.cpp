#include "../entity_private.h"
#include "element_private.h"
#include "text_private.h"
#include "receiver_private.h"
#include "menu_private.h"
#include "menu_item_private.h"

namespace flame
{
	void cMenuItemPrivate::set_checkable(bool v)
	{
		checkable = v;
	}
	
	void cMenuItemPrivate::set_checked(bool v)
	{
		if (!checkable)
			return;
		if (checked == v)
			return;
		checked = v;
		if (arrow)
			arrow->set_visible(checked);
	}
	
	void cMenuItemPrivate::set_radio_checked() 
	{
		set_checked(true);
		for (auto& c : entity->parent->children)
		{
			if (c.get() == entity)
				continue;
			auto cmi = c->get_component_t<cMenuItemPrivate>();
			if (cmi)
				cmi->set_checked(false);
		}
	}

	void cMenuItemPrivate::on_entered_world()
	{
		element = entity->get_component_i<cElementPrivate>(0);
		assert(element);

		receiver = entity->get_component_t<cReceiverPrivate>();
		assert(receiver);

		text = entity->get_component_t<cTextPrivate>();
		assert(text);

		arrow = entity->find_child("arrow");

		receiver->add_mouse_move_listener([](Capture& c, const ivec2& disp, const ivec2& pos) {
			auto thiz = c.thiz<cMenuItemPrivate>();
			auto p = thiz->entity->parent;
			if (p)
			{
				for (auto& e : p->children)
				{
					auto cm = e->get_component_t<cMenuPrivate>();
					if (cm)
						cm->close();
				}
			}
			}, Capture().set_thiz(this));

		element->add_padding(vec4(text->font_size, 0.f, 0.f, 0.f));
	}
	
	cMenuItem* cMenuItem::create(void* parms)
	{
		return new cMenuItemPrivate();
	}
}
