#pragma once

#include "stretched_image.h"

namespace flame
{
	struct cStretchedImagePrivate : cStretchedImage
	{
		vec4 border;

		~cStretchedImagePrivate();
		void on_init() override;

		void set_image_name(const std::filesystem::path& image_name) override;
		void set_tint_col(const cvec4& col) override;
		void set_image_scale(float scale) override;
	};
}
