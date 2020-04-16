#pragma once

#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/splitter.h>

namespace flame
{
	namespace utils
	{
		inline void make_splitter(Entity* e, SplitterType type)
		{
			auto ce = cElement::create();
			ce->size = 8.f;
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
				ca->y_align_flags = AlignMinMax;
			else
				ca->x_align_flags = AlignMinMax;
			e->add_component(ca);
		}
	}
}
