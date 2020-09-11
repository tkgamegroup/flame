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

		void ModelPrivate::substitute_material(const char* _name, const wchar_t* filename)
		{
			auto name = std::string(_name);
			for (auto& m : materials)
			{
				if (m->name == name)
				{
					std::ifstream mtl(filename);
					while (!mtl.eof())
					{
						std::string line;
						std::getline(mtl, line);
						if (line == "color_map")
						{
							std::getline(mtl, line);
							m->color_map = line;
						}
						else if (line == "roughness_map")
						{
							std::getline(mtl, line);
							m->roughness_map = line;
						}
					}
					mtl.close();
					break;
				}
			}
		}

		void ModelPrivate::save(const std::filesystem::path& filename, const std::string& _model_name) const
		{
			std::ofstream file(filename, std::ios::binary);
			
			write_u(file, materials.size());
			for (auto& m : materials)
			{
				write_s(file, m->name);

				write_t(file, m->color);
				write_t(file, m->metallic);
				write_t(file, m->roughness);
				write_t(file, m->alpha_test);

				write_s(file, m->color_map);
				write_s(file, m->alpha_map);
				write_s(file, m->metallic_map);
				write_s(file, m->roughness_map);
				write_s(file, m->normal_map);
			}

			write_u(file, meshes.size());
			for (auto& m : meshes)
			{
				write_i(file, m->material_index);

				write_v(file, m->vertices_1);
				write_v(file, m->indices);
			}

			root->traverse([&](ModelNodePrivate* n) {
				write_s(file, n->name);

				write_t(file, n->pos);
				write_t(file, n->quat);
				write_t(file, n->scale);

				write_i(file, n->mesh_index);

				write_u(file, n->children.size());
			});

			file.close();

			pugi::xml_document prefab;

			auto model_name = _model_name;
			if (model_name.empty())
				model_name = filename.filename().string();
			std::function<void(pugi::xml_node, ModelNodePrivate*)> print_node;
			print_node = [&](pugi::xml_node dst, ModelNodePrivate* src) {
				auto n = dst.append_child("entity");
				n.append_attribute("name").set_value(src->name.c_str());
				auto nn = n.append_child("cNode");
				nn.append_attribute("pos").set_value(to_string(src->pos).c_str());
				nn.append_attribute("quat").set_value(to_string(src->quat).c_str());
				nn.append_attribute("scale").set_value(to_string(src->scale).c_str());
				if (src->mesh_index != -1)
				{
					auto nm = n.append_child("cMeshInstance");
					nm.append_attribute("src").set_value(model_name.c_str());
					nm.append_attribute("mesh_index").set_value(src->mesh_index);
					if (src->name.starts_with("sm_"))
					{
						auto nr = n.append_child("cRigid");
						nr.append_attribute("dynamic").set_value(false);
						auto ns = n.append_child("cShape");
						ns.append_attribute("type").set_value("Mesh");
					}
				}
				else
				{
					if (src->name.starts_with("trigger_"))
					{
						auto nr = n.append_child("cRigid");
						nr.append_attribute("dynamic").set_value(false);
						auto ns = n.append_child("cShape");
						ns.append_attribute("type").set_value("Cube");
						ns.append_attribute("size").set_value("2,2,0.01");
						ns.append_attribute("trigger").set_value(true);
					}
				}
				for (auto& c : src->children)
					print_node(n, c.get());
			};
			print_node(prefab.append_child("prefab"), root.get());

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
				mesh->add_sphere(0.5f, 12, 12, Vec3f(0.f), Mat3f(1.f));
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

			ModelPrivate* ret = nullptr;

			auto extension = filename.extension();

			if (extension == L".fmod")
			{
				ret = new ModelPrivate();
				ret->filename = filename;

				std::ifstream file(filename, std::ios::binary);

				ret->materials.resize(read_u(file));
				for (auto i = 0; i < ret->materials.size(); i++)
				{
					auto m = new ModelMaterialPrivate;
					ret->materials[i].reset(m);

					read_s(file, m->name);

					read_t(file, m->color);
					read_t(file, m->metallic);
					read_t(file, m->roughness);
					read_t(file, m->alpha_test);

					read_s(file, m->color_map);
					read_s(file, m->alpha_map);
					if (!m->alpha_map.empty())
						m->alpha_test = 0.5f;
					read_s(file, m->metallic_map);
					read_s(file, m->roughness_map);
					read_s(file, m->normal_map);
				}

				ret->meshes.resize(read_u(file));
				for (auto i = 0; i < ret->meshes.size(); i++)
				{
					auto m = new ModelMeshPrivate;
					ret->meshes[i].reset(m);

					m->material_index = read_i(file);

					read_v(file, m->vertices_1);
					read_v(file, m->indices);
				}

				std::function<void(ModelNodePrivate*)> load_node;
				load_node = [&](ModelNodePrivate* n) {
					read_s(file, n->name);

					read_t(file, n->pos);
					read_t(file, n->quat);
					read_t(file, n->scale);

					n->mesh_index = read_i(file);

					n->children.resize(read_u(file));
					for (auto i = 0; i < n->children.size(); i++)
					{
						auto c = new ModelNodePrivate;
						n->children[i].reset(c);
						load_node(c);
					}
				};
				load_node(ret->root.get());

				file.close();
			}
			else
			{
				auto is_obj = filename.extension() == L".obj";
#ifdef USE_ASSIMP
				Assimp::Importer importer;
				auto load_flags = 
					aiProcess_RemoveRedundantMaterials |
					aiProcess_Triangulate |
					aiProcess_JoinIdenticalVertices |
					aiProcess_SortByPType;
				if (is_obj)
				{
					load_flags = load_flags |
						aiProcess_FlipUVs;
				}
				auto scene = importer.ReadFile(filename.string(), load_flags);
				if (!scene)
				{
					wprintf(L"load model failed: %s\n", filename.c_str());
					return nullptr;
				}

				ret = new ModelPrivate();

				ret->materials.clear();
				for (auto i = 0; i < scene->mNumMaterials; i++)
				{
					auto src = scene->mMaterials[i];
					auto dst = new ModelMaterialPrivate;
					ret->materials.emplace_back(dst);

					aiString name;
					aiColor3D color;
					ai_real shininess;

					dst->name = src->GetName().C_Str();

					std::string filename;

					name.Clear();
					src->GetTexture(aiTextureType_DIFFUSE, 0, &name);
					filename = name.C_Str();
					if (!filename.empty())
					{
						if (filename[0] == '/')
							filename.erase(filename.begin());
						dst->color_map = filename;
					}

					name.Clear();
					src->GetTexture(aiTextureType_OPACITY, 0, &name);
					filename = name.C_Str();
					if (!filename.empty())
					{
						if (filename[0] == '/')
							filename.erase(filename.begin());
						dst->alpha_map = filename;
					}
				}

				for (auto i = 0; i < scene->mNumMeshes; i++)
				{
					auto src = scene->mMeshes[i];
					auto dst = new ModelMeshPrivate;
					ret->meshes.emplace_back(dst);

					dst->material_index = src->mMaterialIndex;

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
						a *= RAD_ANG;
						dst->pos = Vec3f(p.x, p.y, p.z);
						dst->quat = make_quat(a, Vec3f(r.x, r.y, r.z));
						dst->scale = Vec3f(s.x, s.y, s.z);
					}

					if (src->mNumMeshes > 0)
					{
						dst->mesh_index = src->mMeshes[0];
						if (dst->name.starts_with("trigger_"))
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
				{
					auto n = new ModelNodePrivate;
					ret->root->children.emplace_back(n);
					get_node(n, scene->mRootNode);
				}

				if (is_obj)
				{
					for (auto i = 0; i < ret->meshes.size(); i++)
					{
						if (ret->meshes[i]->ref_cnt == 0)
						{
							auto n = new ModelNodePrivate;
							n->mesh_index = i;
							ret->root->children.emplace_back(n);
						}
					}
				}
#endif
			}

			if (ret && ret->root->children.empty())
			{
				auto n = new ModelNodePrivate;
				auto r = ret->root.release();
				n->children.emplace_back(r);
				ret->root.reset(n);
			}

			return ret;
		}

		Model* Model::create(const wchar_t* filename)
		{
			return ModelPrivate::create(filename);
		}
	}
}
