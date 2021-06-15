#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Bone
		{
			struct Weight
			{
				uint vid;
				float w;
			};

			virtual const char* get_name() const = 0;

			virtual mat4 get_offset_matrix() const = 0;
			virtual uint get_weights_count() const = 0;
			virtual void get_weights(Weight* dst) const = 0;
		};

		struct Mesh
		{
			virtual MaterialPtr get_material() const = 0;

			virtual uint get_vertices_count() const = 0;
			virtual const vec3* get_positions() const = 0;
			virtual const vec2* get_uvs() const = 0;
			virtual const vec3* get_normals() const = 0;

			virtual uint get_indices_count() const = 0;
			virtual const uint* get_indices() const = 0;

			virtual uint get_bones_count() const = 0;
			virtual BonePtr get_bone(uint idx) const = 0;

			virtual vec3 get_lower_bound() const = 0;
			virtual vec3 get_upper_bound() const = 0;
		};

		struct Model
		{
			virtual void release() = 0;

			virtual uint get_meshes_count() const = 0;
			virtual MeshPtr get_mesh(uint idx) const = 0;

			FLAME_GRAPHICS_EXPORTS static void convert(const wchar_t* filename);
			FLAME_GRAPHICS_EXPORTS static Model* get_standard(const char* name);
			FLAME_GRAPHICS_EXPORTS static Model* get(const wchar_t* filename);
		};

		struct PositionKey
		{
			float t;
			vec3 v;
		};

		struct RotationKey
		{
			float t;
			quat v;
		};

		struct Channel
		{
			virtual const char* get_node_name() const = 0;
			virtual uint get_position_keys_count() const = 0;
			virtual const PositionKey* get_position_keys() const = 0;
			virtual uint get_rotation_keys_count() const = 0;
			virtual const RotationKey* get_rotation_keys() const = 0;
		};

		struct Animation
		{
			virtual uint get_channels_count() const = 0;
			virtual ChannelPtr get_channel(uint idx) const = 0;

			FLAME_GRAPHICS_EXPORTS static Animation* get(const wchar_t* filename);
		};
	}
}
