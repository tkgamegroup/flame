#include "particle_emitter_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cParticleEmitterPrivate::draw(sRendererPtr s_renderer)
	{
		sRenderer::Particle p;
		p.coord = vec3(200, 50, 200);
		p.xext = vec3(10.f, 0.f, 0.f);
		p.yext = vec3(0.f, 10.f, 0.f);
		p.uvs = vec4(0.f, 0.f, 1.f, 1.f);
		p.color = cvec4(255);
		s_renderer->draw_particles(1, &p, 0);
	}

	cParticleEmitter* cParticleEmitter::create(void* parms)
	{
		return new cParticleEmitterPrivate();
	}
}
