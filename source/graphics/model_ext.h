#pragma once

#include "model.h"
#include "texture.h"
#include "noise.h"

namespace flame
{
	namespace graphics
	{
		struct ControlMesh
		{
			struct Corner
			{
				uint vertex_id;
				vec2 uv;
			};

			struct Face
			{
				std::vector<Corner> corners;
				vec3 normal;
			};

			std::vector<vec3> vertices;
			std::vector<Face> faces;
			cvec4 color = cvec4(255);

			inline void reset()
			{
				vertices.clear();
				faces.clear();
			}

			void init_as_cube(const vec3& extent);
			void init_as_cone(float radius, float depth, uint vertices_number);

			void subdivide_CatmullClark(ControlMesh& oth);
			void subdivide_Loop(ControlMesh& oth);
			void loop_cut(ControlMesh& oth, const std::vector<Plane>& planes);
			void decimate(float ratio);

			void displace(ControlMesh& oth, Texture* ptexture);

			inline void convert_to_mesh(Mesh& mesh)
			{
				mesh.reset();
				for (auto& f : faces)
				{
					auto face_vtx_off = mesh.positions.size();

					mesh.positions.push_back(vertices[f.corners[0].vertex_id]);
					mesh.uvs.push_back(f.corners[0].uv);
					mesh.normals.push_back(f.normal);

					mesh.positions.push_back(vertices[f.corners[1].vertex_id]);
					mesh.uvs.push_back(f.corners[1].uv);
					mesh.normals.push_back(f.normal);

					auto vtx_off = 2;
					for (auto i = 0; i < f.corners.size() - 2; i++)
					{
						mesh.positions.push_back(vertices[f.corners[i + 2].vertex_id]);
						mesh.uvs.push_back(f.corners[i + 2].uv);
						mesh.normals.push_back(f.normal);

						mesh.indices.push_back(face_vtx_off);
						mesh.indices.push_back(face_vtx_off + vtx_off - 1);
						mesh.indices.push_back(face_vtx_off + vtx_off);

						vtx_off++;
					}
				}
				mesh.calc_bounds();
			}
		};

