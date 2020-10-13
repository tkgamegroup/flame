#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Material
		{
			//virtual const char* get_name() const = 0;

			//virtual int get_bound_index() const = 0;

			//FLAME_GRAPHICS_EXPORTS static Material* create(const wchar_t* path);
		};

		struct Bone
		{
			virtual const char* get_name() const = 0;
		};

		struct Mesh
		{
			//virtual const char* get_name() const = 0;

			virtual uint get_vertices_count() const = 0;
			virtual const Vec3f* get_positions() const = 0;
			virtual const Vec2f* get_uvs() const = 0;
			virtual const Vec3f* get_normals() const = 0;

			virtual uint get_indices_count() const = 0;
			virtual const uint* get_indices() const = 0;

			virtual uint get_bones_count() const = 0;
			virtual Bone* get_bone(uint idx) const = 0;
		};
		
		struct Node
		{
			virtual const char* get_name() const = 0;

			virtual void get_transform(Vec3f& pos, Vec4f& quat, Vec3f& scale) const = 0;

			virtual Node* get_parent() const = 0;
			virtual uint get_children_count() const = 0;
			virtual Node* get_child(uint idx) const = 0;

			virtual int get_mesh_index() const = 0;
		};

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
			virtual Channel* get_channel(uint idx) const = 0;
		};

		struct Model
		{
			//virtual uint get_materials_count() const = 0;
			//virtual Material* get_material(uint idx) const = 0;
			//virtual int find_material(const char* name) const = 0;

			virtual uint get_meshes_count() const = 0;
			virtual Mesh* get_mesh(uint idx) const = 0;
			virtual int find_mesh(const char* name) const = 0;

			virtual Node* get_root() const = 0;

			virtual uint get_animations_count() const = 0;
			virtual Animation* get_animation(uint idx) const = 0;
			virtual int find_animation(const char* name) const = 0;

			virtual void save(const wchar_t* filename, const char* model_name = nullptr) const = 0;

			FLAME_GRAPHICS_EXPORTS static Model* get_standard(const char* name);
			FLAME_GRAPHICS_EXPORTS static Model* create(const wchar_t* filename);
		};
	}
}
