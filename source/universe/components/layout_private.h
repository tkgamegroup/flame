#pragma once

#include "layout.h"

namespace flame
{
	struct cLayoutPrivate : cLayout
	{
		void set_type(Type type) override;
		void set_spacing(float spacing) override;

		void update() override;
	};
}
