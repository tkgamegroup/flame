#include "../base64.h"
#include "../xml.h"
#include "../foundation/typeinfo.h"
#include "material_private.h"
#include "model_private.h"

#ifdef USE_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

namespace flame
{
	namespace graphics
	{
		void MeshPrivate::add_vertices(uint n, vec3* _positions, vec3* _uvs, vec3* _normals)
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

		void MeshPrivate::add_indices(uint n, uint* _indices)
		{
			auto b = indices.size();
			indices.resize(b + n);
			for (auto i = 0; i < n; i++)
				indices[b + i] = _indices[i];
		}

		void MeshPrivate::calc_bounds()
		{
			bounds.reset();
			for (auto& p : positions)
				bounds.expand(p);
		}

		void MeshPrivate::add_cube(const vec3& extent, const vec3& center, const mat3& rotation)
		{
			positions.push_back(rotation * vec3(-0.5f, +0.5f, +0.5f) * extent + center);
			positions.push_back(rotation * vec3(+0.5f, +0.5f, +0.5f) * extent + center);
			positions.push_back(rotation * vec3(+0.5f, -0.5f, +0.5f) * extent + center);
			positions.push_back(rotation * vec3(-0.5f, -0.5f, +0.5f) * extent + center);
			normals.push_back(vec3(+0.f, +0.f, +1.f));
			normals.push_back(vec3(+0.f, +0.f, +1.f));
			normals.push_back(vec3(+0.f, +0.f, +1.f));
			normals.push_back(vec3(+0.f, +0.f, +1.f));
			indices.push_back(0); indices.push_back(2); indices.push_back(1);
			indices.push_back(0); indices.push_back(3); indices.push_back(2);

			positions.push_back(rotation * vec3(-0.5f, +0.5f, -0.5f) * extent + center);
			positions.push_back(rotation * vec3(+0.5f, +0.5f, -0.5f) * extent + center);
			positions.push_back(rotation * vec3(+0.5f, -0.5f, -0.5f) * extent + center);
			positions.push_back(rotation * vec3(-0.5f, -0.5f, -0.5f) * extent + center);
			normals.push_back(vec3(+0.f, +0.f, -1.f));
			normals.push_back(vec3(+0.f, +0.f, -1.f));
			normals.push_back(vec3(+0.f, +0.f, -1.f));
			normals.push_back(vec3(+0.f, +0.f, -1.f));
			indices.push_back(5); indices.push_back(7); indices.push_back(4);
			indices.push_back(5); indices.push_back(6); indices.push_back(7);

			positions.push_back(rotation * vec3(-0.5f, +0.5f, -0.5f) * extent + center);
			positions.push_back(rotation * vec3(+0.5f, +0.5f, -0.5f) * extent + center);
			positions.push_back(rotation * vec3(+0.5f, +0.5f, +0.5f) * extent + center);
			positions.push_back(rotation * vec3(-0.5f, +0.5f, +0.5f) * extent + center);
			normals.push_back(vec3(+0.f, +1.f, +0.f));
			normals.push_back(vec3(+0.f, +1.f, +0.f));
			normals.push_back(vec3(+0.f, +1.f, +0.f));
			normals.push_back(vec3(+0.f, +1.f, +0.f));
			indices.push_back(8); indices.push_back(10); indices.push_back(9);
			indices.push_back(8); indices.push_back(11); indices.push_back(10);

			positions.push_back(rotation * vec3(-0.5f, -0.5f, -0.5f) * extent + center);
			positions.push_back(rotation * vec3(+0.5f, -0.5f, -0.5f) * extent + center);
			positions.push_back(rotation * vec3(+0.5f, -0.5f, +0.5f) * extent + center);
			positions.push_back(rotation * vec3(-0.5f, -0.5f, +0.5f) * extent + center);
			normals.push_back(vec3(+0.f, -1.f, +0.f));
			normals.push_back(vec3(+0.f, -1.f, +0.f));
			normals.push_back(vec3(+0.f, -1.f, +0.f));
			normals.push_back(vec3(+0.f, -1.f, +0.f));
			indices.push_back(15); indices.push_back(13); indices.push_back(14);
			indices.push_back(15); indices.push_back(12); indices.push_back(13);

			positions.push_back(rotation * vec3(-0.5f, +0.5f, -0.5f) * extent + center);
			positions.push_back(rotation * vec3(-0.5f, +0.5f, +0.5f) * extent + center);
			positions.push_back(rotation * vec3(-0.5f, -0.5f, +0.5f) * extent + center);
			positions.push_back(rotation * vec3(-0.5f, -0.5f, -0.5f) * extent + center);
			normals.push_back(vec3(-1.f, +0.f, +0.f));
			normals.push_back(vec3(-1.f, +0.f, +0.f));
			normals.push_back(vec3(-1.f, +0.f, +0.f));
			normals.push_back(vec3(-1.f, +0.f, +0.f));
			indices.push_back(16); indices.push_back(18); indices.push_back(17);
			indices.push_back(16); indices.push_back(19); indices.push_back(18);

			positions.push_back(rotation * vec3(+0.5f, +0.5f, -0.5f) * extent + center);
			positions.push_back(rotation * vec3(+0.5f, +0.5f, +0.5f) * extent + center);
			positions.push_back(rotation * vec3(+0.5f, -0.5f, +0.5f) * extent + center);
			positions.push_back(rotation * vec3(+0.5f, -0.5f, -0.5f) * extent + center);
			normals.push_back(vec3(+1.f, +0.f, +0.f));
			normals.push_back(vec3(+1.f, +0.f, +0.f));
			normals.push_back(vec3(+1.f, +0.f, +0.f));
			normals.push_back(vec3(+1.f, +0.f, +0.f));
			indices.push_back(21); indices.push_back(23); indices.push_back(20);
			indices.push_back(21); indices.push_back(22); indices.push_back(23);
		}

