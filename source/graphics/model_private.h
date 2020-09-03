#pragma once

#include <flame/graphics/model.h>

namespace flame
{
	namespace graphics
	{
		struct ModelMeshPrivate : ModelMesh
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

			uint get_vertices_count() const override { return vertices.size(); }
			const ModelVertex* get_vertices() const override { return vertices.data(); }
			uint get_indices_count() const { return indices.size(); }
			const uint* get_indices() const { return indices.data(); }

			void set_vertices_p(const std::initializer_list<float>& v);
			void set_vertices_pn(const std::initializer_list<float>& v);
			void set_vertices(uint number, Vec3f* poses, Vec3f* uvs, Vec3f* normals, Vec3f* tangents);
			void set_indices(const std::initializer_list<uint>& v);
			void set_indices(uint number, uint* indices);
		};

		struct ModelPrivate : Model
		{
			std::vector<std::unique_ptr<ModelMeshPrivate>> meshes;

			uint get_meshes_count() const override { return meshes.size(); }
			ModelMesh* get_mesh(uint idx) const override { return meshes[idx].get(); }

			static ModelPrivate* create(const std::filesystem::path& filename);
		};
	}
}
