#pragma once

#include "../../graphics/model_ext.h"
#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cProcedureMesh : Component
	{
		// Reflect requires
		cMeshPtr cmesh = nullptr;

		// Reflect
		std::filesystem::path blueprint_name; // use a blueprint as procedure mesh
		// Reflect
		virtual void set_blueprint_name(const std::filesystem::path& name) = 0;

		graphics::MeshPtr converted_mesh;

		struct Create
		{
			virtual cProcedureMeshPtr operator()(EntityPtr e) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}

