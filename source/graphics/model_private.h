#pragma once

#include <flame/graphics/model.h>

namespace flame
{
	namespace graphics
	{
		struct NodePrivate;

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

		struct BonePrivate : Bone
		{
			std::string name;

			Mat4f offset_matrix;
			std::vector<std::pair<uint, float>> weights;

			const char* get_name() const override { return name.c_str(); }
		};

		struct MeshPrivate : Mesh
		{
			std::string name;

			uint material_index = 0;

			std::vector<Vec3f> positions;
			std::vector<Vec2f> uvs;
			std::vector<Vec3f> normals;
			std::vector<uint> indices;

			std::vector<std::unique_ptr<BonePrivate>> bones;

			uint get_vertices_count() const override { return positions.size(); }
			const Vec3f* get_positions() const override { return positions.data(); }
			const Vec2f* get_uvs() const override { return uvs.data(); }
			const Vec3f* get_normals() const override { return normals.data(); }
			uint get_indices_count() const { return indices.size(); }
			const uint* get_indices() const { return indices.data(); }

			uint get_bones_count() const override { return bones.size(); }
			Bone* get_bone(uint idx) const override { return bones[idx].get(); }

			void add_vertices(uint n, Vec3f* positions, Vec3f* uvs, Vec3f* normals);
			void add_indices(uint n, uint* indices);

			void add_cube(const Vec3f& extent, const Vec3f& center, const Mat3f& rotation);
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

			NodePrivate* find(const std::string& name);
			void traverse(const std::function<void(NodePrivate*)>& callback);
		};

		struct AnimationPrivate
		{
			struct Channel
			{
				struct PositionKey
				{
					float t;
					Vec3f v;
				};
				struct RotationKey
				{
					float t;
					Vec4f v;
				};
				struct ScalingKey
				{
					float t;
					Vec3f v;
				};

				std::string node_name;

				std::vector<PositionKey> position_keys;
				std::vector<RotationKey> rotation_keys;
			};

			std::string name;

			std::vector<Channel> channels;
		};

		struct ModelBridge : Model
		{
			int find_mesh(const char* name) const override;

			void save(const wchar_t* filename, const char* model_name) const override;
		};

		struct ModelPrivate : ModelBridge
		{
			std::filesystem::path filename;

			std::vector<std::unique_ptr<MaterialPrivate>> materials;
			std::vector<std::unique_ptr<MeshPrivate>> meshes;

			std::unique_ptr<NodePrivate> root;

			std::vector<std::unique_ptr<AnimationPrivate>> animations;

			ModelPrivate();

			uint get_meshes_count() const override { return meshes.size(); }
			Mesh* get_mesh(uint idx) const override { return meshes[idx].get(); }
			int find_mesh(const std::string& name) const;

			Node* get_root() const override { return root.get(); }

			void save(const std::filesystem::path& filename, const std::string& model_name) const;

			static ModelPrivate* create(const std::filesystem::path& filename);
		};

		inline int ModelBridge::find_mesh(const char* name) const
		{
			return ((ModelPrivate*)this)->find_mesh(name);
		}

		inline void ModelBridge::save(const wchar_t* filename, const char* model_name) const
		{
			return ((ModelPrivate*)this)->save(filename, model_name ? model_name : "");
		}
	}
}
