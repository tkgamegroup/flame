#include "element_renderer_private.h"

namespace flame
{
	void sElementRendererPrivate::update()
	{

	}

	struct sElementRendererCreatePrivate : sElementRenderer::Create
	{
		sElementRendererPtr operator()(WorldPtr w) override
		{
			if (!w)
				return new sElementRendererPrivate();

			return new sElementRendererPrivate();
		}
	}sElementRenderer_create_private;
	sElementRenderer::Create& sElementRenderer::create = sElementRenderer_create_private;
}
