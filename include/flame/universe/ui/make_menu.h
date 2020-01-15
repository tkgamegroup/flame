#pragma once

namespace flame
{
	inline void make_menu(Entity* e)
	{
		auto ce = cElement::create();
		ce->color_ = ui::style(ui::WindowColor).c();
		e->add_component(ce);
		e->add_component(cLayout::create(LayoutVertical));
		e->add_component(cMenu::create());
	}
}
