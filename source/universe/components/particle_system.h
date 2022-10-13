#pragma once

#include "../component.h"

namespace flame
{
	/// Reflect ctor
	struct cParticleSystem : Component
	{
		/// Reflect requires
		cNodePtr node = nullptr;

		/// Reflect
		std::filesystem::path material_name;
		/// Reflect
		virtual void set_material_name(const std::filesystem::path& material_name) = 0;

		graphics::MaterialPtr material = nullptr;
		int material_res_id = -1;

		struct Create
		{
			virtual cParticleSystemPtr operator()(EntityPtr e) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
