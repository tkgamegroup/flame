#pragma once

#include "movie.h"

namespace flame
{
	struct cMoviePrivate : cMovie
	{
		float time = 0.f;

		~cMoviePrivate();
		void on_init() override;

		void set_tint_col(const cvec4& col) override;
	};
}
