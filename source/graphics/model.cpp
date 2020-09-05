#include <flame/serialize.h>
#include <flame/foundation/foundation.h>
#include "model_private.h"

#ifdef USE_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

#include <functional>

namespace flame
{
	namespace graphics
	{
		void ModelMeshPrivate::set_vertices_p(const std::initializer_list<float>& v)
		{
			assert(v.size() % 3 == 0);
			vertices_1.resize(v.size() / 3);
			for (auto i = 0; i < vertices_1.size(); i++)
			{
				vertices_1[i].pos.x() = v.begin()[i * 3 + 0];
				vertices_1[i].pos.y() = v.begin()[i * 3 + 1];
				vertices_1[i].pos.z() = v.begin()[i * 3 + 2];
			}
		}

		void ModelMeshPrivate::set_vertices_pn(const std::initializer_list<float>& v)
		{
			assert(v.size() % 6 == 0);
			vertices_1.resize(v.size() / 6);
			for (auto i = 0; i < vertices_1.size(); i++)
			{
				vertices_1[i].pos.x() = v.begin()[i * 6 + 0];
				vertices_1[i].pos.y() = v.begin()[i * 6 + 1];
				vertices_1[i].pos.z() = v.begin()[i * 6 + 2];
				vertices_1[i].normal.x() = v.begin()[i * 6 + 3];
				vertices_1[i].normal.y() = v.begin()[i * 6 + 4];
				vertices_1[i].normal.z() = v.begin()[i * 6 + 5];
			}
		}

		void ModelMeshPrivate::set_vertices(uint number, Vec3f* poses, Vec3f* uvs, Vec3f* normals)
		{
			vertices_1.resize(number);
			if (poses)
			{
				for (auto i = 0; i < number; i++)
					vertices_1[i].pos = poses[i];
			}
			if (uvs)
			{
				for (auto i = 0; i < number; i++)
					vertices_1[i].uv = uvs[i];
			}
			if (normals)
			{
				for (auto i = 0; i < number; i++)
					vertices_1[i].normal = normals[i];
			}
		}

		void ModelMeshPrivate::set_indices(const std::initializer_list<uint>& v)
		{
			indices.resize(v.size());
			for (auto i = 0; i < indices.size(); i++)
				indices[i] = v.begin()[i];
		}

		void ModelMeshPrivate::set_indices(uint number, uint* _indices)
		{
			indices.resize(number);
			for (auto i = 0; i < number; i++)
				indices[i] = _indices[i];
		}

		void ModelMeshPrivate::add_sphere(float radius, uint horiSubdiv, uint vertSubdiv, const Vec3f& center, const Mat3f& rotation)
		{
			std::vector<std::vector<int>> staging_indices;
			staging_indices.resize(horiSubdiv + 1);

			for (int level = 1; level < horiSubdiv; level++)
			{
				for (int i = 0; i < vertSubdiv; i++)
				{
					auto radian = (level * 180.f / horiSubdiv - 90.f) * ANG_RAD;
					auto ring_radius = cos(radian) * radius;
					auto height = sin(radian) * radius;
					auto ang = (i * 360.f / vertSubdiv) * ANG_RAD;
					staging_indices[level].push_back(vertices_1.size());
					ModelVertex1 v;
					v.pos = rotation * Vec3f(cos(ang) * ring_radius, height, sin(ang) * ring_radius);
					v.normal = normalize(v.pos);
					v.pos += center;
					vertices_1.push_back(v);
				}
			}

			{
				staging_indices[0].push_back(vertices_1.size());
				ModelVertex1 v;
				v.pos = rotation * Vec3f(0.f, -radius, 0.f);
				v.normal = normalize(v.pos);
				v.pos += center;
				vertices_1.push_back(v);
			}

			{
				staging_indices[horiSubdiv].push_back(vertices_1.size());
				ModelVertex1 v;
				v.pos = rotation * Vec3f(0.f, radius, 0.f);
				v.normal = normalize(v.pos);
				v.pos += center;
				vertices_1.push_back(v);
			}

			for (int level = 0; level < horiSubdiv; level++)
			{
				if (level == 0)
				{
					for (int i = 0; i < vertSubdiv; i++)
					{
						auto ii = i + 1; if (ii == vertSubdiv) ii = 0;

						indices.push_back(staging_indices[0][0]);
						indices.push_back(staging_indices[1][i]);
						indices.push_back(staging_indices[1][ii]);
					}
				}
				else if (level == horiSubdiv - 1)
				{
					for (int i = 0; i < vertSubdiv; i++)
					{
						auto ii = i + 1; if (ii == vertSubdiv) ii = 0;

						indices.push_back(staging_indices[horiSubdiv - 1][i]);
						indices.push_back(staging_indices[horiSubdiv][0]);
						indices.push_back(staging_indices[horiSubdiv - 1][ii]);
					}
				}
				else
				{
					for (int i = 0; i < vertSubdiv; i++)
					{
						auto ii = i + 1; if (ii == vertSubdiv) ii = 0;

						indices.push_back(staging_indices[level][i]);
						indices.push_back(staging_indices[level + 1][i]);
						indices.push_back(staging_indices[level][ii]);

						indices.push_back(staging_indices[level][ii]);
						indices.push_back(staging_indices[level + 1][i]);
						indices.push_back(staging_indices[level + 1][ii]);
					}
				}
			}
		}

		static ModelPrivate* standard_models[StandardModelCount];

