#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct ModelVertex
		{
			Vec3f pos = Vec3f(0.f);
			Vec2f uv = Vec2f(0.f);
			Vec3f normal = Vec3f(0.f);
			Vec3f tangent = Vec3f(0.f);
		};

		struct ModelMesh
		{
			virtual uint get_vertices_count() const = 0;
			virtual const ModelVertex* get_vertices() const = 0;
			virtual uint get_indices_count() const = 0;
			virtual const uint* get_indices() const = 0;
		};

		enum StandardModel
		{
			StandardModelCube,
			StandardModelSphere,

			StandardModelCount
		};

		struct Model
		{
			virtual uint get_meshes_count() const = 0;
			virtual ModelMesh* get_mesh(uint idx) const = 0;

			FLAME_GRAPHICS_EXPORTS static Model* get_standard(StandardModel m);
			FLAME_GRAPHICS_EXPORTS static Model* create(const wchar_t* filename);
		};
	}
}
