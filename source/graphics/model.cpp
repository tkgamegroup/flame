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
		void MeshPrivate::set_vertices_p(const std::initializer_list<float>& v)
		{
			assert(v.size() % 3 == 0);
			auto n = v.size() / 3;
			positions.resize(n);
			for (auto i = 0; i < n; i++)
			{
				positions[i].x() = v.begin()[i * 3 + 0];
				positions[i].y() = v.begin()[i * 3 + 1];
				positions[i].z() = v.begin()[i * 3 + 2];
			}
		}

		void MeshPrivate::set_vertices_pn(const std::initializer_list<float>& v)
		{
			assert(v.size() % 6 == 0);
			auto n = v.size() / 6;
			positions.resize(n);
			normals.resize(n);
			for (auto i = 0; i < n; i++)
			{
				positions[i].x() = v.begin()[i * 6 + 0];
				positions[i].y() = v.begin()[i * 6 + 1];
				positions[i].z() = v.begin()[i * 6 + 2];
				normals[i].x() = v.begin()[i * 6 + 3];
				normals[i].y() = v.begin()[i * 6 + 4];
				normals[i].z() = v.begin()[i * 6 + 5];
			}
		}

		void MeshPrivate::set_vertices(uint n, Vec3f* _positions, Vec3f* _uvs, Vec3f* _normals)
		{
			positions.resize(n);
			for (auto i = 0; i < n; i++)
				positions[i] = _positions[i];
			if (_uvs)
			{
				uvs.resize(n);
				for (auto i = 0; i < n; i++)
					uvs[i] = _uvs[i];
			}
			if (_normals)
			{
				normals.resize(n);
				for (auto i = 0; i < n; i++)
					normals[i] = _normals[i];
			}
		}

		void MeshPrivate::set_indices(const std::initializer_list<uint>& v)
		{
			indices.resize(v.size());
			for (auto i = 0; i < indices.size(); i++)
				indices[i] = v.begin()[i];
		}

		void MeshPrivate::set_indices(uint n, uint* _indices)
		{
			indices.resize(n);
			for (auto i = 0; i < n; i++)
				indices[i] = _indices[i];
		}

		void MeshPrivate::add_sphere(float radius, uint horiSubdiv, uint vertSubdiv, const Vec3f& center, const Mat3f& rotation)
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
					staging_indices[level].push_back(positions.size());
					auto p = rotation * Vec3f(cos(ang) * ring_radius, height, sin(ang) * ring_radius);
					normals.push_back(p);
					positions.push_back(p + center);
				}
			}

			{
				staging_indices[0].push_back(positions.size());
				auto p = rotation * Vec3f(0.f, -radius, 0.f);
				normals.push_back(p);
				positions.push_back(p + center);
			}

			{
				staging_indices[horiSubdiv].push_back(positions.size());
				auto p = rotation * Vec3f(0.f, radius, 0.f);
				normals.push_back(p);
				positions.push_back(p + center);
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

		void NodePrivate::traverse(const std::function<void(NodePrivate*)>& callback)
		{
			callback(this);
			for (auto& c : children)
				c->traverse(callback);
		}

		ModelPrivate::ModelPrivate()
		{
			materials.emplace_back(new MaterialPrivate);
			root.reset(new NodePrivate);
		}

		int ModelPrivate::find_mesh(const std::string& name) const
		{
			for (auto i = 0; i < meshes.size(); i++)
			{
				if (meshes[i]->name == name)
					return i;
			}
			return -1;
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

				auto n = m->positions.size();
				write_u(file, n);
				file.write((char*)m->positions.data(), sizeof(Vec3f) * n);
				write_b(file, !m->uvs.empty());
				if (!m->uvs.empty())
					file.write((char*)m->uvs.data(), sizeof(Vec2f) * n);
				write_b(file, !m->normals.empty());
				if (!m->normals.empty())
					file.write((char*)m->normals.data(), sizeof(Vec3f) * n);
				write_v(file, m->indices);
			}

			root->traverse([&](NodePrivate* n) {
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
			std::function<void(pugi::xml_node, NodePrivate*)> print_node;
			print_node = [&](pugi::xml_node dst, NodePrivate* src) {
				auto n = dst.append_child("entity");
				n.append_attribute("name").set_value(src->name.c_str());
				auto nn = n.append_child("cNode");
				nn.append_attribute("pos").set_value(to_string(src->pos).c_str());
				nn.append_attribute("quat").set_value(to_string(src->quat).c_str());
				nn.append_attribute("scale").set_value(to_string(src->scale).c_str());
				if (src->mesh_index != -1)
				{
					auto nm = n.append_child("cMesh");
					nm.append_attribute("src").set_value((model_name + "." + meshes[src->mesh_index]->name).c_str());
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

		static ModelPrivate* standard_cube = nullptr;
		static ModelPrivate* standard_sphere = nullptr;

		Model* Model::get_standard(const char* _name)
		{
			auto name = std::string(_name);
			if (name == "cube")
			{
				if (!standard_cube)
				{
					auto m = new ModelPrivate;
					auto mesh = new MeshPrivate;
					mesh->name = "0";
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
					m->meshes.emplace_back(mesh);
					m->root->mesh_index = 0;

					standard_cube = m;
				}
				return standard_cube;
			}
			else if (name == "sphere")
			{
				if (!standard_sphere)
				{
					auto m = new ModelPrivate;
					auto mesh = new MeshPrivate;
					mesh->name = "0";
					mesh->add_sphere(0.5f, 12, 12, Vec3f(0.f), Mat3f(1.f));
					m->meshes.emplace_back(mesh);
					m->root->mesh_index = 0;

					standard_sphere = m;
				}
				return standard_sphere;
			}
			return nullptr;
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
					auto m = new MaterialPrivate;
					m->path = filename.parent_path();
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
					auto m = new MeshPrivate;
					ret->meshes[i].reset(m);

					m->material_index = read_i(file);

					auto n = read_u(file);
					m->positions.resize(n);
					file.read((char*)m->positions.data(), sizeof(Vec3f) * n);
					if (read_b(file))
					{
						m->uvs.resize(n);
						file.read((char*)m->uvs.data(), sizeof(Vec2f) * n);
					}
					if (read_b(file))
					{
						m->normals.resize(n);
						file.read((char*)m->normals.data(), sizeof(Vec3f) * n);
					}
					read_v(file, m->indices);
				}

				std::function<void(NodePrivate*)> load_node;
				load_node = [&](NodePrivate* n) {
					read_s(file, n->name);

					read_t(file, n->pos);
					read_t(file, n->quat);
					read_t(file, n->scale);

					n->mesh_index = read_i(file);

					n->children.resize(read_u(file));
					for (auto i = 0; i < n->children.size(); i++)
					{
						auto c = new NodePrivate;
						n->children[i].reset(c);
						load_node(c);
					}
				};
				load_node(ret->root.get());

				file.close();
			}
			else
			{
#ifdef USE_ASSIMP
				Assimp::Importer importer;
				auto load_flags = 
					aiProcess_RemoveRedundantMaterials |
					aiProcess_Triangulate |
					aiProcess_JoinIdenticalVertices |
					aiProcess_SortByPType |
					aiProcess_FlipUVs;
				auto scene = importer.ReadFile(filename.string(), load_flags);
				if (!scene)
				{
					printf("load model %s failed: %s\n", filename.string().c_str(), importer.GetErrorString());
					return nullptr;
				}

				ret = new ModelPrivate();

				ret->materials.clear();
				for (auto i = 0; i < scene->mNumMaterials; i++)
				{
					auto src = scene->mMaterials[i];
					auto dst = new MaterialPrivate;
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
					auto dst = new MeshPrivate;
					ret->meshes.emplace_back(dst);

					dst->name = src->mName.C_Str();
					if (dst->name.empty())
						dst->name = std::to_string(i);

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

				for (auto i = 0; i < ret->meshes.size(); i++)
				{
					auto name = ret->meshes[i]->name;
					auto n = 0;
					auto unique = [&]() {
						for (auto j = 0; j < ret->meshes.size(); j++)
						{
							if (ret->meshes[j]->name == name)
								return false;
						}
						return true;
					};
					while (!unique())
					{
						name = ret->meshes[i]->name + std::to_string(i);
						n++;
					}
					ret->meshes[i]->name = name;
				}

				std::vector<uint> mesh_refs;
				mesh_refs.resize(ret->meshes.size());

				std::function<void(NodePrivate*, aiNode*)> get_node;
				get_node = [&](NodePrivate* dst, aiNode* src) {
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
							if (m->positions.size() == 4 && m->indices.size() == 6)
							{
								// plane
								// TODO
							}
							dst->mesh_index = -1;
						}
					}
					if (dst->mesh_index != -1)
						mesh_refs[dst->mesh_index]++;

					for (auto i = 0; i < src->mNumChildren; i++)
					{
						auto n = new NodePrivate;
						dst->children.emplace_back(n);
						get_node(n, src->mChildren[i]);
					}
				};
				{
					auto n = new NodePrivate;
					ret->root->children.emplace_back(n);
					get_node(n, scene->mRootNode);
				}

				for (auto i = 0; i < mesh_refs.size(); i++)
				{
					if (mesh_refs[i] == 0)
					{
						auto n = new NodePrivate;
						n->mesh_index = i;
						ret->root->children.emplace_back(n);
					}
				}
#endif
			}

			if (ret && ret->root->children.empty())
			{
				auto n = new NodePrivate;
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
