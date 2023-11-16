#include "model_ext.h"

namespace flame
{
	namespace graphics
	{
		void ControlMesh::init_as_cube(const vec3& extent)
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

		void ControlMesh::init_as_cone(float radius, float depth, uint vertices_number)
		{
			auto top_vertex_id = (uint)vertices.size();
			vertices.push_back(vec3(0.f, +depth, 0.f));

			auto ring_vertices_id_start = (uint)vertices.size();
			vertices.resize(vertices.size() + vertices_number);
			auto ang = 2.f * pi<float>() / vertices_number;
			for (auto i = 0; i < vertices_number; i++)
				vertices[ring_vertices_id_start + i] = vec3(cos(ang * i), 0.f, sin(ang * i)) * radius;

			// cone faces
			for (auto i = 0; i < vertices_number; i++)
			{
				auto& f = faces.emplace_back();
				auto id0 = ring_vertices_id_start + i;
				auto id1 = i + 1 == vertices_number ? ring_vertices_id_start : id0 + 1;
				f.corners.resize(3);
				f.corners[0] = { .vertex_id = id0, .uv = vec2((float)i / vertices_number, 1.f) };
				f.corners[1] = { .vertex_id = top_vertex_id, .uv = vec2(0.f, 0.f) };
				f.corners[2] = { .vertex_id = id1, .uv = vec2((float)(i + 1) / vertices_number, 1.f) };
				f.normal = -normalize(cross(vertices[id1] - vertices[id0], vertices[top_vertex_id] - vertices[id0]));
			}

			auto& bottom_face = faces.emplace_back();
			bottom_face.corners.resize(vertices_number);
			for (auto i = 0; i < bottom_face.corners.size(); i++)
				bottom_face.corners[i] = { .vertex_id = ring_vertices_id_start + i, .uv = vec2((float)i / vertices_number, 1.f) };
			bottom_face.normal = vec3(0.f, -1.f, 0.f);
		}
		
