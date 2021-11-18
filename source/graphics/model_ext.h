#pragma once

#include "model.h"

namespace flame
{
	namespace graphics
	{
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
				for (int i = 0; i < vertSubdiv; i++)
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
					for (int i = 0; i < vertSubdiv; i++)
					{
						auto ii = i + 1; if (ii == vertSubdiv) ii = 0;

						mesh.indices.push_back(staging_indices[0][0]);
						mesh.indices.push_back(staging_indices[1][i]);
						mesh.indices.push_back(staging_indices[1][ii]);
					}
				}
				else if (level == horiSubdiv - 1)
				{
					for (int i = 0; i < vertSubdiv; i++)
					{
						auto ii = i + 1; if (ii == vertSubdiv) ii = 0;

						mesh.indices.push_back(staging_indices[horiSubdiv - 1][i]);
						mesh.indices.push_back(staging_indices[horiSubdiv][0]);
						mesh.indices.push_back(staging_indices[horiSubdiv - 1][ii]);
					}
				}
				else
				{
					for (int i = 0; i < vertSubdiv; i++)
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
	}
}
