#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cNavMesh : Component
	{
		// Reflect
		std::filesystem::path filename;
		// Reflect
		virtual void set_filename(const std::filesystem::path& filename) = 0;

		struct Create
		{
			virtual cNavMeshPtr operator()(EntityPtr e) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
