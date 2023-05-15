#pragma once

#include "text.h"

namespace flame
{
	struct cTextPrivate : cText
	{
		void on_init() override;
		void on_active() override;

		void set_text(const std::wstring& str) override;
		void set_col(const cvec4& col) override;
		void set_font_size(uint size) override;
		void set_font_names(const std::vector<std::filesystem::path>& names) override;
		void set_sdf(bool v) override;
		void set_thickness(float thickness) override;
		void set_border(float border) override;
		vec2 calc_text_size();
		void set_auto_size(bool v) override;

		void get_font_atlas();
	};
}
