#pragma once

#include "image.h"

namespace flame
{
	struct cImagePrivate : cImage
	{
		~cImagePrivate();
		void on_init() override;

		void set_auto_size(bool v) override;
		void set_image_name(const std::filesystem::path& image_name) override;
		void set_tint_col(const cvec4& col) override;
	};
}
