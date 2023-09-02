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

			void reset()
			{
				vertices.clear();
				faces.clear();
			}

			void init_as_cube(const vec3& extent)
			{
				vertices.resize(8);
				vertices[0] = vec3(-0.5f, -0.5f, +0.5f) * extent;
				vertices[1] = vec3(+0.5f, -0.5f, +0.5f) * extent;
				vertices[2] = vec3(+0.5f, +0.5f, +0.5f) * extent;
				vertices[3] = vec3(-0.5f, +0.5f, +0.5f) * extent;

				vertices[4] = vec3(+0.5f, -0.5f, -0.5f) * extent;
				vertices[5] = vec3(-0.5f, -0.5f, -0.5f) * extent;
				vertices[6] = vec3(-0.5f, +0.5f, -0.5f) * extent;
				vertices[7] = vec3(+0.5f, +0.5f, -0.5f) * extent;

				faces.resize(6);
				faces[0].corners.resize(4);
				faces[0].corners[0] = { .vertex_id = 0, .uv = vec2(0.f, 0.f) };
				faces[0].corners[1] = { .vertex_id = 1, .uv = vec2(1.f, 0.f) };
				faces[0].corners[2] = { .vertex_id = 2, .uv = vec2(1.f, 1.f) };
				faces[0].corners[3] = { .vertex_id = 3, .uv = vec2(0.f, 1.f) };
				faces[0].normal = vec3(+0.f, +0.f, +1.f);
				faces[1].corners.resize(4);
				faces[1].corners[0] = { .vertex_id = 4, .uv = vec2(0.f, 0.f) };
				faces[1].corners[1] = { .vertex_id = 5, .uv = vec2(1.f, 0.f) };
				faces[1].corners[2] = { .vertex_id = 6, .uv = vec2(1.f, 1.f) };
				faces[1].corners[3] = { .vertex_id = 7, .uv = vec2(0.f, 1.f) };
				faces[1].normal = vec3(+0.f, +0.f, -1.f);
				faces[2].corners.resize(4);
				faces[2].corners[0] = { .vertex_id = 6, .uv = vec2(0.f, 0.f) };
				faces[2].corners[1] = { .vertex_id = 5, .uv = vec2(1.f, 0.f) };
				faces[2].corners[2] = { .vertex_id = 0, .uv = vec2(1.f, 1.f) };
				faces[2].corners[3] = { .vertex_id = 3, .uv = vec2(0.f, 1.f) };
				faces[2].normal = vec3(-1.f, +0.f, +0.f);
				faces[3].corners.resize(4);
				faces[3].corners[0] = { .vertex_id = 4, .uv = vec2(0.f, 0.f) };
				faces[3].corners[1] = { .vertex_id = 7, .uv = vec2(1.f, 0.f) };
				faces[3].corners[2] = { .vertex_id = 2, .uv = vec2(1.f, 1.f) };
				faces[3].corners[3] = { .vertex_id = 1, .uv = vec2(0.f, 1.f) };
				faces[3].normal = vec3(+1.f, +0.f, +0.f);
				faces[4].corners.resize(4);
				faces[4].corners[0] = { .vertex_id = 3, .uv = vec2(0.f, 0.f) };
				faces[4].corners[1] = { .vertex_id = 2, .uv = vec2(1.f, 0.f) };
				faces[4].corners[2] = { .vertex_id = 7, .uv = vec2(1.f, 1.f) };
				faces[4].corners[3] = { .vertex_id = 6, .uv = vec2(0.f, 1.f) };
				faces[4].normal = vec3(+0.f, +1.f, +0.f);
				faces[5].corners.resize(4);
				faces[5].corners[0] = { .vertex_id = 5, .uv = vec2(0.f, 0.f) };
				faces[5].corners[1] = { .vertex_id = 4, .uv = vec2(1.f, 0.f) };
				faces[5].corners[2] = { .vertex_id = 1, .uv = vec2(1.f, 1.f) };
				faces[5].corners[3] = { .vertex_id = 0, .uv = vec2(0.f, 1.f) };
				faces[5].normal = vec3(+0.f, -1.f, +0.f);
			}

			void subdivide(ControlMesh& oth)
			{
				oth.reset();

				std::unordered_map<uint, uint> vertices_adjacent_face_count;
				std::unordered_map<std::pair<uint, uint>, uint, uint_pair_hasher> faces_adjacent;
				for (auto i = 0; i < faces.size(); i++)
				{
					auto& f = faces[i];

					uint v0 = f.corners[0].vertex_id;
					uint v1 = f.corners[1].vertex_id;
					uint v2 = f.corners[2].vertex_id;
					uint v3 = f.corners[3].vertex_id;

					vertices_adjacent_face_count[v0]++;
					vertices_adjacent_face_count[v1]++;
					vertices_adjacent_face_count[v2]++;
					vertices_adjacent_face_count[v3]++;

					faces_adjacent[{v1, v0}] = i;
					faces_adjacent[{v2, v1}] = i;
					faces_adjacent[{v3, v2}] = i;
					faces_adjacent[{v0, v3}] = i;
				}

				std::vector<vec3> face_points;
				face_points.resize(faces.size());

				for (auto i = 0; i < faces.size(); i++)
				{
					auto& f = faces[i];

					uint v0 = f.corners[0].vertex_id;
					uint v1 = f.corners[1].vertex_id;
					uint v2 = f.corners[2].vertex_id;
					uint v3 = f.corners[3].vertex_id;

					vec3 face_point = (vertices[v0] + vertices[v1] + vertices[v2] + vertices[v3]) * 0.25f;

					face_points[i] = face_point;
				}

				std::unordered_map<std::pair<uint, uint>, vec3, uint_pair_hasher> edge_points;

				for (auto i = 0; i < faces.size(); i++)
				{
					auto& f = faces[i];

					uint v0 = f.corners[0].vertex_id;
					uint v1 = f.corners[1].vertex_id;
					uint v2 = f.corners[2].vertex_id;
					uint v3 = f.corners[3].vertex_id;

					auto k01 = std::make_pair(v0, v1);
					auto k12 = std::make_pair(v1, v2);
					auto k23 = std::make_pair(v2, v3);
					auto k30 = std::make_pair(v3, v0);
					auto k03 = std::make_pair(v0, v3);
					auto k32 = std::make_pair(v3, v2);
					auto k21 = std::make_pair(v2, v1);
					auto k10 = std::make_pair(v1, v0);

					uint adjacent_f03 = faces_adjacent[k30];
					vec3 edge_point_03 = (face_points[i] + face_points[adjacent_f03] +
						vertices[v0] + vertices[v3]) * 0.25f;
					edge_points[k30] = edge_point_03;
					edge_points[k03] = edge_point_03;

					uint adjacent_f32 = faces_adjacent[k23];
					vec3 edge_point_32 = (face_points[i] + face_points[adjacent_f32] +
						vertices[v3] + vertices[v2]) * 0.25f;
					edge_points[k23] = edge_point_32;
					edge_points[k32] = edge_point_32;

					uint adjacent_f21 = faces_adjacent[k12];
					vec3 edge_point_21 = (face_points[i] + face_points[adjacent_f21] +
						vertices[v2] + vertices[v1]) * 0.25f;
					edge_points[k12] = edge_point_21;
					edge_points[k21] = edge_point_21;

					uint adjacent_f10 = faces_adjacent[k01];
					vec3 edge_point_10 = (face_points[i] + face_points[adjacent_f10] +
						vertices[v1] + vertices[v0]) * 0.25f;
					edge_points[k01] = edge_point_10;
					edge_points[k10] = edge_point_10;
				}

				int vertex_size = vertices.size();
				std::vector<vec3> vertex_points;
				vertex_points.resize(vertex_size);

				for (auto i = 0; i < faces.size(); i++)
				{
					auto& f = faces[i];

					uint v0 = f.corners[0].vertex_id;
					uint v1 = f.corners[1].vertex_id;
					uint v2 = f.corners[2].vertex_id;
					uint v3 = f.corners[3].vertex_id;

					uint n0 = vertices_adjacent_face_count[v0];
					uint n1 = vertices_adjacent_face_count[v1];
					uint n2 = vertices_adjacent_face_count[v2];
					uint n3 = vertices_adjacent_face_count[v3];

					vertex_points[v0] += face_points[i] / (float)n0;
					vertex_points[v0] += (vertices[v0] + vertices[v1]) * 0.5f / (float)n0;
					vertex_points[v0] += (vertices[v3] + vertices[v0]) * 0.5f / (float)n0;

					vertex_points[v1] += face_points[i] / (float)n1;
					vertex_points[v1] += (vertices[v0] + vertices[v1]) * 0.5f / (float)n1;
					vertex_points[v1] += (vertices[v1] + vertices[v2]) * 0.5f / (float)n1;

					vertex_points[v2] += face_points[i] / (float)n2;
					vertex_points[v2] += (vertices[v1] + vertices[v2]) * 0.5f / (float)n2;
					vertex_points[v2] += (vertices[v2] + vertices[v3]) * 0.5f / (float)n2;

					vertex_points[v3] += face_points[i] / (float)n3;
					vertex_points[v3] += (vertices[v3] + vertices[v0]) * 0.5f / (float)n3;
					vertex_points[v3] += (vertices[v2] + vertices[v3]) * 0.5f / (float)n3;
				}
				for (auto i = 0; i < vertex_size; i++)
				{
					uint n = vertices_adjacent_face_count[i];
					vertex_points[i] = (vertex_points[i] + (float)(n - 3) * vertices[i]) / (float)n;
				}

				//Quad
				//3           2  
				//
				//
				//
				//0           1

				//4 quads

				//3    e23    2  
				//
				//e30  f     e12
				//
				//0   e01     1

				uint vertex_ind = 0;

				std::vector<vec3> new_vertices;
				std::vector<Face> new_quads;
				std::unordered_map<std::pair<uint, uint>, uint, uint_pair_hasher> process_vertex;

				for (auto i = 0; i < faces.size(); i++)
				{
					auto& f = faces[i];

					auto ind_f = vertex_ind;

					vertex_ind++;

					new_vertices.emplace_back(face_points[i]);

					uint v0 = f.corners[0].vertex_id;
					uint v1 = f.corners[1].vertex_id;
					uint v2 = f.corners[2].vertex_id;
					uint v3 = f.corners[3].vertex_id;

					auto e01 = std::make_pair(v0, v1);
					auto e12 = std::make_pair(v1, v2);
					auto e23 = std::make_pair(v2, v3);
					auto e30 = std::make_pair(v3, v0);
					auto e10 = std::make_pair(v1, v0);
					auto e21 = std::make_pair(v2, v1);
					auto e32 = std::make_pair(v3, v2);
					auto e03 = std::make_pair(v0, v3);

					//check edge vertex
					uint ind_e01;
					uint ind_e12;
					uint ind_e23;
					uint ind_e30;

					if (process_vertex.find(e01) != process_vertex.end() || process_vertex.find(e10) != process_vertex.end())
					{
						if (process_vertex.find(e01) != process_vertex.end())
							ind_e01 = process_vertex[e01];
						else
							ind_e01 = process_vertex[e10];
					}
					else
					{
						ind_e01 = vertex_ind;

						auto e10 = std::make_pair(v1, v0);

						process_vertex[e01] = vertex_ind;

						if (edge_points.find(e01) != edge_points.end())
							new_vertices.emplace_back(edge_points[e01]);
						else
							new_vertices.emplace_back(edge_points[e10]);

						++vertex_ind;
					}

					if (process_vertex.find(e12) != process_vertex.end() || process_vertex.find(e21) != process_vertex.end())
					{
						if (process_vertex.find(e12) != process_vertex.end())
							ind_e12 = process_vertex[e12];
						else
							ind_e12 = process_vertex[e21];
					}
					else
					{
						ind_e12 = vertex_ind;

						auto e21 = std::make_pair(v2, v1);

						process_vertex[e12] = vertex_ind;

						if (edge_points.find(e12) != edge_points.end())
							new_vertices.emplace_back(edge_points[e12]);
						else
							new_vertices.emplace_back(edge_points[e21]);
						++vertex_ind;
					}

					if (process_vertex.find(e23) != process_vertex.end() || process_vertex.find(e32) != process_vertex.end())
					{
						if (process_vertex.find(e23) != process_vertex.end())
							ind_e23 = process_vertex[e23];
						else
							ind_e23 = process_vertex[e32];
					}
					else
					{
						ind_e23 = vertex_ind;

						auto e32 = std::make_pair(v3, v2);

						process_vertex[e23] = vertex_ind;

						if (edge_points.find(e23) != edge_points.end())
							new_vertices.emplace_back(edge_points[e23]);
						else
							new_vertices.emplace_back(edge_points[e32]);

						++vertex_ind;
					}

					if (process_vertex.find(e30) != process_vertex.end() || process_vertex.find(e03) != process_vertex.end())
					{
						if (process_vertex.find(e30) != process_vertex.end())
							ind_e30 = process_vertex[e30];
						else
							ind_e30 = process_vertex[e03];
					}
					else
					{
						ind_e30 = vertex_ind;

						auto e03 = std::make_pair(v0, v3);

						process_vertex[e30] = vertex_ind;

						if (edge_points.find(e30) != edge_points.end())
							new_vertices.emplace_back(edge_points[e30]);
						else
							new_vertices.emplace_back(edge_points[e03]);
						++vertex_ind;
					}

					//check point vertex
					uint ind_v0;
					uint ind_v1;
					uint ind_v2;
					uint ind_v3;

					auto p0 = std::make_pair(v0, 0xffffffff);
					auto p1 = std::make_pair(v1, 0xffffffff);
					auto p2 = std::make_pair(v2, 0xffffffff);
					auto p3 = std::make_pair(v3, 0xffffffff);

					if (process_vertex.find(p0) != process_vertex.end())
						ind_v0 = process_vertex[p0];
					else
					{
						new_vertices.emplace_back(vertex_points[v0]);
						process_vertex[p0] = vertex_ind;
						ind_v0 = vertex_ind++;
					}

					if (process_vertex.find(p1) != process_vertex.end())
						ind_v1 = process_vertex[p1];
					else
					{
						new_vertices.emplace_back(vertex_points[v1]);
						process_vertex[p1] = vertex_ind;
						ind_v1 = vertex_ind++;
					}

					if (process_vertex.find(p2) != process_vertex.end())
						ind_v2 = process_vertex[p2];
					else
					{
						new_vertices.emplace_back(vertex_points[v2]);
						process_vertex[p2] = vertex_ind;
						ind_v2 = vertex_ind++;
					}

					if (process_vertex.find(p3) != process_vertex.end())
						ind_v3 = process_vertex[p3];
					else
					{
						new_vertices.emplace_back(vertex_points[v3]);
						process_vertex[p3] = vertex_ind;
						ind_v3 = vertex_ind++;
					}

					//3    e23    2  
					//
					//e30  f     e12
					//
					//0   e01     1
					//Final Add Quads

					auto uv_v0 = f.corners[0].uv;
					auto uv_v1 = f.corners[0].uv;
					auto uv_v2 = f.corners[0].uv;
					auto uv_v3 = f.corners[0].uv;

					auto uv_e01 = (uv_v0 + uv_v1) * 0.5f;
					auto uv_e12 = (uv_v1 + uv_v2) * 0.5f;
					auto uv_e23 = (uv_v2 + uv_v3) * 0.5f;
					auto uv_e30 = (uv_v3 + uv_v0) * 0.5f;

					auto uv_f = (uv_v0 + uv_v1 + uv_v2 + uv_v3) * 0.25f;

					Face quad1;
					quad1.corners.resize(4);
					quad1.corners[0] = { .vertex_id = ind_f, .uv = uv_f };
					quad1.corners[1] = { .vertex_id = ind_e01, .uv = uv_e01 };
					quad1.corners[2] = { .vertex_id = ind_v0, .uv = uv_v0 };
					quad1.corners[3] = { .vertex_id = ind_e30, .uv = uv_e30 };
					quad1.normal = normalize(cross(new_vertices[ind_f] - new_vertices[ind_e30], 
						new_vertices[ind_f] - new_vertices[ind_e01]));

					Face quad2;
					quad2.corners.resize(4);
					quad2.corners[0] = { .vertex_id = ind_f, .uv = uv_f };
					quad2.corners[1] = { .vertex_id = ind_e12, .uv = uv_e12 };
					quad2.corners[2] = { .vertex_id = ind_v1, .uv = uv_v1 };
					quad2.corners[3] = { .vertex_id = ind_e01, .uv = uv_e01 };
					quad2.normal = normalize(cross(new_vertices[ind_f] - new_vertices[ind_e01],
						new_vertices[ind_f] - new_vertices[ind_e12]));

					Face quad3;
					quad3.corners.resize(4);
					quad3.corners[0] = { .vertex_id = ind_f, .uv = uv_f };
					quad3.corners[1] = { .vertex_id = ind_e23, .uv = uv_e23 };
					quad3.corners[2] = { .vertex_id = ind_v2, .uv = uv_v2 };
					quad3.corners[3] = { .vertex_id = ind_e12, .uv = uv_e12 };
					quad3.normal = normalize(cross(new_vertices[ind_f] - new_vertices[ind_e12],
						new_vertices[ind_f] - new_vertices[ind_e23]));

					Face quad4;
					quad4.corners.resize(4);
					quad4.corners[0] = { .vertex_id = ind_f, .uv = uv_f };
					quad4.corners[1] = { .vertex_id = ind_e30, .uv = uv_e30 };
					quad4.corners[2] = { .vertex_id = ind_v3, .uv = uv_v3 };
					quad4.corners[3] = { .vertex_id = ind_e23, .uv = uv_e23 };
					quad4.normal = normalize(cross(new_vertices[ind_f] - new_vertices[ind_e23],
						new_vertices[ind_f] - new_vertices[ind_e30]));

					new_quads.emplace_back(quad1);
					new_quads.emplace_back(quad2);
					new_quads.emplace_back(quad3);
					new_quads.emplace_back(quad4);
				}

				oth.vertices = std::move(new_vertices);
				oth.faces = std::move(new_quads);
			}

			void displace(ControlMesh& oth, Texture* ptexture)
			{
				std::vector<std::vector<uint>> vertices_adjacent_faces(vertices.size());

				for (auto i = 0; i < faces.size(); i++)
				{
					auto& f = faces[i];

					uint v0 = f.corners[0].vertex_id;
					uint v1 = f.corners[1].vertex_id;
					uint v2 = f.corners[2].vertex_id;
					uint v3 = f.corners[3].vertex_id;

					vertices_adjacent_faces[v0].push_back(i);
					vertices_adjacent_faces[v1].push_back(i);
					vertices_adjacent_faces[v2].push_back(i);
					vertices_adjacent_faces[v3].push_back(i);
				}

				oth.reset();
				oth.vertices = vertices;
				oth.faces = faces;

				for (auto i = 0; i < vertices.size(); i++)
				{
					vec3 normal(0.f);
					for (auto fi : vertices_adjacent_faces[i])
						normal += faces[fi].normal;
					normal = normalize(normal);

					auto v = vertices[i];
					float disp = 0.f;
					if (ptexture)
					{
						switch (ptexture->type)
						{
						case TextureImage:
							break;
						case TexturePerlin:
							break;
						case TextureVoronoi:
							disp = triplanar_sample<float>(normal, v, [ptexture](const vec2& uv) {
								return voronoi_noise(uv * ptexture->scale);
							});
							break;
						}
					}
					v += normal * disp;
					oth.vertices[i] = v;
				}

				for (auto& f : oth.faces)
				{
					f.normal = normalize(cross(oth.vertices[f.corners[3].vertex_id] - oth.vertices[f.corners[0].vertex_id],
						oth.vertices[f.corners[1].vertex_id] - oth.vertices[f.corners[0].vertex_id]));
				}
			}

			void decimate(float ratio)
			{

			}

			void convert_to_mesh(Mesh& mesh)
			{
				mesh.reset();
				for (auto& f : faces)
				{
					if (f.corners.size() == 4)
					{
						auto vtx_off = mesh.positions.size();
						for (auto i = 0; i < 4; i++)
						{
							mesh.positions.push_back(vertices[f.corners[i].vertex_id]);
							mesh.uvs.push_back(f.corners[i].uv);
							mesh.normals.push_back(f.normal);
						}

						mesh.indices.push_back(vtx_off + 0);
						mesh.indices.push_back(vtx_off + 2);
						mesh.indices.push_back(vtx_off + 1);
						mesh.indices.push_back(vtx_off + 0);
						mesh.indices.push_back(vtx_off + 3);
						mesh.indices.push_back(vtx_off + 2);
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
