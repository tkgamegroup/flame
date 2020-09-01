#pragma once

#include <flame/universe/components/object.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct cNodePrivate;
	struct cCameraPrivate;

	struct cObjectBridge : cObject
	{
		void set_src(const char* src) override;
	};

	struct cObjectPrivate : cObjectBridge // R ~ on_*
	{
		std::string src;

		cNodePrivate* node = nullptr; // R ref
		graphics::Canvas* canvas = nullptr; // R ref
		int model_idx = -1;

		const char* get_src() const override { return src.c_str(); }
		void set_src(const std::string& src);

		void on_gain_canvas();

		void apply_src();

		void draw(graphics::Canvas* canvas, cCamera* camera); // R
	};

	inline void cObjectBridge::set_src(const char* src)
	{
		((cObjectPrivate*)this)->set_src(src);
	}
}
