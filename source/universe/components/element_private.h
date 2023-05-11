#pragma once

#include "element.h"

namespace flame
{
	struct cElementPrivate : cElement
	{
		bool transform_dirty = true;

		void on_init() override;

		void set_pos(const vec2& pos) override;
		void set_global_pos(const vec2& pos) override;
		void set_ext(const vec2& ext) override;
		void set_global_ext(const vec2& ext) override;
		void set_scl(const vec2& scl) override;

		void set_background_col(const cvec4& col) override;
		void set_frame_col(const cvec4& col) override;
		void set_frame_thickness(float thickness) override;

		void set_scissor(bool v) override;

		void set_horizontal_alignment(ElementAlignment alignment) override;
		void set_vertical_alignment(ElementAlignment alignment) override;

		vec2 global_scl() override;

		void mark_transform_dirty() override;
		void mark_drawing_dirty() override;

		bool update_transform() override;
	};
}
