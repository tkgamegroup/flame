#pragma once

#include "model.h"

namespace flame
{
	namespace graphics
	{
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

		struct MeshPrivate : Mesh
		{
			ModelPrivate* model;

			MaterialPtr material = nullptr;

			std::vector<vec3> positions;
			std::vector<vec2> uvs;
			std::vector<vec3> normals;
			std::vector<uint> indices;

			std::vector<std::unique_ptr<BonePrivate>> bones;

			vec3 lower_bound = vec3(0.f);
			vec3 upper_bound = vec3(0.f);

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

		struct ModelPrivate : Model
		{
			std::filesystem::path filename;

			std::vector<std::unique_ptr<MeshPrivate>> meshes;

			void release() override { delete this; }

			uint get_meshes_count() const override { return meshes.size(); }
			MeshPtr get_mesh(uint idx) const override { return meshes[idx].get(); }
		};

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
			std::filesystem::path filename;

			std::vector<std::unique_ptr<ChannelPrivate>> channels;

			uint get_channels_count() const override { return channels.size(); }
			ChannelPtr get_channel(uint idx) const { return channels[idx].get(); }
		};
	}
}
