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

		void ModelNodePrivate::traverse(const std::function<void(ModelNodePrivate*)>& callback)
		{
			callback(this);
			for (auto& c : children)
				c->traverse(callback);
		}

		ModelPrivate::ModelPrivate()
		{
			materials.emplace_back(new ModelMaterialPrivate);
			root.reset(new ModelNodePrivate);
		}

		void ModelPrivate::save(const std::filesystem::path& filename) const
		{
			std::ofstream file(filename, std::ios::binary);
			uint size;
			int idx;
			
			size = materials.size();
			file.write((char*)&size, sizeof(uint));
			for (auto& m : materials)
			{

			}

			size = meshes.size();
			file.write((char*)&size, sizeof(uint));
			for (auto& m : meshes)
			{
				file.write((char*)&m->material_index, sizeof(int));

				size = m->vertices_1.size();
				file.write((char*)&size, sizeof(uint));
				file.write((char*)m->vertices_1.data(), sizeof(ModelVertex1) * m->vertices_1.size());
				size = m->indices.size();
				file.write((char*)&size, sizeof(uint));
				file.write((char*)m->indices.data(), sizeof(uint) * m->indices.size());
			}

			root->traverse([&](ModelNodePrivate* n) {
				size = n->name.size();
				file.write((char*)&size, sizeof(uint));
				file.write(n->name.data(), n->name.size());
				file.write((char*)&n->pos, sizeof(Vec3f));
				file.write((char*)&n->quat, sizeof(Vec4f));
				file.write((char*)&n->scale, sizeof(Vec3f));
				file.write((char*)&n->mesh_index, sizeof(uint));
				size = n->children.size();
				file.write((char*)&size, sizeof(uint));
			});

			file.close();

			pugi::xml_document prefab;
			auto prefab_root = prefab.append_child("prefab");

			std::function<void(pugi::xml_node, ModelNodePrivate*)> print_node;
			print_node = [&](pugi::xml_node dst, ModelNodePrivate* src) {

			};

			auto prefab_path = filename;
			prefab_path.replace_extension(L".prefab");
			prefab.save_file(prefab_path.c_str());
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
				ret->root->mesh_index = 0;
			}
				break;
			case StandardModelSphere:
			{
				auto mesh = new ModelMeshPrivate;
				mesh->add_sphere(0.5f, 8, 8, Vec3f(0.f), Mat3f(1.f));
				ret->meshes.emplace_back(mesh);
				ret->root->mesh_index = 0;
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

			if (filename.extension() == L".fm")
			{
				auto ret = new ModelPrivate();

				std::ifstream file(filename, std::ios::binary);
				uint size;
				int idx;

				file.read((char*)&size, sizeof(uint));
				ret->materials.resize(size);
				for (auto i = 0; i < ret->materials.size(); i++)
				{

				}

				file.read((char*)&size, sizeof(uint));
				ret->meshes.resize(size);
				for (auto i = 0; i < ret->meshes.size(); i++)
				{
					auto m = new ModelMeshPrivate;
					ret->meshes[i].reset(m);

					file.read((char*)&m->material_index, sizeof(int));

					file.read((char*)&size, sizeof(uint));
					m->vertices_1.resize(size);
					file.read((char*)m->vertices_1.data(), sizeof(ModelVertex1) * m->vertices_1.size());
					file.read((char*)&size, sizeof(uint));
					m->indices.resize(size);
					file.read((char*)m->indices.data(), sizeof(uint) * m->indices.size());
				}

				std::function<void(ModelNodePrivate*)> load_node;
				load_node = [&](ModelNodePrivate* n) {
					file.read((char*)&size, sizeof(uint));
					n->name.resize(size);
					file.read(n->name.data(), n->name.size());
					file.read((char*)&n->pos, sizeof(Vec3f));
					file.read((char*)&n->quat, sizeof(Vec4f));
					file.read((char*)&n->scale, sizeof(Vec3f));
					file.read((char*)&n->mesh_index, sizeof(int));
					file.read((char*)&size, sizeof(uint));
					n->children.resize(size);
					for (auto i = 0; i < n->children.size(); i++)
					{
						auto c = new ModelNodePrivate;
						n->children[i].reset(c);
						load_node(c);
					}
				};
				load_node(ret->root.get());

				file.close();

				return ret;
			}
			else
			{
#ifdef USE_ASSIMP
				Assimp::Importer importer;
				importer.SetPropertyString(AI_CONFIG_PP_OG_EXCLUDE_LIST, "trigger0 trigger1 trigger2 trigger3");
				auto scene = importer.ReadFile(filename.string(),
					aiProcess_Triangulate |
					aiProcess_JoinIdenticalVertices |
					aiProcess_SortByPType |
					aiProcess_OptimizeGraph |
					aiProcess_OptimizeMeshes |
					aiProcess_RemoveRedundantMaterials);
				if (!scene)
				{
					wprintf(L"load model failed: %s\n", filename.c_str());
					return nullptr;
				}

				auto ret = new ModelPrivate();

				for (auto i = 0; i < scene->mNumMaterials; i++)
				{
					auto src = scene->mMaterials[i];
					auto dst = new ModelMaterialPrivate;
					ret->materials.emplace_back(dst);
				}

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
					{
						dst->mesh_index = src->mMeshes[0];
						if (dst->name.starts_with("trigger"))
						{
							auto& m = ret->meshes[dst->mesh_index];
							if (m->vertices_1.size() == 4 && m->indices.size() == 6)
							{
								// plane
								// TODO
							}
							dst->mesh_index = -1;
						}
					}
					if (dst->mesh_index != -1)
						ret->meshes[dst->mesh_index]->ref_cnt++;

					for (auto i = 0; i < src->mNumChildren; i++)
					{
						auto n = new ModelNodePrivate;
						dst->children.emplace_back(n);
						get_node(n, src->mChildren[i]);
					}
				};
				get_node(ret->root.get(), scene->mRootNode);

				for (auto it = ret->meshes.begin(); it != ret->meshes.end(); )
				{
					if ((*it)->ref_cnt == 0)
						it = ret->meshes.erase(it);
					else
						it++;
				}

				return ret;
#endif
			}

			return nullptr;
		}

		Model* Model::create(const wchar_t* filename)
		{
			return ModelPrivate::create(filename);
		}
	}
}
