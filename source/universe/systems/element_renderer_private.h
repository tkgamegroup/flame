#pragma once

#include "element_renderer.h"

namespace flame
{
	struct sElementRendererPrivate : sElementRenderer
	{
		sElementRendererPrivate() {}

		void update() override;
	};
}
