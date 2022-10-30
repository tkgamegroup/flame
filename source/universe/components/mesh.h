#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cMesh : Component
	{
		// Reflect requires
		cNodePtr node = nullptr;

		// Reflect
		std::filesystem::path mesh_name;
		// Reflect
		virtual void set_mesh_name(const std::filesystem::path& mesh_name) = 0;

		// Reflect
		std::filesystem::path material_name;
		// Reflect
		virtual void set_material_name(const std::filesystem::path& material_name) = 0;

		inline void set_mesh_and_material(const std::filesystem::path& mesh_name, const std::filesystem::path& material_name)
		{
			set_mesh_name(mesh_name);
			set_material_name(material_name);
		}

		// Reflect
		bool cast_shadow = true;
		// Reflect
		virtual void set_cast_shadow(bool v) = 0;

		graphics::ModelPtr model = nullptr;
		graphics::MeshPtr mesh = nullptr;
		graphics::MaterialPtr material = nullptr;
		int mesh_res_id = -1;
		int material_res_id = -1;
		int instance_id = -1;

		struct Create
		{
			virtual cMeshPtr operator()(EntityPtr e) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
