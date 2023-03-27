#pragma once

#include "element.h"

namespace flame
{
	struct cElementPrivate : cElement
	{
		bool transform_dirty = true;

		void set_pos(const vec2& pos) override;
		void set_ext(const vec2& ext) override;
		void set_scl(const vec2& scl) override;

		void mark_transform_dirty() override;
		void mark_drawing_dirty() override;

		bool update_transform() override;
	};
}
