#pragma once

#include "../component.h"

namespace flame
{
	/// Reflect ctor
	struct cMesh : Component
	{
		/// Reflect requires
		cNodePtr node = nullptr;

		/// Reflect
		std::filesystem::path model_name;
		/// Reflect
		virtual void set_model_name(const std::filesystem::path& model_name) = 0;

		/// Reflect
		uint mesh_index = 0;
		/// Reflect
		virtual void set_mesh_index(uint idx) = 0;

		/// Reflect
		uint skin_index = 0;
		/// Reflect
		virtual void set_skin_index(uint idx) = 0;

		/// Reflect
		bool cast_shadow = true;
		/// Reflect
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
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
