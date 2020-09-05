#pragma once

#include <flame/graphics/model.h>

namespace flame
{
	namespace graphics
	{
		struct ModelMeshPrivate : ModelMesh
		{
			std::vector<ModelVertex1> vertices_1;
			std::vector<ModelVertex2> vertices_2;
			std::vector<uint> indices;

			uint material_index = 0;

			uint get_vertices_count_1() const override { return vertices_1.size(); }
			const ModelVertex1* get_vertices_1() const override { return vertices_1.data(); }
			uint get_vertices_count_2() const override { return vertices_2.size(); }
			const ModelVertex2* get_vertices_2() const override { return vertices_2.data(); }
			uint get_indices_count() const { return indices.size(); }
			const uint* get_indices() const { return indices.data(); }

			void set_vertices_p(const std::initializer_list<float>& v);
			void set_vertices_pn(const std::initializer_list<float>& v);
			void set_vertices(uint number, Vec3f* poses, Vec3f* uvs, Vec3f* normals);
			void set_indices(const std::initializer_list<uint>& v);
			void set_indices(uint number, uint* indices);

			void add_sphere(float radius, uint horiSubdiv, uint vertSubdiv, const Vec3f& center, const Mat3f& rotation);
		};

		struct ModelMaterialPrivate
		{
			Vec3f albedo;
			Vec3f spec;
			float roughness;
			std::filesystem::path albedo_map_filename;
			std::filesystem::path spec_map_filename;
			std::filesystem::path roughness_map_filename;
			std::filesystem::path normal_map_filename;
		};

		struct ModelNodePrivate : ModelNode
		{
			std::string name;

			Vec3f pos;
			Vec4f quat;
			Vec3f scale;

			ModelNodePrivate* parent = nullptr;
			std::vector<std::unique_ptr<ModelNodePrivate>> children;

			int mesh_index = -1;

			const char* get_name() const override { return name.c_str(); }

			void get_transform(Vec3f& p, Vec4f& q, Vec3f& s) const override 
			{ 
				p = pos;
				q = quat;
				s = scale;
			}

			ModelNode* get_parent() const override { return parent; }
			uint get_children_count() const { return children.size(); }
			ModelNode* get_child(uint idx) const override { return children[idx].get(); }

			int get_mesh_index() const override { return mesh_index; }
		};


		struct ModelPrivate : Model
		{
			std::vector<std::unique_ptr<ModelMaterialPrivate>> materials;
			std::vector<std::unique_ptr<ModelMeshPrivate>> meshes;

			std::unique_ptr<ModelNodePrivate> root;

			uint get_meshes_count() const override { return meshes.size(); }
			ModelMesh* get_mesh(uint idx) const override { return meshes[idx].get(); }

			ModelNode* get_root() const override { return root.get(); }

			static ModelPrivate* create(const std::filesystem::path& filename);
		};
	}
}
