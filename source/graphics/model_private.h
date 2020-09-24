#pragma once

#include <flame/graphics/model.h>

namespace flame
{
	namespace graphics
	{
		struct MaterialPrivate : Material
		{
			std::string name;

			Vec4f color = Vec4f(Vec3f(1.f), 1.f);
			float metallic = 0.f;
			float roughness = 0.25f;
			float alpha_test = 0.f;
			std::filesystem::path path;
			std::string color_map;
			std::string alpha_map;
			std::string metallic_map;
			std::string roughness_map;
			std::string normal_map;
			std::string height_map;
		};

		struct MeshPrivate : Mesh
		{
			uint ref_cnt = 0;

			uint material_index = 0;

			std::vector<Vec3f> positions;
			std::vector<Vec2f> uvs;
			std::vector<Vec3f> normals;
			std::vector<uint> indices;

			uint get_vertices_count() const override { return positions.size(); }
			const Vec3f* get_positions() const override { return positions.data(); }
			const Vec2f* get_uvs() const override { return uvs.data(); }
			const Vec3f* get_normals() const override { return normals.data(); }
			uint get_indices_count() const { return indices.size(); }
			const uint* get_indices() const { return indices.data(); }

			void set_vertices_p(const std::initializer_list<float>& v);
			void set_vertices_pn(const std::initializer_list<float>& v);
			void set_vertices(uint n, Vec3f* positions, Vec3f* uvs, Vec3f* normals);
			void set_indices(const std::initializer_list<uint>& v);
			void set_indices(uint n, uint* indices);

			void add_sphere(float radius, uint horiSubdiv, uint vertSubdiv, const Vec3f& center, const Mat3f& rotation);
		};

		struct NodePrivate : Node
		{
			std::string name;

			Vec3f pos = Vec3f(0.f);
			Vec4f quat = Vec4f(0.f, 0.f, 0.f, 1.f);
			Vec3f scale = Vec3f(1.f);

			int mesh_index = -1;

			bool trigger = false;

			NodePrivate* parent = nullptr;
			std::vector<std::unique_ptr<NodePrivate>> children;

			const char* get_name() const override { return name.c_str(); }

			void get_transform(Vec3f& p, Vec4f& q, Vec3f& s) const override { p = pos; q = quat; s = scale; }

			Node* get_parent() const override { return parent; }
			uint get_children_count() const { return children.size(); }
			Node* get_child(uint idx) const override { return children[idx].get(); }

			int get_mesh_index() const override { return mesh_index; }

			void traverse(const std::function<void(NodePrivate*)>& callback);
		};

		struct ModelBridge : Model
		{
			void save(const wchar_t* filename, const char* model_name) const override;
		};

		struct ModelPrivate : ModelBridge
		{
			std::filesystem::path filename;

			std::vector<std::unique_ptr<MaterialPrivate>> materials;
			std::vector<std::unique_ptr<MeshPrivate>> meshes;

			std::unique_ptr<NodePrivate> root;

			ModelPrivate();

			uint get_meshes_count() const override { return meshes.size(); }
			Mesh* get_mesh(uint idx) const override { return meshes[idx].get(); }

			Node* get_root() const override { return root.get(); }

			void substitute_material(const char* name, const wchar_t* filename) override;

			void save(const std::filesystem::path& filename, const std::string& model_name) const;

			static ModelPrivate* create(const std::filesystem::path& filename);
		};

		inline void ModelBridge::save(const wchar_t* filename, const char* model_name) const
		{
			return ((ModelPrivate*)this)->save(filename, model_name ? model_name : "");
		}
	}
}
