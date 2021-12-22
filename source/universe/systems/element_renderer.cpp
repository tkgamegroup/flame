#include "element_renderer_private.h"

namespace flame
{
	void sElementRendererPrivate::update()
	{

	}

	struct sElementRendererCreatePrivate : sElementRenderer::Create
	{
		sElementRendererPtr operator()() override
		{
			return new sElementRendererPrivate();
		}
	}sElementRenderer_create_private;
	sElementRenderer::Create& sElementRenderer::create = sElementRenderer_create_private;
}
