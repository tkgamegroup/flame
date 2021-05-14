#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Material
		{
			virtual const char* get_name() const = 0;

			virtual vec4 get_color() const = 0;
			virtual float get_metallic() const = 0;
			virtual float get_roughness() const = 0;
			virtual float get_alpha_test() const = 0;

			virtual void get_pipeline_file(wchar_t* dst) const = 0;
			inline std::filesystem::path get_pipeline_file() const
			{
				wchar_t buf[260];
				get_pipeline_file(buf);
				return buf;
			}

			virtual const char* get_pipeline_defines() const = 0;

			virtual void get_texture_file(uint idx, wchar_t* dst) const = 0;
			virtual bool get_texture_srgb(uint idx) const = 0;
			virtual SamplerPtr get_texture_sampler(DevicePtr device, uint idx) const = 0;

			FLAME_GRAPHICS_EXPORTS static Material* get(const wchar_t* filename);
		};

		namespace animation
		{
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
				virtual const char* get_name() const = 0;
				virtual uint get_channels_count() const = 0;
				virtual ChannelPtr get_channel(uint idx) const = 0;
			};
		}

		namespace model
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
				virtual const Weight* get_weights() const = 0;
			};

			struct Mesh
			{
				virtual const char* get_name() const = 0;

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

			struct Node
			{
				virtual const char* get_name() const = 0;

				virtual void get_transform(vec3& pos, quat& quat, vec3& scale) const = 0;

				virtual NodePtr get_parent() const = 0;
				virtual uint get_children_count() const = 0;
				virtual NodePtr get_child(uint idx) const = 0;

				virtual int get_mesh_index() const = 0;
			};

			struct Model
			{
				virtual void release() = 0;

				virtual uint get_meshes_count() const = 0;
				virtual MeshPtr get_mesh(uint idx) const = 0;
				virtual int find_mesh(const char* name) const = 0;

				virtual NodePtr get_root() const = 0;

				virtual uint get_animations_count() const = 0;
				virtual animation::AnimationPtr get_animation(uint idx) const = 0;
				virtual int find_animation(const char* name) const = 0;

				virtual void save(const wchar_t* filename) = 0;
				virtual void generate_prefab(const wchar_t* filename) const = 0;

				FLAME_GRAPHICS_EXPORTS static Model* get_standard(const char* name);
				FLAME_GRAPHICS_EXPORTS static Model* get(const wchar_t* filename);
			};
		}
	}
}