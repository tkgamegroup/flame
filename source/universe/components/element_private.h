#pragma once

#include "element.h"

namespace flame
{
	struct cElementPrivate : cElement
	{
		void set_pos(const vec2& pos) override;
		void set_ext(const vec2& ext) override;

		void mark_transform_dirty() override;
		void mark_drawing_dirty() override;
	};
}
