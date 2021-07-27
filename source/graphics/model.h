#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Bone
		{
			virtual const char* get_name() const = 0;

			virtual mat4 get_offset_matrix() const = 0;
		};

		struct Mesh
		{
			virtual uint get_skins_count() const = 0;
			virtual MaterialPtr get_material(uint skin = 0) const = 0;

			virtual uint get_vertices_count() const = 0;
			virtual const vec3* get_positions() const = 0;
			virtual const vec2* get_uvs() const = 0;
			virtual const vec3* get_normals() const = 0;
			virtual const ivec4* get_bone_ids() const = 0;
			virtual const vec4* get_bone_weights() const = 0;

			virtual uint get_indices_count() const = 0;
			virtual const uint* get_indices() const = 0;

			virtual vec3 get_lower_bound() const = 0;
			virtual vec3 get_upper_bound() const = 0;
		};

		struct Model
		{
			virtual void release() = 0;

			virtual uint get_meshes_count() const = 0;
			virtual MeshPtr get_mesh(uint idx) const = 0;

			virtual uint get_bones_count() const = 0;
			virtual BonePtr get_bone(uint idx) const = 0;

			FLAME_GRAPHICS_EXPORTS static void convert(const wchar_t* filename);
			FLAME_GRAPHICS_EXPORTS static Model* get_standard(const wchar_t* name);
			FLAME_GRAPHICS_EXPORTS static Model* get(const wchar_t* filename);
		};

		struct BoneKey
		{
			vec3 p;
			quat q;
		};

		struct Channel
		{
			virtual const char* get_node_name() const = 0;
			virtual uint get_keys_count() const = 0;
			virtual const BoneKey* get_keys() const = 0;
		};

		struct Animation
		{
			virtual uint get_channels_count() const = 0;
			virtual ChannelPtr get_channel(uint idx) const = 0;

			FLAME_GRAPHICS_EXPORTS static Animation* get(const wchar_t* filename);
		};
	}
}