		inline void mesh_add_cube(Mesh& mesh, const vec3& extent, const vec3& center, const mat3& rotation)
		{
			mesh.positions.push_back(rotation * vec3(-0.5f, +0.5f, +0.5f) * extent + center);
			mesh.positions.push_back(rotation * vec3(+0.5f, +0.5f, +0.5f) * extent + center);
			mesh.positions.push_back(rotation * vec3(+0.5f, -0.5f, +0.5f) * extent + center);
			mesh.positions.push_back(rotation * vec3(-0.5f, -0.5f, +0.5f) * extent + center);
			mesh.normals.push_back(vec3(+0.f, +0.f, +1.f));
			mesh.normals.push_back(vec3(+0.f, +0.f, +1.f));
			mesh.normals.push_back(vec3(+0.f, +0.f, +1.f));
			mesh.normals.push_back(vec3(+0.f, +0.f, +1.f));
			mesh.indices.push_back(0); mesh.indices.push_back(2); mesh.indices.push_back(1);
			mesh.indices.push_back(0); mesh.indices.push_back(3); mesh.indices.push_back(2);

			mesh.positions.push_back(rotation * vec3(-0.5f, +0.5f, -0.5f) * extent + center);
			mesh.positions.push_back(rotation * vec3(+0.5f, +0.5f, -0.5f) * extent + center);
			mesh.positions.push_back(rotation * vec3(+0.5f, -0.5f, -0.5f) * extent + center);
			mesh.positions.push_back(rotation * vec3(-0.5f, -0.5f, -0.5f) * extent + center);
			mesh.normals.push_back(vec3(+0.f, +0.f, -1.f));
			mesh.normals.push_back(vec3(+0.f, +0.f, -1.f));
			mesh.normals.push_back(vec3(+0.f, +0.f, -1.f));
			mesh.normals.push_back(vec3(+0.f, +0.f, -1.f));
			mesh.indices.push_back(5); mesh.indices.push_back(7); mesh.indices.push_back(4);
			mesh.indices.push_back(5); mesh.indices.push_back(6); mesh.indices.push_back(7);

			mesh.positions.push_back(rotation * vec3(-0.5f, +0.5f, -0.5f) * extent + center);
			mesh.positions.push_back(rotation * vec3(+0.5f, +0.5f, -0.5f) * extent + center);
			mesh.positions.push_back(rotation * vec3(+0.5f, +0.5f, +0.5f) * extent + center);
			mesh.positions.push_back(rotation * vec3(-0.5f, +0.5f, +0.5f) * extent + center);
			mesh.normals.push_back(vec3(+0.f, +1.f, +0.f));
			mesh.normals.push_back(vec3(+0.f, +1.f, +0.f));
			mesh.normals.push_back(vec3(+0.f, +1.f, +0.f));
			mesh.normals.push_back(vec3(+0.f, +1.f, +0.f));
			mesh.indices.push_back(8); mesh.indices.push_back(10); mesh.indices.push_back(9);
			mesh.indices.push_back(8); mesh.indices.push_back(11); mesh.indices.push_back(10);

			mesh.positions.push_back(rotation * vec3(-0.5f, -0.5f, -0.5f) * extent + center);
			mesh.positions.push_back(rotation * vec3(+0.5f, -0.5f, -0.5f) * extent + center);
			mesh.positions.push_back(rotation * vec3(+0.5f, -0.5f, +0.5f) * extent + center);
			mesh.positions.push_back(rotation * vec3(-0.5f, -0.5f, +0.5f) * extent + center);
			mesh.normals.push_back(vec3(+0.f, -1.f, +0.f));
			mesh.normals.push_back(vec3(+0.f, -1.f, +0.f));
			mesh.normals.push_back(vec3(+0.f, -1.f, +0.f));
			mesh.normals.push_back(vec3(+0.f, -1.f, +0.f));
			mesh.indices.push_back(15); mesh.indices.push_back(13); mesh.indices.push_back(14);
			mesh.indices.push_back(15); mesh.indices.push_back(12); mesh.indices.push_back(13);

			mesh.positions.push_back(rotation * vec3(-0.5f, +0.5f, -0.5f) * extent + center);
			mesh.positions.push_back(rotation * vec3(-0.5f, +0.5f, +0.5f) * extent + center);
			mesh.positions.push_back(rotation * vec3(-0.5f, -0.5f, +0.5f) * extent + center);
			mesh.positions.push_back(rotation * vec3(-0.5f, -0.5f, -0.5f) * extent + center);
			mesh.normals.push_back(vec3(-1.f, +0.f, +0.f));
			mesh.normals.push_back(vec3(-1.f, +0.f, +0.f));
			mesh.normals.push_back(vec3(-1.f, +0.f, +0.f));
			mesh.normals.push_back(vec3(-1.f, +0.f, +0.f));
			mesh.indices.push_back(16); mesh.indices.push_back(18); mesh.indices.push_back(17);
			mesh.indices.push_back(16); mesh.indices.push_back(19); mesh.indices.push_back(18);

			mesh.positions.push_back(rotation * vec3(+0.5f, +0.5f, -0.5f) * extent + center);
			mesh.positions.push_back(rotation * vec3(+0.5f, +0.5f, +0.5f) * extent + center);
			mesh.positions.push_back(rotation * vec3(+0.5f, -0.5f, +0.5f) * extent + center);
			mesh.positions.push_back(rotation * vec3(+0.5f, -0.5f, -0.5f) * extent + center);
			mesh.normals.push_back(vec3(+1.f, +0.f, +0.f));
			mesh.normals.push_back(vec3(+1.f, +0.f, +0.f));
			mesh.normals.push_back(vec3(+1.f, +0.f, +0.f));
			mesh.normals.push_back(vec3(+1.f, +0.f, +0.f));
			mesh.indices.push_back(21); mesh.indices.push_back(23); mesh.indices.push_back(20);
			mesh.indices.push_back(21); mesh.indices.push_back(22); mesh.indices.push_back(23);
		}

