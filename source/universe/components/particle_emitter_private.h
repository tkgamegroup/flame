#pragma once

#include "particle_emitter.h"
#include "node_private.h"

namespace flame
{
	struct cParticleEmitterPrivate : cParticleEmitter, NodeDrawer
	{
		void draw(sRendererPtr s_renderer) override;
	};
}
