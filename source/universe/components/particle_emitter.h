#pragma once

#include "../component.h"

namespace flame
{
	struct cParticleEmitter : Component
	{
		inline static auto type_name = "flame::cMesh";
		inline static auto type_hash = ch(type_name);

		cParticleEmitter() :
			Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static cParticleEmitter* create(void* parms = nullptr);
	};
}
