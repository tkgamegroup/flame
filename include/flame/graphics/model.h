#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct ModelVertex1
		{
			Vec3f pos = Vec3f(0.f);
			Vec2f uv = Vec2f(0.f);
			Vec3f normal = Vec3f(0.f);
		};
		
		struct ModelVertex2
		{
			Vec3f tangent = Vec3f(0.f);
			Vec3f bitangent = Vec3f(0.f);
		};

		struct ModelMesh
		{
			virtual uint get_vertices_count_1() const = 0;
			virtual const ModelVertex1* get_vertices_1() const = 0;
			virtual uint get_vertices_count_2() const = 0;
			virtual const ModelVertex2* get_vertices_2() const = 0;
			virtual uint get_indices_count() const = 0;
			virtual const uint* get_indices() const = 0;
		};
		
		struct ModelNode
		{
			virtual const char* get_name() const = 0;

			virtual void get_transform(Vec3f& pos, Vec4f& quat, Vec3f& scale) const = 0;

			virtual ModelNode* get_parent() const = 0;
			virtual uint get_children_count() const = 0;
			virtual ModelNode* get_child(uint idx) const = 0;

			virtual int get_mesh_index() const = 0;
		};

		enum StandardModel
		{
			StandardModelCube,
			StandardModelSphere,

			StandardModelCount
		};

		struct Model
		{
			virtual uint get_meshes_count() const = 0;
			virtual ModelMesh* get_mesh(uint idx) const = 0;

			virtual ModelNode* get_root() const = 0;

			// name - which material you want to substitute
			// filename - .fmtl file
			virtual void substitute_material(const char* name, const wchar_t* filename) = 0;

			virtual void save(const wchar_t* filename, const char* model_name = nullptr) const = 0;

			FLAME_GRAPHICS_EXPORTS static Model* get_standard(StandardModel m);
			FLAME_GRAPHICS_EXPORTS static Model* create(const wchar_t* filename);
		};
	}
}