		inline void mesh_add_sphere(Mesh& mesh, float radius, uint horiSubdiv, uint vertSubdiv, const vec3& center, const mat3& rotation)
		{
			std::vector<std::vector<int>> staging_indices;
			staging_indices.resize(horiSubdiv + 1);

			for (int level = 1; level < horiSubdiv; level++)
			{
				for (auto i = 0; i < vertSubdiv; i++)
				{
					auto radian = radians((level * 180.f / horiSubdiv - 90.f));
					auto ring_radius = cos(radian) * radius;
					auto height = sin(radian) * radius;
					auto ang = radians((i * 360.f / vertSubdiv));
					staging_indices[level].push_back(mesh.positions.size());
					auto p = rotation * vec3(cos(ang) * ring_radius, height, sin(ang) * ring_radius);
					mesh.normals.push_back(p);
					mesh.positions.push_back(p + center);
				}
			}

			{
				staging_indices[0].push_back(mesh.positions.size());
				auto p = rotation * vec3(0.f, -radius, 0.f);
				mesh.normals.push_back(p);
				mesh.positions.push_back(p + center);
			}

			{
				staging_indices[horiSubdiv].push_back(mesh.positions.size());
				auto p = rotation * vec3(0.f, radius, 0.f);
				mesh.normals.push_back(p);
				mesh.positions.push_back(p + center);
			}

			for (int level = 0; level < horiSubdiv; level++)
			{
				if (level == 0)
				{
					for (auto i = 0; i < vertSubdiv; i++)
					{
						auto ii = i + 1; if (ii == vertSubdiv) ii = 0;

						mesh.indices.push_back(staging_indices[0][0]);
						mesh.indices.push_back(staging_indices[1][i]);
						mesh.indices.push_back(staging_indices[1][ii]);
					}
				}
				else if (level == horiSubdiv - 1)
				{
					for (auto i = 0; i < vertSubdiv; i++)
					{
						auto ii = i + 1; if (ii == vertSubdiv) ii = 0;

						mesh.indices.push_back(staging_indices[horiSubdiv - 1][i]);
						mesh.indices.push_back(staging_indices[horiSubdiv][0]);
						mesh.indices.push_back(staging_indices[horiSubdiv - 1][ii]);
					}
				}
				else
				{
					for (auto i = 0; i < vertSubdiv; i++)
					{
						auto ii = i + 1; if (ii == vertSubdiv) ii = 0;

						mesh.indices.push_back(staging_indices[level][i]);
						mesh.indices.push_back(staging_indices[level + 1][i]);
						mesh.indices.push_back(staging_indices[level][ii]);

						mesh.indices.push_back(staging_indices[level][ii]);
						mesh.indices.push_back(staging_indices[level + 1][i]);
						mesh.indices.push_back(staging_indices[level + 1][ii]);
					}
				}
			}
		}

		inline void mesh_add_cylinder(Mesh& mesh, float radius, float height, uint subdiv, const vec3& center)
		{
			auto hf_height = height * 0.5f;
			// top cap
			auto top_center = vec3(0.f, +hf_height, 0.f);
			auto top_center_index = mesh.positions.size();
			mesh.positions.push_back(top_center + center);
			std::vector<uint> top_ring_indices(subdiv);
			for (auto i = 0; i < subdiv; i++)
			{
				top_ring_indices[i] = mesh.positions.size();
				auto rad = radians((float)i / subdiv * 360.f);
				mesh.positions.push_back(vec3(cos(rad) * radius, +hf_height, sin(rad) * radius) + center);
			}
			for (auto i = 0; i < subdiv; i++)
			{
				mesh.indices.push_back(top_ring_indices[i]);
				mesh.indices.push_back(top_center_index);
				mesh.indices.push_back(top_ring_indices[i + 1 >= subdiv ? 0 : i + 1]);
			}

			// bottom cap
			auto bottom_center = vec3(0.f, -hf_height, 0.f);
			auto bottom_center_index = mesh.positions.size();
			mesh.positions.push_back(bottom_center + center);
			std::vector<uint> bottom_ring_indices(subdiv);
			for (auto i = 0; i < subdiv; i++)
			{
				bottom_ring_indices[i] = mesh.positions.size();
				auto rad = radians((float)i / subdiv * 360.f);
				mesh.positions.push_back(vec3(cos(rad) * radius, -hf_height, sin(rad) * radius) + center);
			}
			for (auto i = 0; i < subdiv; i++)
			{
				mesh.indices.push_back(bottom_ring_indices[i + 1 >= subdiv ? 0 : i + 1]);
				mesh.indices.push_back(bottom_center_index);
				mesh.indices.push_back(bottom_ring_indices[i]);
			}

			// body
			for (auto i = 0; i < subdiv; i++)
			{
				mesh.indices.push_back(top_ring_indices[i]);
				mesh.indices.push_back(top_ring_indices[i + 1 >= subdiv ? 0 : i + 1]);
				mesh.indices.push_back(bottom_ring_indices[i]);

				mesh.indices.push_back(top_ring_indices[i + 1 >= subdiv ? 0 : i + 1]);
				mesh.indices.push_back(bottom_ring_indices[i + 1 >= subdiv ? 0 : i + 1]);
				mesh.indices.push_back(bottom_ring_indices[i]);
			}
		}
	}
}
