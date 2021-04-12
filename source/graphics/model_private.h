#pragma once

#include "model.h"

namespace flame
{
	namespace graphics
	{
		struct MaterialPrivate : Material
		{
			struct Texture
			{
				std::filesystem::path filename;
				Filter mag_filter = FilterLinear;
				Filter min_filter = FilterLinear;
				bool linear_mipmap = true;
				AddressMode address_mode = AddressClampToEdge;
			};

			std::filesystem::path filename;

			std::string name;

			vec4 color = vec4(1.f);
			float metallic = 0.f;
			float roughness = 1.f;
			float alpha_test = 0.f;
			bool double_side = false;

			std::filesystem::path pipeline_file = "standard.mat";
			std::string pipeline_defines;

			std::filesystem::path dir;
			Texture textures[4] = {};

			const char* get_name() const override { return name.c_str(); };
			vec4 get_color() const override { return color; }
			float get_metallic() const override { return metallic; }
			float get_roughness() const override { return roughness; }

			static MaterialPrivate* get(const std::filesystem::path& filename);
		};

		namespace animation
		{
			struct ChannelPrivate : Channel
			{
				std::string node_name;

				std::vector<PositionKey> position_keys;
				std::vector<RotationKey> rotation_keys;

				const char* get_node_name() const override { return node_name.c_str(); }
				uint get_position_keys_count() const override { return position_keys.size(); }
				const PositionKey* get_position_keys() const override { return position_keys.data(); }
				uint get_rotation_keys_count() const { return rotation_keys.size(); }
				const RotationKey* get_rotation_keys() const { return rotation_keys.data(); }
			};

			struct AnimationPrivate : Animation
			{
				std::string name;

				std::vector<std::unique_ptr<ChannelPrivate>> channels;

				const char* get_name() const override { return name.c_str(); }
				uint get_channels_count() const override { return channels.size(); }
				ChannelPtr get_channel(uint idx) const { return channels[idx].get(); }
			};
		}

		namespace model
		{
			struct ModelPrivate : Model
			{
				std::filesystem::path filename;

				std::vector<std::unique_ptr<MaterialPrivate>> materials;
				std::vector<std::unique_ptr<MeshPrivate>> meshes;

				std::unique_ptr<NodePrivate> root;

				std::vector<std::unique_ptr<animation::AnimationPrivate>> animations;

				ModelPrivate();

				void release() override { delete this; }

				uint get_meshes_count() const override { return meshes.size(); }
				MeshPtr get_mesh(uint idx) const override { return meshes[idx].get(); }
				int find_mesh(const std::string& name) const;
				int find_mesh(const char* name) const override { return find_mesh(std::string(name)); }

				NodePtr get_root() const override { return root.get(); }

				uint get_animations_count() const override { return animations.size(); }
				animation::AnimationPtr get_animation(uint idx) const override { return animations[idx].get(); }
				int find_animation(const std::string& name) const;
				int find_animation(const char* name) const override { return find_animation(std::string(name)); }

				void save(const std::filesystem::path& filename);
				void save(const wchar_t* filename) override { save(std::filesystem::path(filename)); }
				void generate_prefab(const std::filesystem::path& filename) const;
				void generate_prefab(const wchar_t* filename) const override { generate_prefab(std::filesystem::path(filename)); }

				static ModelPtr get(const std::filesystem::path& filename);
			};

			struct MeshPrivate : Mesh
			{
				std::string name;

				uint material_index = 0;
				MaterialPtr material = nullptr;

				std::vector<vec3> positions;
				std::vector<vec2> uvs;
				std::vector<vec3> normals;
				std::vector<uint> indices;

				std::vector<std::unique_ptr<BonePrivate>> bones;

				vec3 lower_bound = vec3(0.f);
				vec3 upper_bound = vec3(0.f);

				const char* get_name() const override { return name.c_str(); }

				MaterialPtr get_material() const override { return material; }

				uint get_vertices_count() const override { return positions.size(); }
				const vec3* get_positions() const override { return positions.data(); }
				const vec2* get_uvs() const override { return uvs.empty() ? nullptr : uvs.data(); }
				const vec3* get_normals() const override { return normals.empty() ? nullptr : normals.data(); }
				uint get_indices_count() const { return indices.size(); }
				const uint* get_indices() const { return indices.data(); }

				uint get_bones_count() const override { return bones.size(); }
				BonePtr get_bone(uint idx) const override { return bones[idx].get(); }

				vec3 get_lower_bound() const override { return lower_bound; }
				vec3 get_upper_bound() const override { return upper_bound; }

				void add_vertices(uint n, vec3* positions, vec3* uvs, vec3* normals);
				void add_indices(uint n, uint* indices);

				void add_cube(const vec3& extent, const vec3& center, const mat3& rotation);
				void add_sphere(float radius, uint horiSubdiv, uint vertSubdiv, const vec3& center, const mat3& rotation);
			};

			struct NodePrivate : Node
			{
				std::string name;

				vec3 pos = vec3(0.f);
				quat qut = quat(1.f, 0.f, 0.f, 0.f);
				vec3 scl = vec3(1.f);

				int mesh_index = -1;

				bool trigger = false;

				NodePtr parent = nullptr;
				std::vector<std::unique_ptr<NodePrivate>> children;

				const char* get_name() const override { return name.c_str(); }

				void get_transform(vec3& p, quat& q, vec3& s) const override { p = pos; q = qut; s = scl; }

				NodePtr get_parent() const override { return parent; }
				uint get_children_count() const { return children.size(); }
				NodePtr get_child(uint idx) const override { return children[idx].get(); }

				int get_mesh_index() const override { return mesh_index; }

				NodePrivate* find(const std::string& name);
			};

			struct BonePrivate : Bone
			{
				std::string name;

				mat4 offset_matrix;
				std::vector<Weight> weights;

				const char* get_name() const override { return name.c_str(); }
				mat4 get_offset_matrix() const override { return offset_matrix; }
				uint get_weights_count() const override { return weights.size(); }
				const Weight* get_weights() const override { return weights.data(); }
			};
		}
	}
}
