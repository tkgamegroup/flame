#pragma once

#include "element_renderer.h"

namespace flame
{
	struct sElementRendererPrivate : sElementRenderer
	{
		void update() override;
	};
}
