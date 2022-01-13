#pragma once

#include "../component.h"

namespace flame
{
	/// Reflect ctor
	struct cMesh : Component
	{
		/// Reflect
		std::filesystem::path model_name = L"standard:cube";
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

		/// Reflect
		ShadingFlags shading_flags = ShadingMaterial;
		/// Reflect
		virtual void set_shading_flags(ShadingFlags flags) = 0;

		struct Create
		{
			virtual cMeshPtr operator()(EntityPtr e) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_EXPORTS static Create& create;
	};
}
