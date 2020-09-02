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
		void ModelMesh::set_vertices_p(const std::initializer_list<float>& v)
		{
			assert(v.size() % 3 == 0);
			vertices.resize(v.size() / 3);
			for (auto i = 0; i < vertices.size(); i++)
			{
				vertices[i].pos.x() = v.begin()[i * 3 + 0];
				vertices[i].pos.y() = v.begin()[i * 3 + 1];
				vertices[i].pos.z() = v.begin()[i * 3 + 2];
			}
		}

		void ModelMesh::set_vertices_pn(const std::initializer_list<float>& v)
		{
			assert(v.size() % 6 == 0);
			vertices.resize(v.size() / 6);
			for (auto i = 0; i < vertices.size(); i++)
			{
				vertices[i].pos.x() = v.begin()[i * 6 + 0];
				vertices[i].pos.y() = v.begin()[i * 6 + 1];
				vertices[i].pos.z() = v.begin()[i * 6 + 2];
				vertices[i].normal.x() = v.begin()[i * 6 + 3];
				vertices[i].normal.y() = v.begin()[i * 6 + 4];
				vertices[i].normal.z() = v.begin()[i * 6 + 5];
			}
		}

		void ModelMesh::set_vertices(uint number, Vec3f* poses, Vec3f* uvs, Vec3f* normals, Vec3f* tangents)
		{
			vertices.resize(number);
			if (poses)
			{
				for (auto i = 0; i < number; i++)
					vertices[i].pos = poses[i];
			}
			if (uvs)
			{
				for (auto i = 0; i < number; i++)
					vertices[i].uv = uvs[i];
			}
			if (normals)
			{
				for (auto i = 0; i < number; i++)
					vertices[i].normal = normals[i];
			}
			if (tangents)
			{
				for (auto i = 0; i < number; i++)
					vertices[i].tangent = tangents[i];
			}
		}

		void ModelMesh::set_indices(const std::initializer_list<uint>& v)
		{
			indices.resize(v.size());
			for (auto i = 0; i < indices.size(); i++)
				indices[i] = v.begin()[i];
		}

		void ModelMesh::set_indices(uint number, uint* _indices)
		{
			indices.resize(number);
			for (auto i = 0; i < number; i++)
				indices[i] = _indices[i];
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
				auto mesh = new ModelMesh;

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

			std::function<void(aiNode*, Mat4f)> get_mesh;
			get_mesh = [&](aiNode* n, Mat4f mat) {
				mat = mat * Mat4f(
					Vec4f(n->mTransformation.a1, n->mTransformation.b1, n->mTransformation.c1, n->mTransformation.d1),
					Vec4f(n->mTransformation.a2, n->mTransformation.b2, n->mTransformation.c2, n->mTransformation.d2),
					Vec4f(n->mTransformation.a3, n->mTransformation.b3, n->mTransformation.c3, n->mTransformation.d3),
					Vec4f(n->mTransformation.a4, n->mTransformation.b4, n->mTransformation.c4, n->mTransformation.d4)
				);
				auto mat3 = Mat3f(mat);
				for (auto i = 0; i < n->mNumMeshes; i++)
				{
					auto src = scene->mMeshes[n->mMeshes[i]];
					auto dst = new ModelMesh;

					dst->set_vertices(src->mNumVertices, (Vec3f*)src->mVertices, (Vec3f*)src->mTextureCoords[0], (Vec3f*)src->mNormals, (Vec3f*)src->mTangents);
					for (auto j = 0; j < src->mNumVertices; j++)
					{
						dst->vertices[j].pos = mat * Vec4f(dst->vertices[j].pos, 1.f);
						dst->vertices[j].normal = mat3 * dst->vertices[j].normal;
						dst->vertices[j].tangent = mat3 * dst->vertices[j].tangent;
					}
					std::vector<uint> indices(src->mNumFaces * 3);
					for (auto j = 0; j < src->mNumFaces; j++)
					{
						indices[j * 3 + 0] = src->mFaces[j].mIndices[0];
						indices[j * 3 + 1] = src->mFaces[j].mIndices[1];
						indices[j * 3 + 2] = src->mFaces[j].mIndices[2];
					}
					dst->set_indices(indices.size(), indices.data());

					//auto src_mat = scene->mMaterials[src->mMaterialIndex];
					//aiString str;
					//src_mat->GetTexture(aiTextureType_BASE_COLOR, 0, &str);
					//src_mat->GetTexture(aiTextureType_DIFFUSE, 0, &str);
					//src_mat->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &str);

					ret->meshes.emplace_back(dst);
				}
				for (auto i = 0; i < n->mNumChildren; i++)
					get_mesh(n->mChildren[i], mat);
			};
			get_mesh(scene->mRootNode, Mat4f(1.f));

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
