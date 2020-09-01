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

			Vec3f albedo;
			Vec3f spec;
			float roughness;
			std::filesystem::path albedo_map_filename;
			std::filesystem::path spec_map_filename;
			std::filesystem::path roughness_map_filename;
			std::filesystem::path normal_map_filename;

			void set_vertices_p(const std::initializer_list<float>& v);
			void set_vertices_pn(const std::initializer_list<float>& v);
			void set_vertices(uint number, Vec3f* poses, Vec3f* uvs, Vec3f* normals, Vec3f* tangents);
			void set_indices(const std::initializer_list<uint>& v);
			void set_indices(uint number, uint* indices);
		};

		struct ModelPrivate : Model
		{
			std::vector<std::unique_ptr<ModelMesh>> meshes;

			static ModelPrivate* create(const std::filesystem::path& filename);
		};
	}
}