		void ControlMesh::subdivide_CatmullClark(ControlMesh& oth)
		{
			oth.reset();

			std::unordered_map<uint, uint> vertices_adjacent_face_count;
			std::unordered_map<std::pair<uint, uint>, uint, uint_pair_hasher> faces_adjacent;
			for (auto i = 0; i < faces.size(); i++)
			{
				auto& f = faces[i];

				for (auto j = 0; j < f.corners.size(); j++)
				{
					uint v0 = f.corners[j].vertex_id;
					uint v1 = f.corners[j + 1 < f.corners.size() ? j + 1 : 0].vertex_id;
					vertices_adjacent_face_count[v0]++;
					faces_adjacent[{v1, v0}] = i;
				}
			}

			std::vector<vec3> face_points(faces.size());

			for (auto i = 0; i < faces.size(); i++)
			{
				auto& f = faces[i];

				vec3 face_point(0.f);
				for (auto& c : f.corners)
					face_point += vertices[c.vertex_id];
				face_point *= 1.f / (float)f.corners.size();
				face_points[i] = face_point;
			}

			std::unordered_map<std::pair<uint, uint>, vec3, uint_pair_hasher> edge_points;

			for (auto i = 0; i < faces.size(); i++)
			{
				auto& f = faces[i];
				if (f.corners.size() > 4)
					continue;

				if (f.corners.size() == 3)
				{
					uint v0 = f.corners[0].vertex_id;
					uint v1 = f.corners[1].vertex_id;
					uint v2 = f.corners[2].vertex_id;

					auto k01 = std::make_pair(v0, v1);
					auto k12 = std::make_pair(v1, v2);
					auto k20 = std::make_pair(v2, v0);
					auto k10 = std::make_pair(v1, v0);
					auto k21 = std::make_pair(v2, v1);
					auto k02 = std::make_pair(v0, v2);

					uint adjacent_f01 = faces_adjacent[k01];
					vec3 edge_point_01 = (face_points[i] + face_points[adjacent_f01] +
						vertices[v0] + vertices[v1]) * 0.25f;
					edge_points[k01] = edge_point_01;
					edge_points[k10] = edge_point_01;

					uint adjacent_f12 = faces_adjacent[k12];
					vec3 edge_point_12 = (face_points[i] + face_points[adjacent_f12] +
						vertices[v1] + vertices[v2]) * 0.25f;
					edge_points[k12] = edge_point_12;
					edge_points[k21] = edge_point_12;

					uint adjacent_f20 = faces_adjacent[k20];
					vec3 edge_point_20 = (face_points[i] + face_points[adjacent_f20] +
						vertices[v2] + vertices[v0]) * 0.25f;
					edge_points[k20] = edge_point_20;
					edge_points[k02] = edge_point_20;

				}
				else if (f.corners.size() == 4)
				{
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
			}

			int vertex_size = vertices.size();
			std::vector<vec3> vertex_points(vertex_size);
			for (auto& v : vertex_points) v = vec3(0.f);

			for (auto i = 0; i < faces.size(); i++)
			{
				auto& f = faces[i];
				if (f.corners.size() > 4)
					continue;

				if (f.corners.size() == 3)
				{
					uint v0 = f.corners[0].vertex_id;
					uint v1 = f.corners[1].vertex_id;
					uint v2 = f.corners[2].vertex_id;

					uint n0 = vertices_adjacent_face_count[v0];
					uint n1 = vertices_adjacent_face_count[v1];
					uint n2 = vertices_adjacent_face_count[v2];

					vertex_points[v0] += face_points[i] / (float)n0;
					vertex_points[v0] += (vertices[v0] + vertices[v1]) * 0.5f / (float)n0;
					vertex_points[v0] += (vertices[v2] + vertices[v0]) * 0.5f / (float)n0;

					vertex_points[v1] += face_points[i] / (float)n1;
					vertex_points[v1] += (vertices[v0] + vertices[v1]) * 0.5f / (float)n1;
					vertex_points[v1] += (vertices[v1] + vertices[v2]) * 0.5f / (float)n1;

					vertex_points[v2] += face_points[i] / (float)n2;
					vertex_points[v2] += (vertices[v1] + vertices[v2]) * 0.5f / (float)n2;
					vertex_points[v2] += (vertices[v2] + vertices[v0]) * 0.5f / (float)n2;
				}
				else if (f.corners.size() == 4)
				{
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
			}
			for (auto i = 0; i < vertex_size; i++)
			{
				uint n = vertices_adjacent_face_count[i];
				vertex_points[i] = (vertex_points[i] + (float)(n - 3) * vertices[i]) / (float)n;
			}

			uint vertex_idx = 0;

			std::vector<vec3> new_vertices;
			std::vector<Face> new_faces;
			std::unordered_map<std::pair<uint, uint>, uint, uint_pair_hasher> process_vertex;

			for (auto i = 0; i < faces.size(); i++)
			{
				auto& f = faces[i];
				if (f.corners.size() > 4)
					continue;

				auto idx_f = vertex_idx;
				vertex_idx++;
				new_vertices.emplace_back(face_points[i]);

				if (f.corners.size() == 3)
				{

					//Triangle
					//2
					//|\
					//| \
					//|  \
					//0---1

					//3 Quads
					//       2
					//      / \
					//     /   \
					//    /     \
					//  e20     e12
					//  /    f    \
					// /           \
					///             \ 
					//0-----e01-----1\

					uint v0 = f.corners[0].vertex_id;
					uint v1 = f.corners[1].vertex_id;
					uint v2 = f.corners[2].vertex_id;

					auto e01 = std::make_pair(v0, v1);
					auto e12 = std::make_pair(v1, v2);
					auto e20 = std::make_pair(v2, v0);
					auto e10 = std::make_pair(v1, v0);
					auto e21 = std::make_pair(v2, v1);
					auto e02 = std::make_pair(v0, v2);

					//check edge vertex
					uint idx_e01;
					uint idx_e12;
					uint idx_e20;

					if (process_vertex.find(e01) != process_vertex.end() || process_vertex.find(e10) != process_vertex.end())
					{
						if (process_vertex.find(e01) != process_vertex.end())
							idx_e01 = process_vertex[e01];
						else
							idx_e01 = process_vertex[e10];
					}
					else
					{
						idx_e01 = vertex_idx;

						auto e10 = std::make_pair(v1, v0);

						process_vertex[e01] = vertex_idx;

						if (edge_points.find(e01) != edge_points.end())
							new_vertices.emplace_back(edge_points[e01]);
						else
							new_vertices.emplace_back(edge_points[e10]);

						++vertex_idx;
					}

					if (process_vertex.find(e12) != process_vertex.end() || process_vertex.find(e21) != process_vertex.end())
					{
						if (process_vertex.find(e12) != process_vertex.end())
							idx_e12 = process_vertex[e12];
						else
							idx_e12 = process_vertex[e21];
					}
					else
					{
						idx_e12 = vertex_idx;

						auto e21 = std::make_pair(v2, v1);

						process_vertex[e12] = vertex_idx;

						if (edge_points.find(e12) != edge_points.end())
							new_vertices.emplace_back(edge_points[e12]);
						else
							new_vertices.emplace_back(edge_points[e21]);
						++vertex_idx;
					}

					if (process_vertex.find(e20) != process_vertex.end() || process_vertex.find(e02) != process_vertex.end())
					{
						if (process_vertex.find(e20) != process_vertex.end())
							idx_e20 = process_vertex[e20];
						else
							idx_e20 = process_vertex[e02];
					}
					else
					{
						idx_e20 = vertex_idx;

						auto e02 = std::make_pair(v0, v2);

						process_vertex[e20] = vertex_idx;

						if (edge_points.find(e20) != edge_points.end())
							new_vertices.emplace_back(edge_points[e20]);
						else
							new_vertices.emplace_back(edge_points[e02]);
						++vertex_idx;
					}

					//check point vertex
					uint idx_v0;
					uint idx_v1;
					uint idx_v2;

					auto p0 = std::make_pair(v0, 0xffffffff);
					auto p1 = std::make_pair(v1, 0xffffffff);
					auto p2 = std::make_pair(v2, 0xffffffff);

					if (process_vertex.find(p0) != process_vertex.end())
						idx_v0 = process_vertex[p0];
					else
					{
						new_vertices.emplace_back(vertex_points[v0]);
						process_vertex[p0] = vertex_idx;
						idx_v0 = vertex_idx++;
					}

					if (process_vertex.find(p1) != process_vertex.end())
						idx_v1 = process_vertex[p1];
					else
					{
						new_vertices.emplace_back(vertex_points[v1]);
						process_vertex[p1] = vertex_idx;
						idx_v1 = vertex_idx++;
					}

					if (process_vertex.find(p2) != process_vertex.end())
						idx_v2 = process_vertex[p2];
					else
					{
						new_vertices.emplace_back(vertex_points[v2]);
						process_vertex[p2] = vertex_idx;
						idx_v2 = vertex_idx++;
					}

					//       2
					//      / \
					//     /   \
					//    /     \
					//  e20     e12
					//  /    f    \
					// /           \
					///             \ 
					//0-----e01-----1\
					//Final Add Quads

					auto uv_v0 = f.corners[0].uv;
					auto uv_v1 = f.corners[0].uv;
					auto uv_v2 = f.corners[0].uv;

					auto uv_e01 = (uv_v0 + uv_v1) * 0.5f;
					auto uv_e12 = (uv_v1 + uv_v2) * 0.5f;
					auto uv_e20 = (uv_v2 + uv_v0) * 0.5f;

					auto uv_f = (uv_v0 + uv_v1 + uv_v2) * (1.f / 3.f);

					Face quad1;
					quad1.corners.resize(4);
					quad1.corners[0] = { .vertex_id = idx_f, .uv = uv_f };
					quad1.corners[1] = { .vertex_id = idx_e20, .uv = uv_e20 };
					quad1.corners[2] = { .vertex_id = idx_v0, .uv = uv_v0 };
					quad1.corners[3] = { .vertex_id = idx_e01, .uv = uv_e01 };
					quad1.normal = normalize(cross(new_vertices[idx_f] - new_vertices[idx_e01],
						new_vertices[idx_e20] - new_vertices[idx_f]));

					Face quad2;
					quad2.corners.resize(4);
					quad2.corners[0] = { .vertex_id = idx_f, .uv = uv_f };
					quad2.corners[1] = { .vertex_id = idx_e01, .uv = uv_e01 };
					quad2.corners[2] = { .vertex_id = idx_v1, .uv = uv_v1 };
					quad2.corners[3] = { .vertex_id = idx_e12, .uv = uv_e12 };
					quad2.normal = normalize(cross(new_vertices[idx_f] - new_vertices[idx_e12],
						new_vertices[idx_e01] - new_vertices[idx_f]));

					Face quad3;
					quad3.corners.resize(4);
					quad3.corners[0] = { .vertex_id = idx_f, .uv = uv_f };
					quad3.corners[1] = { .vertex_id = idx_e12, .uv = uv_e12 };
					quad3.corners[2] = { .vertex_id = idx_v2, .uv = uv_v2 };
					quad3.corners[3] = { .vertex_id = idx_e20, .uv = uv_e20 };
					quad3.normal = normalize(cross(new_vertices[idx_f] - new_vertices[idx_e20],
						new_vertices[idx_e12] - new_vertices[idx_f]));

					new_faces.emplace_back(quad1);
					new_faces.emplace_back(quad2);
					new_faces.emplace_back(quad3);
				}
				else if (f.corners.size() == 4)
				{
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
					uint idx_e01;
					uint idx_e12;
					uint idx_e23;
					uint idx_e30;

					if (process_vertex.find(e01) != process_vertex.end() || process_vertex.find(e10) != process_vertex.end())
					{
						if (process_vertex.find(e01) != process_vertex.end())
							idx_e01 = process_vertex[e01];
						else
							idx_e01 = process_vertex[e10];
					}
					else
					{
						idx_e01 = vertex_idx;

						auto e10 = std::make_pair(v1, v0);

						process_vertex[e01] = vertex_idx;

						if (edge_points.find(e01) != edge_points.end())
							new_vertices.emplace_back(edge_points[e01]);
						else
							new_vertices.emplace_back(edge_points[e10]);

						++vertex_idx;
					}

					if (process_vertex.find(e12) != process_vertex.end() || process_vertex.find(e21) != process_vertex.end())
					{
						if (process_vertex.find(e12) != process_vertex.end())
							idx_e12 = process_vertex[e12];
						else
							idx_e12 = process_vertex[e21];
					}
					else
					{
						idx_e12 = vertex_idx;

						auto e21 = std::make_pair(v2, v1);

						process_vertex[e12] = vertex_idx;

						if (edge_points.find(e12) != edge_points.end())
							new_vertices.emplace_back(edge_points[e12]);
						else
							new_vertices.emplace_back(edge_points[e21]);
						++vertex_idx;
					}

					if (process_vertex.find(e23) != process_vertex.end() || process_vertex.find(e32) != process_vertex.end())
					{
						if (process_vertex.find(e23) != process_vertex.end())
							idx_e23 = process_vertex[e23];
						else
							idx_e23 = process_vertex[e32];
					}
					else
					{
						idx_e23 = vertex_idx;

						auto e32 = std::make_pair(v3, v2);

						process_vertex[e23] = vertex_idx;

						if (edge_points.find(e23) != edge_points.end())
							new_vertices.emplace_back(edge_points[e23]);
						else
							new_vertices.emplace_back(edge_points[e32]);

						++vertex_idx;
					}

					if (process_vertex.find(e30) != process_vertex.end() || process_vertex.find(e03) != process_vertex.end())
					{
						if (process_vertex.find(e30) != process_vertex.end())
							idx_e30 = process_vertex[e30];
						else
							idx_e30 = process_vertex[e03];
					}
					else
					{
						idx_e30 = vertex_idx;

						auto e03 = std::make_pair(v0, v3);

						process_vertex[e30] = vertex_idx;

						if (edge_points.find(e30) != edge_points.end())
							new_vertices.emplace_back(edge_points[e30]);
						else
							new_vertices.emplace_back(edge_points[e03]);
						++vertex_idx;
					}

					//check point vertex
					uint idx_v0;
					uint idx_v1;
					uint idx_v2;
					uint idx_v3;

					auto p0 = std::make_pair(v0, 0xffffffff);
					auto p1 = std::make_pair(v1, 0xffffffff);
					auto p2 = std::make_pair(v2, 0xffffffff);
					auto p3 = std::make_pair(v3, 0xffffffff);

					if (process_vertex.find(p0) != process_vertex.end())
						idx_v0 = process_vertex[p0];
					else
					{
						new_vertices.emplace_back(vertex_points[v0]);
						process_vertex[p0] = vertex_idx;
						idx_v0 = vertex_idx++;
					}

					if (process_vertex.find(p1) != process_vertex.end())
						idx_v1 = process_vertex[p1];
					else
					{
						new_vertices.emplace_back(vertex_points[v1]);
						process_vertex[p1] = vertex_idx;
						idx_v1 = vertex_idx++;
					}

					if (process_vertex.find(p2) != process_vertex.end())
						idx_v2 = process_vertex[p2];
					else
					{
						new_vertices.emplace_back(vertex_points[v2]);
						process_vertex[p2] = vertex_idx;
						idx_v2 = vertex_idx++;
					}

					if (process_vertex.find(p3) != process_vertex.end())
						idx_v3 = process_vertex[p3];
					else
					{
						new_vertices.emplace_back(vertex_points[v3]);
						process_vertex[p3] = vertex_idx;
						idx_v3 = vertex_idx++;
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
					quad1.corners[0] = { .vertex_id = idx_f, .uv = uv_f };
					quad1.corners[1] = { .vertex_id = idx_e01, .uv = uv_e01 };
					quad1.corners[2] = { .vertex_id = idx_v0, .uv = uv_v0 };
					quad1.corners[3] = { .vertex_id = idx_e30, .uv = uv_e30 };
					quad1.normal = normalize(cross(new_vertices[idx_f] - new_vertices[idx_e30],
						new_vertices[idx_f] - new_vertices[idx_e01]));

					Face quad2;
					quad2.corners.resize(4);
					quad2.corners[0] = { .vertex_id = idx_f, .uv = uv_f };
					quad2.corners[1] = { .vertex_id = idx_e12, .uv = uv_e12 };
					quad2.corners[2] = { .vertex_id = idx_v1, .uv = uv_v1 };
					quad2.corners[3] = { .vertex_id = idx_e01, .uv = uv_e01 };
					quad2.normal = normalize(cross(new_vertices[idx_f] - new_vertices[idx_e01],
						new_vertices[idx_f] - new_vertices[idx_e12]));

					Face quad3;
					quad3.corners.resize(4);
					quad3.corners[0] = { .vertex_id = idx_f, .uv = uv_f };
					quad3.corners[1] = { .vertex_id = idx_e23, .uv = uv_e23 };
					quad3.corners[2] = { .vertex_id = idx_v2, .uv = uv_v2 };
					quad3.corners[3] = { .vertex_id = idx_e12, .uv = uv_e12 };
					quad3.normal = normalize(cross(new_vertices[idx_f] - new_vertices[idx_e12],
						new_vertices[idx_f] - new_vertices[idx_e23]));

					Face quad4;
					quad4.corners.resize(4);
					quad4.corners[0] = { .vertex_id = idx_f, .uv = uv_f };
					quad4.corners[1] = { .vertex_id = idx_e30, .uv = uv_e30 };
					quad4.corners[2] = { .vertex_id = idx_v3, .uv = uv_v3 };
					quad4.corners[3] = { .vertex_id = idx_e23, .uv = uv_e23 };
					quad4.normal = normalize(cross(new_vertices[idx_f] - new_vertices[idx_e23],
						new_vertices[idx_f] - new_vertices[idx_e30]));

					new_faces.emplace_back(quad1);
					new_faces.emplace_back(quad2);
					new_faces.emplace_back(quad3);
					new_faces.emplace_back(quad4);
				}
			}

			oth.vertices = std::move(new_vertices);
			oth.faces = std::move(new_faces);
			oth.color = color;
		}

		void ControlMesh::subdivide_Loop(ControlMesh& oth)
		{

		}

		void ControlMesh::loop_cut(ControlMesh& oth, const std::vector<Plane>& planes)
		{
			oth.reset();

			auto new_vertices = vertices;
			std::vector<Face> new_faces;
			uint vertex_idx = new_vertices.size();

			for (auto& f : faces)
			{
				auto edge_count = f.corners.size();

				struct CutResult
				{
					struct CutPoint
					{
						uint edge_idx;
						uint new_vertex_id;
						vec2 uv;
					};
					std::vector<CutPoint> points;
				};
				std::vector<CutResult> cut_results;
				cut_results.resize(planes.size());

				for (auto i = 0; i < planes.size(); i++)
				{
					for (auto j = 0; j < edge_count; j++)
					{
						auto next = j + 1; if (next >= edge_count) next = 0;

						auto edge_v0 = vertices[f.corners[j].vertex_id];
						auto edge_v1 = vertices[f.corners[next].vertex_id];

						vec3 intersected; float t;
						if (planes[i].intersects(edge_v0, edge_v1, &intersected, &t))
						{
							new_vertices.push_back(intersected);
							auto& pt = cut_results[i].points.emplace_back();
							pt.edge_idx = j;
							pt.new_vertex_id = vertex_idx;
							pt.uv = mix(f.corners[j].uv, f.corners[next].uv, t);
							vertex_idx++;
						}
					}
				}

				auto is_back_face = f.normal.z < 0.f;
				//if (f.normal.z < 0.f)
				//{
				//	for (auto& cr : cut_results)
				//	{
				//		if (cr.points.size() == 2)
				//			std::swap(cr.points[0], cr.points[1]);
				//	}
				//}

				for (auto i = 0; i < planes.size(); i++)
				{
					auto& res = cut_results[i];
					if (res.points.size() < 2)
					{
						new_faces.push_back(f);
						continue;
					}

					auto add_original_edges = [&](Face& new_face, uint start, uint end) {
						if (start == end)
							return;
						for (auto i = (start + 1) % edge_count; i != end; i = (i + 1) % edge_count)
							new_face.corners.push_back(f.corners[i]);
					};

					if (i == 0 || cut_results[i - 1].points.size() < 2)
					{
						Face new_face;
						new_face.normal = f.normal;
						new_face.corners.push_back({ res.points[0].new_vertex_id, res.points[0].uv });
						add_original_edges(new_face, res.points[0].edge_idx, res.points[1].edge_idx);
						new_face.corners.push_back({ res.points[1].new_vertex_id, res.points[1].uv });
						new_faces.push_back(new_face);
					}
					else
					{
						Face new_face;
						new_face.normal = f.normal;
						auto& last_res = cut_results[i - 1];
						new_face.corners.push_back({ res.points[0].new_vertex_id, res.points[0].uv });
						add_original_edges(new_face, res.points[0].edge_idx, last_res.points[0].edge_idx);
						new_face.corners.push_back({ last_res.points[0].new_vertex_id, last_res.points[0].uv });
						new_face.corners.push_back({ last_res.points[1].new_vertex_id, last_res.points[1].uv });
						add_original_edges(new_face, last_res.points[1].edge_idx, res.points[1].edge_idx);
						new_face.corners.push_back({ res.points[1].new_vertex_id, res.points[1].uv });
						new_faces.push_back(new_face);
					}

					if (i == planes.size() - 1) // the last plane also generate another face
					{
						Face new_face;
						new_face.normal = f.normal;
						new_face.corners.push_back({ res.points[1].new_vertex_id, res.points[1].uv });
						add_original_edges(new_face, res.points[1].edge_idx, res.points[0].edge_idx);
						new_face.corners.push_back({ res.points[0].new_vertex_id, res.points[0].uv });
						new_faces.push_back(new_face);
					}
				}

			}

			oth.vertices = std::move(new_vertices);
			oth.faces = std::move(new_faces);
			oth.color = color;
		}

		void ControlMesh::decimate(float ratio)
		{

		}

		void ControlMesh::displace(ControlMesh& oth, Texture* ptexture)
		{
			std::vector<std::vector<uint>> vertices_adjacent_faces(vertices.size());

			for (auto i = 0; i < faces.size(); i++)
			{
				auto& f = faces[i];
				for (auto& v : f.corners)
					vertices_adjacent_faces[v.vertex_id].push_back(i);
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
				f.normal = normalize(cross(oth.vertices[f.corners[1].vertex_id] - oth.vertices[f.corners[0].vertex_id],
					oth.vertices[f.corners[2].vertex_id] - oth.vertices[f.corners[1].vertex_id]));
			}
		}
	}
}
