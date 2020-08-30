#pragma once

#include <flame/graphics/model.h>

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
			std::vector<ModelVertex> vertices;
			std::vector<uint> indices;

			void set_vertices_p(const std::initializer_list<float>& v);
			void set_vertices_pn(const std::initializer_list<float>& v);
			void set_indices(const std::initializer_list<uint>& v);
		};

		struct ModelPrivate : Model
		{
			std::vector<std::unique_ptr<ModelMesh>> meshes;
		};
	}
}
