#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cText : Component
	{
		// Reflect requires
		cElementPtr element = nullptr;

		// Reflect
		std::wstring text;
		// Reflect
		virtual void set_text(const std::wstring& str) = 0;

		// Reflect
		cvec4 col = cvec4(255);
		// Reflect
		virtual void set_col(const cvec4& col) = 0;

		// Reflect
		uint font_size = 14;
		// Reflect
		virtual void set_font_size(uint size) = 0;

		// Reflect
		std::vector<std::filesystem::path> font_names;
		// Reflect
		virtual void set_font_names(const std::vector<std::filesystem::path>& names) = 0;

		// Reflect
		bool sdf = false;
		// Reflect
		virtual void set_sdf(bool v) = 0;

		// Reflect
		float thickness = 0.f;
		// Reflect
		virtual void set_thickness(float thickness) = 0;
		// Reflect
		float border = 0.f;
		// Reflect
		virtual void set_border(float border) = 0;

		graphics::FontAtlas* font_atlas = nullptr;

		struct Create
		{
			virtual cTextPtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