		void MeshPrivate::add_sphere(float radius, uint horiSubdiv, uint vertSubdiv, const vec3& center, const mat3& rotation)
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
					staging_indices[level].push_back(positions.size());
					auto p = rotation * vec3(cos(ang) * ring_radius, height, sin(ang) * ring_radius);
					normals.push_back(p);
					positions.push_back(p + center);
				}
			}

			{
				staging_indices[0].push_back(positions.size());
				auto p = rotation * vec3(0.f, -radius, 0.f);
				normals.push_back(p);
				positions.push_back(p + center);
			}

			{
				staging_indices[horiSubdiv].push_back(positions.size());
				auto p = rotation * vec3(0.f, radius, 0.f);
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

		void Model::convert(const wchar_t* _filename)
		{
#ifdef USE_ASSIMP
			auto filename = std::filesystem::path(_filename);
			auto ppath = filename.parent_path();
			auto model_name = filename.filename().stem().string();
			auto model_filename = filename;
			model_filename.replace_extension(L".fmod");

			Assimp::Importer importer;
			importer.SetPropertyString(AI_CONFIG_PP_OG_EXCLUDE_LIST, "trigger");
			auto load_flags =
				aiProcess_RemoveRedundantMaterials |
				aiProcess_Triangulate |
				aiProcess_JoinIdenticalVertices |
				aiProcess_SortByPType |
				aiProcess_FlipUVs |
				aiProcess_LimitBoneWeights |
				aiProcess_OptimizeMeshes |
				aiProcess_OptimizeGraph;
			auto scene = importer.ReadFile(filename.string(), load_flags);
			if (!scene)
			{
				printf("cannot import model %s: %s\n", filename.string().c_str(), importer.GetErrorString());
				return;
			}

			std::vector<std::string> material_names;

			for (auto i = 0; i < scene->mNumMaterials; i++)
			{
				aiString ai_name;
				auto ai_mat = scene->mMaterials[i];

				pugi::xml_document doc;
				auto n_material = doc.append_child("material");
				
				std::string pipeline_defines;

				auto map_id = 0;

				{
					ai_name.Clear();
					ai_mat->GetTexture(aiTextureType_DIFFUSE, 0, &ai_name);
					auto name = std::string(ai_name.C_Str());
					if (!name.empty())
					{
						if (name[0] == '/')
							name.erase(name.begin());
						auto n_texture = n_material.append_child("texture");
						n_texture.append_attribute("filename").set_value(name.c_str());
						pipeline_defines += sfmt("COLOR_MAP=%d ", map_id++);
					}
				}

				{
					ai_name.Clear();
					ai_mat->GetTexture(aiTextureType_OPACITY, 0, &ai_name);
					auto name = std::string(ai_name.C_Str());
					if (!name.empty())
					{
						if (name[0] == '/')
							name.erase(name.begin());
						auto n_texture = n_material.append_child("texture");
						n_texture.append_attribute("filename").set_value(name.c_str());
						pipeline_defines += "ALPHA_TEST ";
						pipeline_defines += sfmt("ALPHA_MAP=%d ", map_id++);
						n_material.append_attribute("alpha_test").set_value(to_string(0.5f).c_str());
					}
				}

				if (!pipeline_defines.empty())
					n_material.append_attribute("pipeline_defines").set_value(pipeline_defines.c_str());

				auto material_name = std::string(ai_mat->GetName().C_Str());
				if (material_name.empty())
					material_name = std::to_string(i);
				else
				{
					for (auto& ch : material_name)
					{
						if (ch == ' ' || ch == ':')
							ch = '_';
					}
				}
				material_name = model_name + "_" + material_name + ".fmat";
				material_names.push_back(material_name);
				doc.save_file((ppath / material_name).c_str());
			}

			pugi::xml_document doc;
			auto n_model = doc.append_child("model");

			auto model_data_filename = model_filename;
			model_data_filename += L".dat";
			std::ofstream model_data_file(model_data_filename, std::ios::binary);

			std::vector<std::pair<std::string, mat4>> bones;

			auto n_meshes = n_model.append_child("meshes");
			for (auto i = 0; i < scene->mNumMeshes; i++)
			{
				auto ai_mesh = scene->mMeshes[i];

				auto n_mesh = n_meshes.append_child("mesh");

				n_mesh.append_attribute("material").set_value(material_names[ai_mesh->mMaterialIndex].c_str());

				auto vertex_count = ai_mesh->mNumVertices;

				{
					auto size = vertex_count * sizeof(vec3);
					auto n_positions = n_mesh.append_child("positions");
					n_positions.append_attribute("offset").set_value(model_data_file.tellp());
					n_positions.append_attribute("size").set_value(size);
					model_data_file.write((char*)ai_mesh->mVertices, size);
				}

				auto puv = ai_mesh->mTextureCoords[0];
				if (puv)
				{
					std::vector<vec2> uvs;
					uvs.resize(vertex_count);
					for (auto j = 0; j < vertex_count; j++)
					{
						auto& uv = puv[j];
						uvs[j] = vec2(uv.x, uv.y);
					}
					auto size = vertex_count * sizeof(vec2);
					auto n_uvs = n_mesh.append_child("uvs");
					n_uvs.append_attribute("offset").set_value(model_data_file.tellp());
					n_uvs.append_attribute("size").set_value(size);
					model_data_file.write((char*)uvs.data(), size);
				}

				if (ai_mesh->mNormals)
				{
					auto size = vertex_count * sizeof(vec3);
					auto n_normals = n_mesh.append_child("normals");
					n_normals.append_attribute("offset").set_value(model_data_file.tellp());
					n_normals.append_attribute("size").set_value(size);
					model_data_file.write((char*)ai_mesh->mNormals, size);
				}

				AABB bounds;
				bounds.reset();
				for (auto j = 0; j < vertex_count; j++)
				{
					auto& p = ai_mesh->mVertices[j]; 
					bounds.expand(vec3(p.x, p.y, p.z));
				}
				n_mesh.append_attribute("bounds").set_value(to_string((mat2x3&)bounds).c_str());

				std::vector<ivec4> bone_ids;
				std::vector<vec4> bone_weights;
				if (ai_mesh->mNumBones > 0)
				{
					bone_ids.resize(vertex_count);
					bone_weights.resize(vertex_count);
					for (auto j = 0; j < vertex_count; j++)
					{
						bone_ids[j] = ivec4(-1);
						bone_weights[j] = vec4(0.f);
					}
				}

				for (auto j = 0; j < ai_mesh->mNumBones; j++)
				{
					auto ai_bone = ai_mesh->mBones[j];

					auto name = std::string(ai_bone->mName.C_Str());
					auto find_bone = [&](const std::string& name) {
						for (auto i = 0; i < bones.size(); i++)
						{
							if (bones[i].first == name)
								return i;
						}
						return -1;
					};
					auto bid = find_bone(name);
					if (bid == -1)
					{
						bid = bones.size();
						auto& m = ai_bone->mOffsetMatrix;
						auto offset_matrix = mat4(
							vec4(m.a1, m.b1, m.c1, m.d1),
							vec4(m.a2, m.b2, m.c2, m.d2),
							vec4(m.a3, m.b3, m.c3, m.d3),
							vec4(m.a4, m.b4, m.c4, m.d4)
						);
						bones.emplace_back(name, offset_matrix);
					}

					auto weights_count = ai_bone->mNumWeights;
					if (weights_count > 0)
					{
						auto get_idx = [&](uint vi) {
							auto& ids = bone_ids[vi];
							for (auto i = 0; i < 4; i++)
							{
								if (ids[i] == -1)
									return i;
							}
							return -1;
						};
						for (auto j = 0; j < weights_count; j++)
						{
							auto w = ai_bone->mWeights[j];
							auto idx = get_idx(w.mVertexId);
							if (idx == -1)
								continue;
							bone_ids[w.mVertexId][idx] = bid;
							bone_weights[w.mVertexId][idx] = w.mWeight;
						}
					}
				}

				if (!bone_ids.empty())
				{
					auto size = vertex_count * sizeof(ivec4);
					auto n_bids = n_mesh.append_child("bone_ids");
					n_bids.append_attribute("offset").set_value(model_data_file.tellp());
					n_bids.append_attribute("size").set_value(size);
					model_data_file.write((char*)bone_ids.data() , size);
				}

				if (!bone_weights.empty())
				{
					auto size = vertex_count * sizeof(vec4);
					auto n_wgts = n_mesh.append_child("bone_weights");
					n_wgts.append_attribute("offset").set_value(model_data_file.tellp());
					n_wgts.append_attribute("size").set_value(size);
					model_data_file.write((char*)bone_weights.data(), size);
				}

				{
					std::vector<uint> indices(ai_mesh->mNumFaces * 3);
					for (auto j = 0; j < ai_mesh->mNumFaces; j++)
					{
						indices[j * 3 + 0] = ai_mesh->mFaces[j].mIndices[0];
						indices[j * 3 + 1] = ai_mesh->mFaces[j].mIndices[1];
						indices[j * 3 + 2] = ai_mesh->mFaces[j].mIndices[2];
					}
					auto size = indices.size() * sizeof(uint);
					auto n_indices = n_mesh.append_child("indices");
					n_indices.append_attribute("offset").set_value(model_data_file.tellp());
					n_indices.append_attribute("size").set_value(size);
					model_data_file.write((char*)indices.data(), size);
				}
			}

			auto n_bones = n_model.append_child("bones");
			for (auto& b : bones)
			{
				auto n_bone = n_bones.append_child("bone");
				n_bone.append_attribute("name").set_value(b.first.c_str());

				{
					auto size = sizeof(mat4);
					auto n_matrix = n_bone.append_child("offset_matrix");
					n_matrix.append_attribute("offset").set_value(model_data_file.tellp());
					n_matrix.append_attribute("size").set_value(size);
					model_data_file.write((char*)&b.second, size);
				}
			}

			model_data_file.close();

			doc.save_file(model_filename.c_str());

			pugi::xml_document doc_prefab;

			std::function<void(pugi::xml_node, aiNode*)> print_node;
			print_node = [&](pugi::xml_node dst, aiNode* src) {
				auto n = dst.append_child("eNode");

				auto name = std::string(src->mName.C_Str());
				n.append_attribute("name").set_value(name.c_str());

				{
					aiVector3D s;
					aiVector3D r;
					ai_real a;
					aiVector3D p;
					src->mTransformation.Decompose(s, r, a, p);
					a *= 0.5f;
					auto q = normalize(vec4(sin(a) * vec3(r.x, r.y, r.z), cos(a)));

					auto pos = vec3(p.x, p.y, p.z);
					if (pos != vec3(0.f))
						n.append_attribute("pos").set_value(to_string(pos).c_str());
					auto qut = vec4(q.w, q.x, q.y, q.z);
					if (qut != vec4(1.f, 0.f, 0.f, 0.f))
						n.append_attribute("quat").set_value(to_string(qut).c_str());
					auto scl = vec3(s.x, s.y, s.z);
					if (scl != vec3(1.f))
						n.append_attribute("scale").set_value(to_string(scl).c_str());
				}

				if (src == scene->mRootNode)
				{
					if (!bones.empty())
					{
						auto na = n.append_child("cArmature");
						na.append_attribute("model").set_value((model_name + ".fmod").c_str());
					}
				}
				else
					n.append_attribute("assemble_sub").set_value(true);

				if (src->mNumMeshes > 0)
				{
					if (name == "trigger")
					{
						auto nr = n.append_child("cRigid");
						nr.append_attribute("dynamic").set_value(false);
						auto ns = n.append_child("cShape");
						ns.append_attribute("type").set_value("Cube");
						ns.append_attribute("size").set_value("2,2,0.01");
						ns.append_attribute("trigger").set_value(true);
					}
					else
					{
						auto nm = n.append_child("cMesh");
						nm.append_attribute("src").set_value((model_name + ".fmod").c_str());
						nm.append_attribute("sub_index").set_value(src->mMeshes[0]);
						if (name == "mesh_collider")
						{
							auto nr = n.append_child("cRigid");
							nr.append_attribute("dynamic").set_value(false);
							auto ns = n.append_child("cShape");
							ns.append_attribute("type").set_value("Mesh");
						}
					}
				}

				for (auto i = 0; i < src->mNumChildren; i++)
					print_node(n, src->mChildren[i]);
			};
			print_node(doc_prefab.append_child("prefab"), scene->mRootNode);

			auto prefab_path = filename;
			prefab_path.replace_extension(L".prefab");
			doc_prefab.save_file(prefab_path.c_str());

			for (auto i = 0; i < scene->mNumAnimations; i++)
			{
				auto ai_ani = scene->mAnimations[i];

				auto animation_name = model_name + "_" + std::string(ai_ani->mName.C_Str()) + ".fani";
				for (auto& ch : animation_name)
				{
					if (ch == '|')
						ch = '_';
				}
				auto animation_filename = ppath / animation_name;

				pugi::xml_document doc_animation;
				auto n_animation = doc_animation.append_child("animation");
				auto n_channels = n_animation.append_child("channels");

				auto data_filename = animation_filename;
				data_filename += L".dat";
				std::ofstream data_file(data_filename, std::ios::binary);

				for (auto j = 0; j < ai_ani->mNumChannels; j++)
				{
					auto ai_ch = ai_ani->mChannels[j];
					auto n_channel = n_channels.append_child("channel");
					n_channel.append_attribute("node_name").set_value(ai_ch->mNodeName.C_Str());
					fassert(ai_ch->mNumPositionKeys > 0 && ai_ch->mNumRotationKeys > 0 &&
						ai_ch->mNumPositionKeys == ai_ch->mNumRotationKeys);

					std::vector<BoneKey> keys;
					keys.resize(ai_ch->mNumPositionKeys);
					for (auto k = 0; k < keys.size(); k++)
					{
						auto& p = ai_ch->mPositionKeys[k].mValue;
						auto& q = ai_ch->mRotationKeys[k].mValue;
						keys[k].p = vec3(p.x, p.y, p.z);
						keys[k].q = quat(q.w, q.x, q.y, q.z);
					}
					auto n_keys = n_channel.append_child("keys");
					n_keys.append_attribute("offset").set_value(data_file.tellp());
					auto size = sizeof(BoneKey) * keys.size();
					n_keys.append_attribute("size").set_value(size);
					data_file.write((char*)keys.data(), size);
				}

				data_file.close();

				doc_animation.save_file(animation_filename.c_str());
			}
#endif
		}

		static ModelPrivate* standard_cube = nullptr;
		static ModelPrivate* standard_sphere = nullptr;

		Model* Model::get_standard(const wchar_t* _name)
		{
			auto name = std::wstring(_name);
			if (name == L"cube")
			{
				if (!standard_cube)
				{
					auto m = new ModelPrivate;
					auto mesh = new MeshPrivate;
					mesh->model = m;
					mesh->materials.push_back(default_material);
					mesh->add_cube(vec3(1.f), vec3(0.f), mat3(1.f));
					mesh->calc_bounds();
					m->meshes.emplace_back(mesh);

					standard_cube = m;
				}
				return standard_cube;
			}
			else if (name == L"sphere")
			{
				if (!standard_sphere)
				{
					auto m = new ModelPrivate;
					auto mesh = new MeshPrivate;
					mesh->model = m;
					mesh->materials.push_back(default_material);
					mesh->add_sphere(0.5f, 12, 12, vec3(0.f), mat3(1.f));
					mesh->calc_bounds();
					m->meshes.emplace_back(mesh);

					standard_sphere = m;
				}
				return standard_sphere;
			}
			return nullptr;
		}

		static std::vector<std::pair<std::filesystem::path, UniPtr<ModelPrivate>>> models;

		Model* Model::get(const wchar_t* _filename)
		{
			auto filename = std::filesystem::path(_filename);
			for (auto& m : models)
			{
				if (m.first == filename)
					return m.second.get();
			}

			if (!std::filesystem::exists(filename))
			{
				wprintf(L"cannot find model: %s\n", filename.c_str());
				return nullptr;
			}

			if (filename.extension() != L".fmod")
				return nullptr;

			pugi::xml_document doc;
			pugi::xml_node doc_root;
			if (!doc.load_file(filename.c_str()) || (doc_root = doc.first_child()).name() != std::string("model"))
			{
				printf("model does not exist: %s\n", filename.string().c_str());
				return nullptr;
			}

			auto model_data_filename = filename;
			model_data_filename += L".dat";
			std::ifstream model_data_file(model_data_filename, std::ios::binary);
			if (!model_data_file.good())
			{
				printf("missing .dat file for: %s\n", filename.string().c_str());
				return nullptr;
			}

			auto ret = new ModelPrivate();
			ret->filename = filename;
			auto ppath = filename.parent_path();

			for (auto& n_mesh : doc_root.child("meshes"))
			{
				auto m = new MeshPrivate;
				m->model = ret;
				for (auto& sp : SUS::split(n_mesh.attribute("material").value()))
				{
					auto material_filename = std::filesystem::path(sp);
					auto fn = ppath / material_filename;
					if (!std::filesystem::exists(fn))
						fn = material_filename;
					m->materials.push_back(MaterialPrivate::get(fn.c_str()));
				}

				auto n_positions = n_mesh.child("positions");
				{
					auto offset = n_positions.attribute("offset").as_uint();
					auto size = n_positions.attribute("size").as_uint();
					m->positions.resize(size / sizeof(vec3));
					model_data_file.read((char*)m->positions.data(), size);
				}

				auto n_uvs = n_mesh.child("uvs");
				if (n_uvs)
				{
					auto offset = n_uvs.attribute("offset").as_uint();
					auto size = n_uvs.attribute("size").as_uint();
					m->uvs.resize(size / sizeof(vec2));
					model_data_file.read((char*)m->uvs.data(), size);
				}

				auto n_normals = n_mesh.child("normals");
				if (n_normals)
				{
					auto offset = n_normals.attribute("offset").as_uint();
					auto size = n_normals.attribute("size").as_uint();
					m->normals.resize(size / sizeof(vec3));
					model_data_file.read((char*)m->normals.data(), size);
				}

				auto n_bids = n_mesh.child("bone_ids");
				if (n_bids)
				{
					auto offset = n_bids.attribute("offset").as_uint();
					auto size = n_bids.attribute("size").as_uint();
					m->bone_ids.resize(size / sizeof(ivec4));
					model_data_file.read((char*)m->bone_ids.data(), size);
				}

				auto n_wgts = n_mesh.child("bone_weights");
				if (n_wgts)
				{
					auto offset = n_wgts.attribute("offset").as_uint();
					auto size = n_wgts.attribute("size").as_uint();
					m->bone_weights.resize(size / sizeof(vec4));
					model_data_file.read((char*)m->bone_weights.data(), size);
				}

				auto n_indices = n_mesh.child("indices");
				{
					auto offset = n_indices.attribute("offset").as_uint();
					auto size = n_indices.attribute("size").as_uint();
					m->indices.resize(size / sizeof(uint));
					model_data_file.read((char*)m->indices.data(), size);
				}

				m->bounds = (AABB&)sto<2, 3, float>(n_mesh.attribute("bounds").value());

				ret->meshes.emplace_back(m);
			}

			for (auto n_bone : doc_root.child("bones"))
			{
				auto b = new BonePrivate;
				b->name = n_bone.attribute("name").value();
				{
					auto n_matrix = n_bone.child("offset_matrix");
					auto offset = n_matrix.attribute("offset").as_uint();
					auto size = n_matrix.attribute("size").as_uint();
					model_data_file.read((char*)&b->offset_matrix, size);
				}
				ret->bones.emplace_back(b);
			}

			model_data_file.close();

			models.emplace_back(filename, ret);

			return ret;
		}

		static std::vector<std::unique_ptr<AnimationPrivate>> animations;

		Animation* Animation::get(const wchar_t* _filename)
		{
			auto filename = std::filesystem::path(_filename);
			for (auto& a : animations)
			{
				if (a->filename == filename)
					return a.get();
			}

			pugi::xml_document doc;
			pugi::xml_node doc_root;
			if (!doc.load_file(filename.c_str()) || (doc_root = doc.first_child()).name() != std::string("animation"))
			{
				printf("animation does not exist: %s\n", filename.string().c_str());
				return nullptr;
			}

			auto data_filename = filename;
			data_filename += L".dat";
			std::ifstream data_file(data_filename, std::ios::binary);
			if (!data_file.good())
			{
				printf("missing .dat file for: %s\n", filename.string().c_str());
				return nullptr;
			}

			auto ret = new AnimationPrivate;
			ret->filename = filename;

			for (auto n_channel : doc_root.child("channels"))
			{
				auto c = new ChannelPrivate;
				c->node_name = n_channel.attribute("node_name").value();
				{
					auto n_keys = n_channel.child("keys");
					auto offset = n_keys.attribute("offset").as_uint();
					auto size = n_keys.attribute("size").as_uint();
					c->keys.resize(size / sizeof(BoneKey));
					data_file.read((char*)c->keys.data(), size);
				}
				ret->channels.emplace_back(c);
			}

			data_file.close();

			animations.emplace_back(ret);

			return ret;
		}
	}
}
