#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Bone
		{
			std::string name;
			mat4 offset_matrix;
		};

		struct Mesh
		{
			ModelPtr model;

			std::vector<vec3> positions;
			std::vector<vec2> uvs;
			std::vector<vec3> normals;
			std::vector<vec3> tangents;
			std::vector<cvec4> colors;
			std::vector<ivec4> bone_ids;
			std::vector<vec4> bone_weights;
			std::vector<uint> indices;

			AABB bounds;

			inline void add_vertices(uint n, vec3* _positions, vec3* _uvs, vec3* _normals)
			{
				auto b = positions.size();
				positions.resize(b + n);
				for (auto i = 0; i < n; i++)
				{
					auto& p = _positions[i];
					positions[b + i] = p;
				}
				if (_uvs)
				{
					uvs.resize(b + n);
					for (auto i = 0; i < n; i++)
						uvs[b + i] = _uvs[i];
				}
				if (_normals)
				{
					normals.resize(b + n);
					for (auto i = 0; i < n; i++)
						normals[b + i] = _normals[i];
				}
			}

			inline void add_indices(uint n, uint* _indices)
			{
				auto b = indices.size();
				indices.resize(b + n);
				for (auto i = 0; i < n; i++)
					indices[b + i] = _indices[i];
			}

			inline void calc_bounds()
			{
				bounds.reset();
				for (auto& p : positions)
					bounds.expand(p);
			}
		};

		struct Model
		{
			std::vector<Mesh> meshes;
			std::vector<Bone> bones;

			AABB bounds;

			std::filesystem::path filename;
			uint ref = 0;

			virtual ~Model() {}

			inline int find_bone(std::string_view name) 
			{
				for (auto i = 0; i < bones.size(); i++)
				{
					if (bones[i].name == name)
						return i;
				}
				return -1;
			};

			virtual void save(const std::filesystem::path& filename, bool binary = false) = 0;

			struct Create
			{
				virtual ModelPtr operator()() = 0;
			};
			FLAME_GRAPHICS_API static Create& create;

			struct Get
			{
				// could be "standard_<name>" to get standard models
				// standard models:
				//  Name		| Size
				//   plane		|  ext: 10
				//   cube		|  hf-ext: 0.5
				//   sphere		|  radius: 0.5
				//   cylinder	|  radius: 0.5, height: 1.0
				//   tri_prism	|  width: 1.0, height: 1.0, depth: 1.0
				virtual ModelPtr operator()(const std::filesystem::path& filename) = 0;
			};
			FLAME_GRAPHICS_API static Get& get;

			struct Release
			{
				virtual void operator()(ModelPtr model) = 0;
			};
			FLAME_GRAPHICS_API static Release& release;
		};

		FLAME_GRAPHICS_API void import_scene(const std::filesystem::path& filename, const std::filesystem::path& destination = L"", const vec3& rotation = vec3(0.f), float scaling = 1.f,
			bool only_animation = false);
	}
}