		Model* Model::get_standard(StandardModel m)
		{
			auto ret = standard_models[m];
			if (ret)
				return ret;
			ret = new ModelPrivate;
			switch (m)
			{
			case StandardModelCube:
			{
				auto mesh = new ModelMeshPrivate;

				mesh->set_vertices_pn({
					// F
					-0.5f, +0.5f, +0.5f, +0.f, +0.f, +1.f,
					+0.5f, +0.5f, +0.5f, +0.f, +0.f, +1.f,
					+0.5f, -0.5f, +0.5f, +0.f, +0.f, +1.f,
					-0.5f, -0.5f, +0.5f, +0.f, +0.f, +1.f,
					// B
					-0.5f, +0.5f, -0.5f, +0.f, +0.f, -1.f,
					+0.5f, +0.5f, -0.5f, +0.f, +0.f, -1.f,
					+0.5f, -0.5f, -0.5f, +0.f, +0.f, -1.f,
					-0.5f, -0.5f, -0.5f, +0.f, +0.f, -1.f,
					// T
					-0.5f, +0.5f, -0.5f, +0.f, +1.f, +0.f,
					+0.5f, +0.5f, -0.5f, +0.f, +1.f, +0.f,
					+0.5f, +0.5f, +0.5f, +0.f, +1.f, +0.f,
					-0.5f, +0.5f, +0.5f, +0.f, +1.f, +0.f,
					// B
					-0.5f, -0.5f, -0.5f, +0.f, -1.f, +0.f,
					+0.5f, -0.5f, -0.5f, +0.f, -1.f, +0.f,
					+0.5f, -0.5f, +0.5f, +0.f, -1.f, +0.f,
					-0.5f, -0.5f, +0.5f, +0.f, -1.f, +0.f,
					// L
					-0.5f, +0.5f, -0.5f, -1.f, +0.f, +0.f,
					-0.5f, +0.5f, +0.5f, -1.f, +0.f, +0.f,
					-0.5f, -0.5f, +0.5f, -1.f, +0.f, +0.f,
					-0.5f, -0.5f, -0.5f, -1.f, +0.f, +0.f,
					// R
					+0.5f, +0.5f, -0.5f, +1.f, +0.f, +0.f,
					+0.5f, +0.5f, +0.5f, +1.f, +0.f, +0.f,
					+0.5f, -0.5f, +0.5f, +1.f, +0.f, +0.f,
					+0.5f, -0.5f, -0.5f, +1.f, +0.f, +0.f,
				});

				mesh->set_indices({
					0, 2, 1, 0, 3, 2, // F
					5, 7, 4, 5, 6, 7, // B
					8, 10, 9, 8, 11, 10, // T
					15, 13, 14, 15, 12, 13, // B
					16, 18, 17, 16, 19, 18, // L
					21, 23, 20, 21, 22, 23, // R

				});

				ret->meshes.emplace_back(mesh);
			}
				break;
			case StandardModelSphere:
			{
				auto mesh = new ModelMeshPrivate;

				mesh->add_sphere(0.5f, 8, 8, Vec3f(0.f), Mat3f(1.f));

				ret->meshes.emplace_back(mesh);
			}
				break;
			}
			standard_models[m] = ret;
			return ret;
		}

		ModelPrivate* ModelPrivate::create(const std::filesystem::path& filename)
		{
			if (!std::filesystem::exists(filename))
			{
				wprintf(L"cannot find model: %s\n", filename.c_str());
				return nullptr;
			}

#ifdef USE_ASSIMP
			Assimp::Importer importer;
			auto scene = importer.ReadFile(filename.string(), aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
			if (!scene)
			{
				wprintf(L"load model failed: %s\n", filename.c_str());
				return nullptr;
			}

			auto ret = new ModelPrivate();
			ret->root.reset(new ModelNodePrivate);

			for (auto i = 0; i < scene->mNumMeshes; i++)
			{
				auto src = scene->mMeshes[i];
				auto dst = new ModelMeshPrivate;
				ret->meshes.emplace_back(dst);

				dst->set_vertices(src->mNumVertices, (Vec3f*)src->mVertices, (Vec3f*)src->mTextureCoords[0], (Vec3f*)src->mNormals);

				std::vector<uint> indices(src->mNumFaces * 3);
				for (auto j = 0; j < src->mNumFaces; j++)
				{
					indices[j * 3 + 0] = src->mFaces[j].mIndices[0];
					indices[j * 3 + 1] = src->mFaces[j].mIndices[1];
					indices[j * 3 + 2] = src->mFaces[j].mIndices[2];
				}
				dst->set_indices(indices.size(), indices.data());
			}

			std::function<void(ModelNodePrivate*, aiNode*)> get_node;
			get_node = [&](ModelNodePrivate* dst, aiNode* src) {
				dst->name = std::string(src->mName.C_Str());

				{
					aiVector3D s;
					aiVector3D r;
					ai_real a;
					aiVector3D p;
					src->mTransformation.Decompose(s, r, a, p);
					dst->pos = Vec3f(p.x, p.y, p.z);
					dst->quat = make_quat(a, Vec3f(r.x, r.y, r.z));
					dst->scale = Vec3f(s.x, s.y, s.z);
				}

				if (src->mNumMeshes > 0)
					dst->mesh_index = src->mMeshes[0];

				for (auto i = 0; i < src->mNumChildren; i++)
				{
					auto n = new ModelNodePrivate;
					dst->children.emplace_back(n);
					get_node(n, src->mChildren[i]);
				}
			};
			get_node(ret->root.get(), scene->mRootNode);

			return ret;
#endif

			return nullptr;
		}

		Model* Model::create(const wchar_t* filename)
		{
			return ModelPrivate::create(filename);
		}
	}
}
