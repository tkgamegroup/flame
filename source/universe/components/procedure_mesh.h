#pragma once

#include "../../graphics/model_ext.h"
#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cProcedureMesh : Component
	{
		// Reflect requires
		cMeshPtr mesh = nullptr;

		// Reflect
		std::filesystem::path blueprint_path; // use a blueprint as procedure mesh

		graphics::Mesh converted_mesh;

		struct Create
		{
			virtual cProcedureMeshPtr operator()(EntityPtr e) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}

