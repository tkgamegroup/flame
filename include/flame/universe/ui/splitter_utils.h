#pragma once

namespace flame
{
	namespace ui
	{
		inline void make_splitter(Entity* e, SplitterType type)
		{
			auto ce = cElement::create();
			ce->size_ = 8.f;
			e->add_component(ce);
			e->add_component(cEventReceiver::create());
			auto cs = cStyleColor::create();
			cs->color_normal = Vec4c(0);
			cs->color_hovering = style_4c(FrameColorHovering);
			cs->color_active = style_4c(FrameColorActive);
			e->add_component(cs);
			e->add_component(cSplitter::create(type));
			auto ca = cAligner::create();
			if (type == SplitterHorizontal)
				ca->height_policy_ = SizeFitParent;
			else
				ca->width_policy_ = SizeFitParent;
			e->add_component(ca);
		}
	}
}
