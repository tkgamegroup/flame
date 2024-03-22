#include "../serialize_extension.h"
#include "../foundation/bitmap.h"
#include "../foundation/typeinfo.h"
#include "../foundation/typeinfo_serialize.h"
#include "../foundation/system.h"
#include "material_private.h"
#include "model_private.h"
#include "model_ext.h"
#include "animation_private.h"

#ifdef USE_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

#ifdef USE_FBXSDK
#include <fbxsdk.h>
#endif

namespace flame
{
	namespace graphics
	{
		void MeshPrivate::save(const std::filesystem::path& filename)
		{
			std::ofstream dst(filename);
			dst << "mesh:" << std::endl;

			pugi::xml_document doc;
			auto n_mesh = doc.append_child("mesh");

			DataSoup data_soup;

			if (!positions.empty())
				data_soup.xml_append_v(positions, n_mesh.append_child("positions"));
			if (!uvs.empty())
				data_soup.xml_append_v(uvs, n_mesh.append_child("uvs"));
			if (!normals.empty())
				data_soup.xml_append_v(normals, n_mesh.append_child("normals"));
			if (!tangents.empty())
				data_soup.xml_append_v(tangents, n_mesh.append_child("tangents"));
			if (!colors.empty())
				data_soup.xml_append_v(colors, n_mesh.append_child("colors"));
			if (!bone_ids.empty())
				data_soup.xml_append_v(bone_ids, n_mesh.append_child("bone_ids"));
			if (!bone_weights.empty())
				data_soup.xml_append_v(bone_weights, n_mesh.append_child("bone_weights"));
			if (!indices.empty())
				data_soup.xml_append_v(indices, n_mesh.append_child("indices"));

			n_mesh.append_attribute("bounds").set_value(str((mat2x3&)bounds).c_str());

			doc.save(dst);
			dst << std::endl;

			dst << "data:" << std::endl;
			data_soup.save(dst);
		}

		std::filesystem::path find_file(const std::filesystem::path& dir, const std::filesystem::path& name)
		{
			if (std::filesystem::exists(name))
				return name;
			auto fn = name.filename();
			auto ret = dir / fn;
			if (std::filesystem::exists(ret))
				return ret;
			for (auto& it : std::filesystem::directory_iterator(dir))
			{
				if (std::filesystem::is_directory(it.status()))
				{
					ret = it.path() / fn;
					if (std::filesystem::exists(ret))
						return ret;
				}
			}
			return name;
		}

		struct MeshCreate : Mesh::Create
		{
			MeshPtr operator()() override
			{
				return new MeshPrivate;
			}
		}Mesh_create;
		Mesh::Create& Mesh::create = Mesh_create;

		static MeshPtr standard_plane = nullptr;
		static MeshPtr standard_cube = nullptr;
		static MeshPtr standard_sphere = nullptr;
		static MeshPtr standard_cylinder = nullptr;
		static MeshPtr standard_tri_prism = nullptr;

		static std::vector<std::unique_ptr<MeshT>> meshes;

		struct MeshGet : Mesh::Get
		{
			MeshPtr operator()(const std::filesystem::path& _filename) override
			{
				auto wstr = _filename.wstring();
				if (SUW::strip_head_if(wstr, L"standard_"))
				{
					if (wstr == L"plane")
					{
						if (!standard_plane)
						{
							auto m = new MeshPrivate;
							m->filename = L"standard_plane";
							{
								m->positions.push_back(vec3(0.f));
								m->normals.push_back(vec3(0.f, 1.f, 0.f));
								m->positions.push_back(vec3(10.f, 0.f, 0.f));
								m->normals.push_back(vec3(0.f, 1.f, 0.f));
								m->positions.push_back(vec3(10.f, 0.f, 10.f));
								m->normals.push_back(vec3(0.f, 1.f, 0.f));
								m->positions.push_back(vec3(0.f, 0.f, 10.f));
								m->normals.push_back(vec3(0.f, 1.f, 0.f));

								m->indices.push_back(0);
								m->indices.push_back(3);
								m->indices.push_back(1);
								m->indices.push_back(1);
								m->indices.push_back(3);
								m->indices.push_back(2);
							}
							m->calc_bounds();

							standard_plane = m;
						}
						return standard_plane;
					}
					else if (wstr == L"cube")
					{
						if (!standard_cube)
						{
							auto m = new MeshPrivate;
							m->filename = L"standard_cube";
							mesh_add_cube(*m, vec3(1.f), vec3(0.f, 0.5f, 0.f), mat3(1.f));
							m->calc_bounds();

							standard_cube = m;
						}
						return standard_cube;
					}
					else if (wstr == L"sphere")
					{
						if (!standard_sphere)
						{
							auto m = new MeshPrivate;
							m->filename = L"standard_sphere";
							mesh_add_sphere(*m, 0.5f, 12, 12, vec3(0.f, 0.5f, 0.f), mat3(1.f));
							m->calc_bounds();

							standard_sphere = m;
						}
						return standard_sphere;
					}
					else if (wstr == L"cylinder")
					{
						if (!standard_cylinder)
						{
							auto m = new MeshPrivate;
							m->filename = L"standard_cylinder";
							mesh_add_cylinder(*m, 0.5f, 1.f, 12, vec3(0.f, 0.5f, 0.f));
							m->calc_bounds();

							standard_cylinder = m;
						}
						return standard_cylinder;
					}
					else if (wstr == L"tri_prism")
					{
						if (!standard_tri_prism)
						{
							auto m = new MeshPrivate;
							m->filename = L"standard_tri_prism";
							{
								auto lt_0 = vec3(0.f, 0.f, -0.5f);
								auto lt_1 = vec3(0.f, 1.f, -0.5f);
								auto lt_2 = vec3(1.f, 0.f, -0.5f);
								auto rt_0 = vec3(0.f, 0.f, +0.5f);
								auto rt_1 = vec3(0.f, 1.f, +0.5f);
								auto rt_2 = vec3(1.f, 0.f, +0.5f);

								m->positions.push_back(lt_0);
								m->normals.push_back(vec3(0.f, 0.f, -1.f));
								m->positions.push_back(lt_1);
								m->normals.push_back(vec3(0.f, 0.f, -1.f));
								m->positions.push_back(lt_2);
								m->normals.push_back(vec3(0.f, 0.f, -1.f));
								
								m->indices.push_back(0);
								m->indices.push_back(1);
								m->indices.push_back(2);

								m->positions.push_back(rt_0);
								m->normals.push_back(vec3(0.f, 0.f, +1.f));
								m->positions.push_back(rt_1);
								m->normals.push_back(vec3(0.f, 0.f, +1.f));
								m->positions.push_back(rt_2);
								m->normals.push_back(vec3(0.f, 0.f, +1.f));

								m->indices.push_back(3);
								m->indices.push_back(5);
								m->indices.push_back(4);

								m->positions.push_back(lt_0);
								m->normals.push_back(vec3(-1.f, 0.f, 0.f));
								m->positions.push_back(lt_1);
								m->normals.push_back(vec3(-1.f, 0.f, 0.f));
								m->positions.push_back(rt_0);
								m->normals.push_back(vec3(-1.f, 0.f, 0.f));
								m->positions.push_back(rt_1);
								m->normals.push_back(vec3(-1.f, 0.f, 0.f));

								m->indices.push_back(6);
								m->indices.push_back(9);
								m->indices.push_back(7);
								m->indices.push_back(6);
								m->indices.push_back(8);
								m->indices.push_back(9);

								auto n = normalize(cross(normalize(lt_2 - lt_1), vec3(0.f, 0.f, -1.f)));
								m->positions.push_back(lt_1);
								m->normals.push_back(n);
								m->positions.push_back(lt_2);
								m->normals.push_back(n);
								m->positions.push_back(rt_1);
								m->normals.push_back(n);
								m->positions.push_back(rt_2);
								m->normals.push_back(n);

								m->indices.push_back(10);
								m->indices.push_back(13);
								m->indices.push_back(11);
								m->indices.push_back(10);
								m->indices.push_back(12);
								m->indices.push_back(13);

								m->positions.push_back(lt_2);
								m->normals.push_back(vec3(0.f, -1.f, 0.f));
								m->positions.push_back(lt_0);
								m->normals.push_back(vec3(0.f, -1.f, 0.f));
								m->positions.push_back(rt_2);
								m->normals.push_back(vec3(0.f, -1.f, 0.f));
								m->positions.push_back(rt_0);
								m->normals.push_back(vec3(0.f, -1.f, 0.f));

								m->indices.push_back(14);
								m->indices.push_back(17);
								m->indices.push_back(15);
								m->indices.push_back(14);
								m->indices.push_back(16);
								m->indices.push_back(17);


							}
							m->calc_bounds();

							standard_tri_prism = m;
						}
						return standard_tri_prism;
					}
					return nullptr;
				}

				auto filename = Path::get(_filename);

				for (auto& m : meshes)
				{
					if (m->filename == filename)
					{
						m->ref++;
						return m.get();
					}
				}

				std::ifstream file(filename);
				if (!file.good())
				{
					wprintf(L"cannot find armature: %s\n", _filename.c_str());
					return nullptr;
				}

				auto ret = new MeshPrivate();
				ret->filename = filename;

				LineReader src(file);
				src.read_block("mesh:");

				pugi::xml_document doc;
				pugi::xml_node doc_root;
				if (!doc.load_string(src.to_string().c_str()) || (doc_root = doc.first_child()).name() != std::string("mesh"))
				{
					wprintf(L"mesh format is incorrect: %s\n", _filename.c_str());
					delete ret;
					return nullptr;
				}

				DataSoup data_soup;
				src.read_block("data:");
				data_soup.load(src);

				data_soup.xml_read_v(ret->positions, doc_root.child("positions"));
				if (auto n_uvs = doc_root.child("uvs"); n_uvs)
					data_soup.xml_read_v(ret->uvs, n_uvs);
				if (auto n_normals = doc_root.child("normals"); n_normals)
					data_soup.xml_read_v(ret->normals, n_normals);
				if (auto n_tangents = doc_root.child("tangents"); n_tangents)
					data_soup.xml_read_v(ret->tangents, n_tangents);
				if (auto n_colors = doc_root.child("colors"); n_colors)
					data_soup.xml_read_v(ret->colors, n_colors);
				if (auto n_bids = doc_root.child("bone_ids"); n_bids)
					data_soup.xml_read_v(ret->bone_ids, n_bids);
				if (auto n_wgts = doc_root.child("bone_weights"); n_wgts)
					data_soup.xml_read_v(ret->bone_weights, n_wgts);
				data_soup.xml_read_v(ret->indices, doc_root.child("indices"));

				ret->bounds = (AABB&)s2t<2, 3, float>(doc_root.attribute("bounds").value());

				ret->ref = 1;
				meshes.emplace_back(ret);
				return ret;
			}
		}Mesh_get;
		Mesh::Get& Mesh::get = Mesh_get;

		struct MeshRelease : Mesh::Release
		{
			void operator()(MeshPtr mesh) override
			{
				if (mesh->filename.wstring().starts_with(L"standard:"))
					return;
				if (mesh->ref == 1)
				{
					std::erase_if(meshes, [&](const auto& i) {
						return i.get() == mesh;
					});
				}
				else
					mesh->ref--;
			}
		}Mesh_release;
		Mesh::Release& Mesh::release = Mesh_release;
		void ArmaturePrivate::save(const std::filesystem::path& filename)
		{
			std::ofstream dst(filename);
			dst << "armature:" << std::endl;

			pugi::xml_document doc;
			auto n_armature = doc.append_child("armature");

			DataSoup data_soup;

			auto n_bones = n_armature.append_child("bones");
			for (auto& b : bones)
			{
				auto n_bone = n_bones.append_child("bone");
				n_bone.append_attribute("name").set_value(b.name.c_str());
				data_soup.xml_append((uint*)&b.offset_matrix, sizeof(b.offset_matrix), n_bone.append_child("offset_matrix"));
			}

			doc.save(dst);
			dst << std::endl;

			dst << "data:" << std::endl;
			data_soup.save(dst);
		}

		struct ArmatureCreate : Armature::Create
		{
			ArmaturePtr operator()() override
			{
				return new ArmaturePrivate;
			}
		}Armature_create;
		Armature::Create& Armature::create = Armature_create;

		static std::vector<std::unique_ptr<ArmatureT>> armatures;

		struct ArmatureGet : Armature::Get
		{
			ArmaturePtr operator()(const std::filesystem::path& _filename) override
			{
				auto filename = Path::get(_filename);

				for (auto& a : armatures)
				{
					if (a->filename == filename)
					{
						a->ref++;
						return a.get();
					}
				}

				std::ifstream file(filename);
				if (!file.good())
				{
					wprintf(L"cannot find armature: %s\n", _filename.c_str());
					return nullptr;
				}

				auto ret = new ArmaturePrivate();
				ret->filename = filename;

				LineReader src(file);
				src.read_block("armature:");

				pugi::xml_document doc;
				pugi::xml_node doc_root;
				if (!doc.load_string(src.to_string().c_str()) || (doc_root = doc.first_child()).name() != std::string("armature"))
				{
					wprintf(L"armature format is incorrect: %s\n", _filename.c_str());
					delete ret;
					return nullptr;
				}

				DataSoup data_soup;
				src.read_block("data:");
				data_soup.load(src);

				for (auto n_bone : doc_root.child("bones"))
				{
					auto& b = ret->bones.emplace_back();
					b.name = n_bone.attribute("name").value();
					data_soup.xml_read(&b.offset_matrix, n_bone.child("offset_matrix"));
				}

				ret->ref = 1;
				armatures.emplace_back(ret);
				return ret;
			}
		}Armature_get;
		Armature::Get& Armature::get = Armature_get;

		struct ArmatureRelease : Armature::Release
		{
			void operator()(ArmaturePtr arm) override
			{
				if (arm->filename.wstring().starts_with(L"standard:"))
					return;
				if (arm->ref == 1)
				{
					std::erase_if(armatures, [&](const auto& i) {
						return i.get() == arm;
					});
				}
				else
					arm->ref--;
			}
		}Armature_release;
		Armature::Release& Armature::release = Armature_release;

		void import_scene(const std::filesystem::path& _filename, const std::filesystem::path& _destination, const vec3& rotation, float scaling, bool only_animations)
		{
			auto filename = Path::get(_filename);
			if (!std::filesystem::exists(filename))
			{
				wprintf(L"import scene: cannot find file: %s", _filename.c_str());
				return;
			}

			auto parent_path = filename.parent_path();
			auto destination = _destination;
			if (destination.empty())
				destination = parent_path;
			auto textures_destination = destination / L"textures";
			auto materials_destination = destination / L"materials";
			auto models_destination = destination / L"models";
			auto meshes_destination = destination / L"meshes";
			auto armatures_destination = destination / L"armatures";
			auto animations_destination = destination / L"animations";
			auto ext = SUW::get_lowered(filename.extension().wstring());
			filename.replace_extension(L".fmod");
			auto model_name_with_ext = filename.filename().string();
			auto model_name = filename.filename().stem().string();

			struct MaterialData
			{
				std::string name;
				std::filesystem::path filename;

				vec4 color;
				int color_map;
				int alpha_map;
				std::vector<FileTexture> textures;
			};

			struct MeshData
			{
				std::string name;
				std::filesystem::path filename;

				std::vector<vec3>	positions;
				std::vector<vec2>	uvs;
				std::vector<vec3>	normals;
				std::vector<vec3>	tangents;
				std::vector<cvec4>	colors;
				std::vector<ivec4>	bone_ids;
				std::vector<vec4>	bone_weights;
				std::vector<uint>	indices;
			};

			struct ArmatureData
			{
				std::string name;
				std::filesystem::path filename;

				std::vector<Bone> bones;

				inline int find_bone(std::string_view name)
				{
					for (auto i = 0; i < bones.size(); i++)
					{
						if (bones[i].name == name)
							return i;
					}
					return -1;
				};
			};

			struct AnimationData
			{
				std::string name;
				std::filesystem::path filename;

				float duration;
				std::vector<Channel> channels;

				inline Channel& get_channel(const std::string& node_name)
				{
					for (auto& c : channels)
					{
						if (c.node_name == node_name)
							return c;
					}
					auto& ch = channels.emplace_back();
					ch.node_name = node_name;
					return ch;
				}
			};

			std::vector<MaterialData> materials;
			std::vector<MeshData> meshes;
			std::vector<ArmatureData> armatures;
			std::vector<AnimationData> animations;
			pugi::xml_document doc_prefab;

			auto find_material = [&](const std::string& name) ->MaterialData* {
				for (auto& m : materials)
				{
					if (m.name == name)
						return &m;
				}
				return nullptr;
			};

			auto new_material = [&](const std::string& name) ->MaterialData& {
				auto& m = materials.emplace_back();
				m.name = name;
				m.filename = Path::reverse(materials_destination / (model_name + '_' + name + ".fmat"));
				return m;
			};

			auto new_mesh = [&](const std::string& name) ->MeshData& {
				auto& m = meshes.emplace_back();
				m.name = name;
				m.filename = Path::reverse(meshes_destination / (model_name + '_' + name + ".fmesh"));
				return m;
			};

			auto new_armature = [&](const std::string& name) ->ArmatureData& {
				auto& a = armatures.emplace_back();
				a.name = name;
				a.filename = Path::reverse(armatures_destination / (model_name + '_' + name + ".farm"));
				return a;
			};

			auto new_animation = [&](const std::string& name, float duration) ->AnimationData& {
				auto& a = animations.emplace_back();
				a.name = name;
				a.duration = duration;
				a.filename = Path::reverse(animations_destination / (model_name + '_' + name + ".fani"));
				return a;
			};

			auto format_name = [&](const std::string& name) {
				auto ret = name;
				SUS::replace_char(ret, ' ', '_');
				SUS::replace_char(ret, ':', '_');
				SUS::replace_char(ret, '|', '_');
				SUS::replace_char(ret, '#', '_');
				return ret;
			};

			auto add_bone_weight = [](ivec4& ids, vec4& weights, uint bid, float w) {
				auto idx = -1;
				for (auto i = 0; i < 4; i++)
				{
					if (ids[i] == -1)
					{
						idx = i;
						break;
					}
				}
				if (idx == -1)
				{
					auto mi = 0; auto mv = weights[0];
					for (auto i = 1; i < 4; i++)
					{
						if (weights[i] < mv)
						{
							mi = i;
							mv = weights[i];
						}
					}
					if (weights[mi] < w)
						idx = mi;
				}
				if (idx != -1)
				{
					ids[idx] = bid;
					weights[idx] = w;
				}
			};

			auto normalize_weights = [](std::vector<ivec4>& bone_ids, std::vector<vec4>& weights) {
				for (auto i = 0; i < bone_ids.size(); i++)
				{
					auto n = 0;
					auto ids = bone_ids[i];
					for (auto j = 0; j < 4; j++)
					{
						if (ids[j] == -1)
							break;
						n++;
					}
					auto& w = weights[i];
					switch (n)
					{
					case 1: w = vec4(1.f, 0.f, 0.f, 0.f);				break;
					case 2: w = vec4(w.xy() / (w.x + w.y), 0.f, 0.f);	break;
					case 3: w = vec4(w.xyz() / (w.x + w.y + w.z), 0.f);	break;
					case 4: w /= (w.x + w.y + w.z + w.w);				break;
					}
				}
			};

			auto preprocess = [&](pugi::xml_node first_node)->pugi::xml_node {
				if (rotation != vec3(0.f) || scaling != 1.f)
				{
					first_node.append_attribute("file_id").set_value(generate_guid().to_string().c_str());
					auto n_node = first_node.append_child("components").append_child("item");
					n_node.append_attribute("type_name").set_value("flame::cNode");
					return first_node.append_child("children").append_child("item");
				}
				return first_node;
			};

			auto add_animator = [&](pugi::xml_node n_components, ArmatureData* arm) {
				auto n_armature = n_components.append_child("item");
				n_armature.append_attribute("type_name").set_value("flame::cAnimator");
				if (arm)
					n_armature.append_attribute("armature_name").set_value(arm->filename.string().c_str());
				if (!animations.empty())
				{
					auto default_animation = "idle";
					auto found_default = false;

					auto n_animation_names = n_armature.append_child("animation_names");
					for (auto& a : animations)
					{
						auto n_item = n_animation_names.append_child("item");
						n_item.append_attribute("first").set_value(a.filename.string().c_str());
						n_item.append_attribute("second").set_value(a.name.c_str());

						if (a.name == default_animation)
							found_default = true;
					}

					if (found_default)
						n_armature.append_attribute("default_animation").set_value("idle"_h);
				}
			};

#ifdef USE_FBXSDK
			if (ext == L".fbx")
			{
				auto get_geometry = [](fbxsdk::FbxNode* pNode) {
					const FbxVector4 lT = pNode->GetGeometricTranslation(fbxsdk::FbxNode::eSourcePivot);
					const FbxVector4 lR = pNode->GetGeometricRotation(fbxsdk::FbxNode::eSourcePivot);
					const FbxVector4 lS = pNode->GetGeometricScaling(fbxsdk::FbxNode::eSourcePivot);

					return FbxAMatrix(lT, lR, lS);
				};

				auto get_quat = [](FbxEuler::EOrder ro, const vec3& e)->quat {
					auto make_quat = [&](int _1, int _2, int _3) {
						vec3 axis[3] = {
							vec3(1.f, 0.f, 0.f),
							vec3(0.f, 1.f, 0.f),
							vec3(0.f, 0.f, 1.f)
						};

						return
							angleAxis(radians((float)e[_3]), axis[_3])
							* angleAxis(radians((float)e[_2]), axis[_2])
							* angleAxis(radians((float)e[_1]), axis[_1]);
					};

					switch (ro)
					{
					case FbxEuler::eOrderXYZ: { return make_quat(0, 1, 2); } break;
					case FbxEuler::eOrderXZY: { return make_quat(0, 2, 1); } break;
					case FbxEuler::eOrderYZX: { return make_quat(1, 2, 0); } break;
					case FbxEuler::eOrderYXZ: { return make_quat(1, 0, 2); } break;
					case FbxEuler::eOrderZXY: { return make_quat(2, 0, 1); } break;
					case FbxEuler::eOrderZYX: { return make_quat(2, 1, 0); } break;
					}
				};

				auto to_glm = [](const FbxAMatrix& src) {
					return mat4(
						vec4(src.Get(0, 0), src.Get(0, 1), src.Get(0, 2), src.Get(0, 3)),
						vec4(src.Get(1, 0), src.Get(1, 1), src.Get(1, 2), src.Get(1, 3)),
						vec4(src.Get(2, 0), src.Get(2, 1), src.Get(2, 2), src.Get(2, 3)),
						vec4(src.Get(3, 0), src.Get(3, 1), src.Get(3, 2), src.Get(3, 3))
					);
				};

				static FbxManager* sdk_manager = nullptr;
				if (!sdk_manager)
				{
					sdk_manager = FbxManager::Create();
					if (!sdk_manager)
					{
						printf("FBX SDK: Unable to create FBX Manager!\n");
						return;
					}

					auto ios = FbxIOSettings::Create(sdk_manager, IOSROOT);
					sdk_manager->SetIOSettings(ios);
				}

				auto scene = FbxScene::Create(sdk_manager, "Scene");
				if (!scene)
				{
					printf("FBX SDK: Unable to create FBX scene!\n");
					return;
				}

				auto importer = FbxImporter::Create(sdk_manager, "");
				if (!importer->Initialize(_filename.string().c_str(), -1, sdk_manager->GetIOSettings()))
				{
					wprintf(L"cannot import model %s: ", _filename.c_str());
					printf("%s\n", importer->GetStatus().GetErrorString());
					return;
				}

				if (!importer->Import(scene))
				{
					wprintf(L"cannot import model %s: ", _filename.c_str());
					printf("%s\n", importer->GetStatus().GetErrorString());
					return;
				}

				importer->Destroy();

				FbxAxisSystem our_axis_system(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);
				if (scene->GetGlobalSettings().GetAxisSystem() != our_axis_system)
					our_axis_system.ConvertScene(scene);
				FbxSystemUnit our_system_unit(100.0);
				our_system_unit.ConvertScene(scene);

				auto scene_root = scene->GetRootNode();

				FbxArray<FbxString*> anim_names;
				scene->FillAnimStackNameArray(anim_names);
				auto anim_count = anim_names.GetCount();
				for (auto i = 0; i < anim_count; i++)
				{
					auto anim_stack = scene->FindMember<fbxsdk::FbxAnimStack>(anim_names[i]->Buffer());
					auto layer = anim_stack->GetMember<FbxAnimLayer>(0);
					scene->SetCurrentAnimationStack(anim_stack);

					float start_time, stop_time;
					if (auto take_info = scene->GetTakeInfo(*(anim_names[i])); take_info)
					{
						FbxTime fbx_time;
						fbx_time = take_info->mLocalTimeSpan.GetStart();
						start_time = fbx_time.GetMilliSeconds() / 1000.f;
						fbx_time = take_info->mLocalTimeSpan.GetStop();
						stop_time = fbx_time.GetMilliSeconds() / 1000.f;
					}
					else
					{
						FbxTimeSpan timeLine_timespan;
						scene->GetGlobalSettings().GetTimelineDefaultTimeSpan(timeLine_timespan);
						FbxTime fbx_time;
						fbx_time = timeLine_timespan.GetStart();
						start_time = fbx_time.GetMilliSeconds() / 1000.f;
						fbx_time = timeLine_timespan.GetStop();
						stop_time = fbx_time.GetMilliSeconds() / 1000.f;
					}

					std::string ani_name = format_name(anim_stack->GetName());
					SUS::strip_head_if(ani_name, "Armature_");
					auto& ani = new_animation(ani_name, stop_time - start_time);

					std::function<void(fbxsdk::FbxNode*, float)> get_node_animation;
					get_node_animation = [&](fbxsdk::FbxNode* node, float time) {
						FbxTime fbx_time;
						fbx_time.SetMilliSeconds(time * 1000.f);
						auto transform = to_glm(node->EvaluateLocalTransform(fbx_time) * get_geometry(node));

						if (auto node_attr = node->GetNodeAttribute(); node_attr)
						{
							auto node_type = node_attr->GetAttributeType();
							if (node_type == FbxNodeAttribute::eSkeleton || node_type == FbxNodeAttribute::eMesh)
							{
								auto& channel = ani.get_channel(node->GetName());

								vec3 pos; quat qut; vec3 scl; vec3 skew; vec4 perspective;
								decompose(transform, scl, qut, pos, skew, perspective);
								channel.position_keys.push_back({ time, pos });
								channel.rotation_keys.push_back({ time, qut });
								channel.scaling_keys.push_back({ time, scl });
							}
						}

						auto children_count = node->GetChildCount();
						for (auto i = 0; i < children_count; i++)
							get_node_animation(node->GetChild(i), time);
					};

					for (auto t = start_time; t < stop_time; t += 1.f / 24.f)
						get_node_animation(scene_root, t);

					if (ani.channels.empty())
						animations.pop_back();
					else
					{
						for (auto& ch : ani.channels)
						{
							for (auto& k : ch.position_keys)
								k.t -= start_time;
							for (auto& k : ch.rotation_keys)
								k.t -= start_time;
							for (auto& k : ch.scaling_keys)
								k.t -= start_time;
						}
					}
				}
				scene->SetCurrentAnimationStack(nullptr);

				if (!only_animations)
				{
					FbxGeometryConverter geom_converter(sdk_manager);
					try
					{
						geom_converter.Triangulate(scene, true);
					}
					catch (std::runtime_error)
					{
						scene->Destroy();
						wprintf(L"FBX SDK: cannot triangulate %s\n", _filename.c_str());
						return;
					}

					std::function<void(pugi::xml_node, fbxsdk::FbxNode*)> process_node;
					process_node = [&](pugi::xml_node dst, fbxsdk::FbxNode* src) {
						dst.append_attribute("file_id").set_value(generate_guid().to_string().c_str());
						dst.append_attribute("name").set_value(src->GetName());

						auto n_components = dst.append_child("components");
						pugi::xml_node n_children;

						{
							auto n_node = n_components.append_child("item");
							n_node.append_attribute("type_name").set_value("flame::cNode");

							auto mat = to_glm(src->EvaluateGlobalTransform());
							if (auto parent = src->GetParent(); parent)
								mat = inverse(to_glm(parent->EvaluateGlobalTransform())) * mat;
							vec3 pos; quat qut; vec3 scl; vec3 skew; vec4 perspective;
							decompose(mat, scl, qut, pos, skew, perspective);

							if (src == scene_root)
							{
								//if (rotation != vec3(0.f))
								//	n_node.append_attribute("eul").set_value(str(rotation).c_str());
								if (scaling != 1.f)
									scl *= scaling;
							}

							if (pos != vec3(0.f))
								n_node.append_attribute("pos").set_value(str(pos).c_str());
							if (qut != quat(1.f, 0.f, 0.f, 0.f))
								n_node.append_attribute("qut").set_value(str(*(vec4*)&qut).c_str());
							if (scl != vec3(1.f))
								n_node.append_attribute("scl").set_value(str(scl).c_str());
						}

						auto material_count = src->GetMaterialCount();
						for (auto i = 0; i < material_count; i++)
						{
							auto fbx_mat = src->GetMaterial(i);
							auto mat_name = format_name(fbx_mat->GetName());

							if (!find_material(mat_name))
							{
								auto& mat = new_material(mat_name);
								auto map_id = 0;

								if (auto prop = fbx_mat->FindProperty(fbxsdk::FbxSurfaceMaterial::sDiffuse); prop.IsValid())
								{
									auto res = prop.Get<fbxsdk::FbxDouble3>();
									mat.color[0] = res[0];
									mat.color[1] = res[1];
									mat.color[2] = res[2];

									if (auto tex = prop.GetSrcObjectCount<FbxFileTexture>() ? prop.GetSrcObject<FbxFileTexture>() : nullptr; tex)
									{
										if (std::string tex_name = tex->GetFileName(); !tex_name.empty())
										{
											auto fn = find_file(parent_path, tex_name);
											if (std::filesystem::is_regular_file(fn))
											{
												auto dst = fn.filename();
												dst = textures_destination / dst;
												if (dst != fn)
												{
													if (!std::filesystem::exists(textures_destination))
														std::filesystem::create_directories(textures_destination);
													std::filesystem::copy_file(fn, dst, std::filesystem::copy_options::overwrite_existing);
													auto config_path = dst;
													config_path += L".ini";
													std::ofstream config(config_path);
													config << "srgb=true\n";
													config << "auto_mipmapping=true\n";
													config.close();
													fn = dst;
												}
											}

											auto& texture = mat.textures.emplace_back();
											texture.filename = Path::rebase(materials_destination, fn);
											mat.color_map = map_id++;
										}
									}
								}
							}
						}

						if (auto node_attr = src->GetNodeAttribute(); node_attr)
						{
							auto node_type = node_attr->GetAttributeType();
							if (node_type == fbxsdk::FbxNodeAttribute::eMesh)
							{
								auto fbx_mesh = src->GetMesh();
								auto mesh_name = fbx_mesh->GetName();
								auto polygon_count = fbx_mesh->GetPolygonCount();

								auto mesh_idx = meshes.size();
								for (auto i = 0; i < max(1, material_count); i++)
									new_mesh(material_count > 1 ? mesh_name + '_' + str(i) : mesh_name);

								fbxsdk::FbxLayerElementArrayTemplate<int> empty_index_array(fbxsdk::eFbxUndefined);
								fbxsdk::FbxLayerElementArrayTemplate<fbxsdk::FbxVector2> empty_vector2_array(fbxsdk::eFbxUndefined);
								fbxsdk::FbxLayerElementArrayTemplate<fbxsdk::FbxVector4> empty_vector4_array(fbxsdk::eFbxUndefined);
								fbxsdk::FbxLayerElementArrayTemplate<fbxsdk::FbxColor> empty_color_array(fbxsdk::eFbxUndefined);

								auto element_mat = fbx_mesh->GetElementMaterialCount() > 0 ? fbx_mesh->GetElementMaterial() : nullptr;
								auto element_uvs = fbx_mesh->GetElementUVCount() > 0 ? fbx_mesh->GetElementUV(0) : nullptr;
								auto element_normals = fbx_mesh->GetElementNormalCount() > 0 ? fbx_mesh->GetElementNormal(0) : nullptr;
								auto element_tangents = fbx_mesh->GetElementTangentCount() > 0 ? fbx_mesh->GetElementTangent(0) : nullptr;
								auto element_colors = fbx_mesh->GetElementVertexColorCount() > 0 ? fbx_mesh->GetElementVertexColor(0) : nullptr;
								auto mat_mapping_mode = element_mat ? element_mat->GetMappingMode() : FbxGeometryElement::eNone;
								auto& mat_index_array = element_mat ? element_mat->GetIndexArray() : empty_index_array;
								auto uv_mapping_mode = element_uvs ? element_uvs->GetMappingMode() : FbxGeometryElement::eNone;
								auto uv_reference_mode = element_uvs ? element_uvs->GetReferenceMode() : FbxGeometryElement::eNone;
								auto normal_mapping_mode = element_normals ? element_normals->GetMappingMode() : FbxGeometryElement::eNone;
								auto normal_reference_mode = element_normals ? element_normals->GetReferenceMode() : FbxGeometryElement::eNone;
								auto tangent_mapping_mode = element_tangents ? element_tangents->GetMappingMode() : FbxGeometryElement::eNone;
								auto tangent_reference_mode = element_tangents ? element_tangents->GetReferenceMode() : FbxGeometryElement::eNone;
								auto color_mapping_mode = element_colors ? element_colors->GetMappingMode() : FbxGeometryElement::eNone;
								auto color_reference_mode = element_colors ? element_colors->GetReferenceMode() : FbxGeometryElement::eNone;
								auto& uv_direct_array = element_uvs ? element_uvs->GetDirectArray() : empty_vector2_array;
								auto& uv_index_array = element_uvs ? element_uvs->GetIndexArray() : empty_index_array;
								auto& normal_direct_array = element_normals ? element_normals->GetDirectArray() : empty_vector4_array;
								auto& normal_index_array = element_normals ? element_normals->GetIndexArray() : empty_index_array;
								auto& tangent_direct_array = element_tangents ? element_tangents->GetDirectArray() : empty_vector4_array;
								auto& tangent_index_array = element_tangents ? element_tangents->GetIndexArray() : empty_index_array;
								auto& color_direct_array = element_colors ? element_colors->GetDirectArray() : empty_color_array;
								auto& color_index_array = element_colors ? element_colors->GetIndexArray() : empty_index_array;

								std::string uv_name;
								if (element_uvs)
								{
									FbxStringList names;
									fbx_mesh->GetUVSetNames(names);
									uv_name = names[0].Buffer();
								}

								auto control_points = fbx_mesh->GetControlPoints();
								auto control_points_count = fbx_mesh->GetControlPointsCount();
								std::vector<ivec4> control_point_bone_ids;
								std::vector<vec4> control_point_bone_weights;
								if (auto skin_count = fbx_mesh->GetDeformerCount(FbxDeformer::eSkin); skin_count > 0)
								{
									ArmatureData* arm = nullptr;

									control_point_bone_ids.resize(control_points_count);
									control_point_bone_weights.resize(control_points_count);
									for (auto i = 0; i < control_points_count; i++)
									{
										control_point_bone_ids[i] = ivec4(-1);
										control_point_bone_weights[i] = vec4(0.f);
									}
									for (auto i = 0; i < skin_count; i++)
									{
										auto skin_deformer = (FbxSkin*)fbx_mesh->GetDeformer(i, FbxDeformer::eSkin);
										int cluster_count = skin_deformer->GetClusterCount();
										for (int j = 0; j < cluster_count; j++)
										{
											auto cluster = skin_deformer->GetCluster(j);
											auto link = cluster->GetLink();
											if (!link)
												continue;

											std::string name = link->GetName();
											if (!arm)
												arm = &new_armature(name);

											auto bone_idx = arm->find_bone(name);
											if (bone_idx == -1)
											{
												bone_idx = arm->bones.size();
												auto& b = arm->bones.emplace_back();
												b.name = name;
												b.offset_matrix = mat4(1.f);
											}
											if (bone_idx != -1)
											{
												auto& bone = arm->bones[bone_idx];

												FbxAMatrix reference_init;
												FbxAMatrix cluster_init;
												cluster->GetTransformMatrix(reference_init);
												reference_init *= get_geometry(src);
												cluster->GetTransformLinkMatrix(cluster_init);
												bone.offset_matrix = to_glm(cluster_init.Inverse() * reference_init);

												int vertex_idx_count = cluster->GetControlPointIndicesCount();
												for (int k = 0; k < vertex_idx_count; k++)
												{
													auto control_idx = cluster->GetControlPointIndices()[k];
													if (control_idx >= control_points_count)
														continue;

													auto weight = (float)cluster->GetControlPointWeights()[k];
													if (weight <= 0.f)
														continue;

													add_bone_weight(control_point_bone_ids[control_idx], control_point_bone_weights[control_idx], bone_idx, weight);
												}
											}
										}
									}

									normalize_weights(control_point_bone_ids, control_point_bone_weights);

									if (auto pnode = dst.parent().parent(); pnode)
										add_animator(pnode.child("components"), arm);
								}

								auto vertex_count = 0;
								for (auto i = 0; i < polygon_count; i++)
								{
									auto mat_idx = element_mat && mat_mapping_mode == FbxGeometryElement::eByPolygon ?
										max(0, mat_index_array.GetAt(i)) : 0;

									auto& m = meshes[mesh_idx + mat_idx];
									for (auto j = 0; j < 3; j++)
									{
										auto control_point_idx = fbx_mesh->GetPolygonVertex(i, j);
										if (control_point_idx >= 0)
										{
											m.indices.push_back(m.positions.size());

											auto vtx = control_points[control_point_idx];
											m.positions.push_back(vec3(vtx[0], vtx[1], vtx[2]));

											if (element_uvs)
											{
												switch (uv_mapping_mode)
												{
												case FbxGeometryElement::eByControlPoint:
													switch (uv_reference_mode)
													{
													case FbxGeometryElement::eDirect:
													{
														auto src = uv_direct_array.GetAt(control_point_idx);
														m.uvs.push_back(vec2(src[0], 1.f - src[1]));
													}
														break;
													case FbxGeometryElement::eIndexToDirect:
													{
														auto idx = uv_index_array.GetAt(control_point_idx);
														auto src = uv_direct_array.GetAt(idx);
														m.uvs.push_back(vec2(src[0], 1.f - src[1]));
													}
														break;
													}
													break;
												case FbxGeometryElement::eByPolygonVertex:
													switch (uv_reference_mode)
													{
													case FbxGeometryElement::eDirect:
													{
														auto src = uv_direct_array.GetAt(vertex_count);
														m.uvs.push_back(vec2(src[0], 1.f - src[1]));
													}
														break;
													case FbxGeometryElement::eIndexToDirect:
													{
														auto idx = uv_index_array.GetAt(vertex_count);
														auto src = uv_direct_array.GetAt(idx);
														m.uvs.push_back(vec2(src[0], 1.f - src[1]));
													}
														break;
													}
													break;
												}
											}

											if (element_normals)
											{
												switch (normal_mapping_mode)
												{
												case FbxGeometryElement::eByControlPoint:
													switch (normal_reference_mode)
													{
													case FbxGeometryElement::eDirect:
													{
														auto src = normal_direct_array.GetAt(control_point_idx);
														m.normals.push_back(vec3(src[0], src[1], src[2]));
													}
														break;
													case FbxGeometryElement::eIndexToDirect:
													{
														auto idx = normal_index_array.GetAt(control_point_idx);
														auto src = normal_direct_array.GetAt(idx);
														m.normals.push_back(vec3(src[0], src[1], src[2]));
													}
														break;
													}
													break;
												case FbxGeometryElement::eByPolygonVertex:
													switch (normal_reference_mode)
													{
													case FbxGeometryElement::eDirect:
													{
														auto src = normal_direct_array.GetAt(vertex_count);
														m.normals.push_back(vec3(src[0], src[1], src[2]));
													}
														break;
													case FbxGeometryElement::eIndexToDirect:
													{
														auto idx = normal_index_array.GetAt(vertex_count);
														auto src = normal_direct_array.GetAt(idx);
														m.normals.push_back(vec3(src[0], src[1], src[2]));
													}
														break;
													}
													break;
												}
											}

											if (element_tangents)
											{
												switch (tangent_mapping_mode)
												{
												case FbxGeometryElement::eByControlPoint:
													switch (tangent_reference_mode)
													{
													case FbxGeometryElement::eDirect:
													{
														auto src = tangent_direct_array.GetAt(control_point_idx);
														m.tangents.push_back(vec3(src[0], src[1], src[2]));
													}
														break;
													case FbxGeometryElement::eIndexToDirect:
													{
														auto idx = tangent_index_array.GetAt(control_point_idx);
														auto src = tangent_direct_array.GetAt(idx);
														m.tangents.push_back(vec3(src[0], src[1], src[2]));
													}
														break;
													}
													break;
												case FbxGeometryElement::eByPolygonVertex:
													switch (tangent_reference_mode)
													{
													case FbxGeometryElement::eDirect:
													{
														auto src = tangent_direct_array.GetAt(vertex_count);
														m.tangents.push_back(vec3(src[0], src[1], src[2]));
													}
														break;
													case FbxGeometryElement::eIndexToDirect:
													{
														auto idx = tangent_index_array.GetAt(vertex_count);
														auto src = tangent_direct_array.GetAt(idx);
														m.tangents.push_back(vec3(src[0], src[1], src[2]));
													}
														break;
													}
													break;
												}
											}

											if (element_colors)
											{
												switch (color_mapping_mode)
												{
												case FbxGeometryElement::eByControlPoint:
													switch (color_reference_mode)
													{
													case FbxGeometryElement::eDirect:
													{
														auto src = color_direct_array.GetAt(control_point_idx);
														m.colors.push_back(cvec4(src[0] * 255.f, src[1] * 255.f, src[2] * 255.f, src[3] * 255.f));
													}
														break;
													case FbxGeometryElement::eIndexToDirect:
													{
														auto idx = color_index_array.GetAt(control_point_idx);
														auto src = color_direct_array.GetAt(idx);
														m.colors.push_back(cvec4(src[0] * 255.f, src[1] * 255.f, src[2] * 255.f, src[3] * 255.f));
													}
														break;
													}
													break;
												case FbxGeometryElement::eByPolygonVertex:
													switch (color_reference_mode)
													{
													case FbxGeometryElement::eDirect:
													{
														auto src = color_direct_array.GetAt(vertex_count);
														m.colors.push_back(cvec4(src[0] * 255.f, src[1] * 255.f, src[2] * 255.f, src[3] * 255.f));
													}
														break;
													case FbxGeometryElement::eIndexToDirect:
													{
														auto idx = color_index_array.GetAt(vertex_count);
														auto src = color_direct_array.GetAt(idx);
														m.colors.push_back(cvec4(src[0] * 255.f, src[1] * 255.f, src[2] * 255.f, src[3] * 255.f));
													}
														break;
													}
													break;
												}
											}

											if (!control_point_bone_ids.empty() && !control_point_bone_weights.empty())
											{
												m.bone_ids.push_back(control_point_bone_ids[control_point_idx]);
												m.bone_weights.push_back(control_point_bone_weights[control_point_idx]);
											}

											vertex_count++;
										}
									}
								}

								for (auto i = 0; i < max(1, material_count); i++)
								{
									if (material_count > 1)
									{
										auto n_sub = n_children.append_child("item");
										n_sub.append_attribute("name").set_value(i);
										n_components = n_sub.append_child("components");
										auto n_node = n_components.append_child("item");
										n_node.append_attribute("type_name").set_value("flame::cNode");
									}

									auto& mesh = meshes[mesh_idx + i];
									auto n_mesh = n_components.append_child("item");
									n_mesh.append_attribute("type_name").set_value(mesh.bone_ids.empty() ? "flame::cMesh" : "flame::cSkinnedMesh");
									n_mesh.append_attribute("mesh_name").set_value(mesh.filename.string().c_str());
									std::string material_name = "default";
									if (auto fbx_mat = src->GetMaterial(i); fbx_mat)
									{
										if (auto m = find_material(format_name(fbx_mat->GetName())); m)
											material_name = m->filename.string();
									}
									n_mesh.append_attribute("material_name").set_value(material_name.c_str());
								}
							}
						}

						if (auto child_count = src->GetChildCount(); child_count > 0)
						{
							if (!n_children)
								n_children = dst.append_child("children");
							for (auto i = 0; i < child_count; i++)
								process_node(n_children.append_child("item"), src->GetChild(i));
						}
					};
					process_node(preprocess(doc_prefab.append_child("prefab")), scene_root);

					scene->Destroy(true);
				}
			}
			else
#endif
			{
#ifdef USE_ASSIMP
				Assimp::Importer importer;
				importer.SetPropertyString(AI_CONFIG_PP_OG_EXCLUDE_LIST, "trigger");
				auto load_flags =
					aiProcess_RemoveRedundantMaterials |
					aiProcess_Triangulate |
					aiProcess_JoinIdenticalVertices |
					aiProcess_SortByPType |
					aiProcess_GenNormals |
					aiProcess_FlipUVs |
					aiProcess_LimitBoneWeights;
				auto scene = importer.ReadFile(_filename.string(), load_flags);
				if (!scene)
				{
					wprintf(L"cannot import model %s: ", _filename.c_str());
					printf("%s\n", importer.GetErrorString());
					return;
				}
				
				auto scene_root = scene->mRootNode;

				for (auto i = 0; i < scene->mNumAnimations; i++)
				{
					auto ai_ani = scene->mAnimations[i];
					if (ai_ani->mNumChannels == 0)
						continue;

					std::string ani_name = format_name(ai_ani->mName.C_Str());
					auto& ani = new_animation(ani_name, (float)ai_ani->mDuration);

					for (auto j = 0; j < ai_ani->mNumChannels; j++)
					{
						auto ai_ch = ai_ani->mChannels[j];

						auto& ch = ani.channels.emplace_back();
						ch.node_name = ai_ch->mNodeName.C_Str();

						ch.position_keys.resize(ai_ch->mNumPositionKeys);
						for (auto k = 0; k < ch.position_keys.size(); k++)
						{
							ch.position_keys[k].t = ai_ch->mPositionKeys[k].mTime;
							auto& p = ai_ch->mPositionKeys[k].mValue;
							ch.position_keys[k].p = vec3(p.x, p.y, p.z);
						}
						ch.rotation_keys.resize(ai_ch->mNumRotationKeys);
						for (auto k = 0; k < ch.rotation_keys.size(); k++)
						{
							ch.rotation_keys[k].t = ai_ch->mRotationKeys[k].mTime;
							auto& q = ai_ch->mRotationKeys[k].mValue;
							ch.rotation_keys[k].q = quat(q.w, q.x, q.y, q.z);
						}
						ch.scaling_keys.resize(ai_ch->mNumScalingKeys);
						for (auto k = 0; k < ch.scaling_keys.size(); k++)
						{
							ch.scaling_keys[k].t = ai_ch->mScalingKeys[k].mTime;
							auto& s = ai_ch->mScalingKeys[k].mValue;
							ch.scaling_keys[k].s = vec3(s.x, s.y, s.z);
						}
					}
				}

				if (!only_animations)
				{
					for (auto i = 0; i < scene->mNumMaterials; i++)
					{
						aiString ai_name;

						auto ai_mat = scene->mMaterials[i];
						auto mat_name = format_name(ai_mat->GetName().C_Str());
						auto& mat = new_material(mat_name);
						auto map_id = 0;

						aiColor3D ai_color;
						ai_mat->Get(AI_MATKEY_COLOR_DIFFUSE, ai_color);
						mat.color = vec4(ai_color.r, ai_color.g, ai_color.b, 1.f);

						{
							ai_name.Clear();
							ai_mat->GetTexture(aiTextureType_DIFFUSE, 0, &ai_name);
							auto name = std::string(ai_name.C_Str());
							if (!name.empty())
							{
								auto& texture = mat.textures.emplace_back();
								texture.filename = Path::reverse(find_file(parent_path, name));
								mat.color_map = map_id++;
							}
						}

						{
							ai_name.Clear();
							ai_mat->GetTexture(aiTextureType_OPACITY, 0, &ai_name);
							auto name = std::string(ai_name.C_Str());
							if (!name.empty())
							{
								auto& texture = mat.textures.emplace_back();
								texture.filename = Path::reverse(find_file(parent_path, name));
								mat.alpha_map = map_id++;
							}
						}
					}

					pugi::xml_document doc_prefab;

					std::function<void(pugi::xml_node, aiNode*)> process_node;
					process_node = [&](pugi::xml_node dst, aiNode* src) {
						auto name = std::string(src->mName.C_Str());
						dst.append_attribute("name").set_value(name.c_str());

						auto n_components = dst.append_child("components");
						auto n_node = n_components.append_child("item");
						n_node.append_attribute("type_name").set_value("flame::cNode");

						pugi::xml_node n_children;

						{
							aiVector3D s;
							aiVector3D r;
							ai_real a;
							aiVector3D p;
							src->mTransformation.Decompose(s, r, a, p);
							a *= 0.5f;
							auto q = normalize(vec4(sin(a) * vec3(r.x, r.y, r.z), cos(a)));

							if (src == scene_root)
							{
								//if (rotation != vec3(0.f))
								//	n_node.append_attribute("eul").set_value(str(rotation).c_str());
								if (scaling != 1.f)
									s *= scaling;
							}

							auto pos = vec3(p.x, p.y, p.z);
							if (pos != vec3(0.f))
								n_node.append_attribute("pos").set_value(str(pos).c_str());
							auto qut = vec4(q.w, q.x, q.y, q.z);
							if (qut != vec4(1.f, 0.f, 0.f, 0.f))
								n_node.append_attribute("qut").set_value(str(qut).c_str());
							auto scl = vec3(s.x, s.y, s.z);
							if (scl != vec3(1.f))
								n_node.append_attribute("scl").set_value(str(scl).c_str());
						}

						if (src->mNumMeshes > 0)
						{
							for (auto i = 0; i < src->mNumMeshes; i++)
							{
								auto ai_mesh = scene->mMeshes[src->mMeshes[i]];
								auto& mesh = new_mesh(ai_mesh->mName.length == 0 ? str((int)meshes.size()) : ai_mesh->mName.C_Str());

								auto vertex_count = ai_mesh->mNumVertices;

								if (ai_mesh->mVertices)
								{
									mesh.positions.resize(vertex_count);
									memcpy(mesh.positions.data(), ai_mesh->mVertices, sizeof(vec3) * vertex_count);
								}

								if (auto puv = ai_mesh->mTextureCoords[0]; puv)
								{
									mesh.uvs.resize(vertex_count);
									for (auto j = 0; j < vertex_count; j++)
									{
										auto& uv = puv[j];
										mesh.uvs[j] = vec2(uv.x, uv.y);
									}
								}

								if (ai_mesh->mNormals)
								{
									mesh.normals.resize(vertex_count);
									memcpy(mesh.normals.data(), ai_mesh->mNormals, sizeof(vec3) * vertex_count);
								}

								if (ai_mesh->mTangents)
								{
									mesh.tangents.resize(vertex_count);
									memcpy(mesh.tangents.data(), ai_mesh->mTangents, sizeof(vec3) * vertex_count);
								}

								if (ai_mesh->mColors[0])
								{
									mesh.colors.resize(vertex_count);
									for (auto i = 0; i < vertex_count; i++)
									{
										auto& col = ai_mesh->mColors[0][i];
										mesh.colors[i] = cvec4(col.r * 255.f, col.g * 255.f, col.b * 255.f, col.a * 255.f);
									}
								}

								if (ai_mesh->mNumBones > 0)
								{
									auto& arm = new_armature(ai_mesh->mName.C_Str());

									mesh.bone_ids.resize(vertex_count);
									mesh.bone_weights.resize(vertex_count);
									for (auto j = 0; j < vertex_count; j++)
									{
										mesh.bone_ids[j] = ivec4(-1);
										mesh.bone_weights[j] = vec4(0.f);
									}

									for (auto j = 0; j < ai_mesh->mNumBones; j++)
									{
										auto ai_bone = ai_mesh->mBones[j];

										std::string name = ai_bone->mName.C_Str();
										auto bone_idx = arm.find_bone(name);
										if (bone_idx == -1)
										{
											bone_idx = arm.bones.size();
											auto& m = ai_bone->mOffsetMatrix;
											auto& b = arm.bones.emplace_back();
											b.name = name;
											b.offset_matrix = mat4(
												vec4(m.a1, m.b1, m.c1, m.d1),
												vec4(m.a2, m.b2, m.c2, m.d2),
												vec4(m.a3, m.b3, m.c3, m.d3),
												vec4(m.a4, m.b4, m.c4, m.d4)
											);
										}

										auto weights_count = ai_bone->mNumWeights;
										if (weights_count > 0)
										{
											for (auto j = 0; j < weights_count; j++)
											{
												auto w = ai_bone->mWeights[j];
												add_bone_weight(mesh.bone_ids[w.mVertexId], mesh.bone_weights[w.mVertexId], bone_idx, w.mWeight);
											}
										}
									}

									if (!n_children)
									{
										if (ai_mesh->mNumBones > 0)
										{
											if (auto pnode = dst.parent().parent(); pnode)
												add_animator(pnode.child("components"), &arm);
										}
									}
								}

								mesh.indices.resize(ai_mesh->mNumFaces * 3);
								for (auto j = 0; j < ai_mesh->mNumFaces; j++)
								{
									mesh.indices[j * 3 + 0] = ai_mesh->mFaces[j].mIndices[0];
									mesh.indices[j * 3 + 1] = ai_mesh->mFaces[j].mIndices[1];
									mesh.indices[j * 3 + 2] = ai_mesh->mFaces[j].mIndices[2];
								}

								if (src->mNumMeshes > 1)
								{
									if (!n_children)
										n_children = dst.append_child("children");

									auto n_sub = n_children.append_child("item");
									n_sub.append_attribute("name").set_value(i);
									n_components = n_sub.append_child("components");
									auto n_node = n_components.append_child("item");
									n_node.append_attribute("type_name").set_value("flame::cNode");
								}

								auto n_mesh = n_components.append_child("item");
								n_mesh.append_attribute("type_name").set_value(mesh.bone_ids.empty() ? "flame::cMesh" : "flame::cSkinnedMesh");
								n_mesh.append_attribute("mesh_name").set_value(mesh.filename.c_str());
								n_mesh.append_attribute("material_name").set_value(materials[ai_mesh->mMaterialIndex].filename.string().c_str());
							}
						}

						if (src->mNumChildren > 0)
						{
							n_children = dst.append_child("children");
							for (auto i = 0; i < src->mNumChildren; i++)
								process_node(n_children.append_child("item"), src->mChildren[i]);
						}
					};
					process_node(preprocess(doc_prefab.append_child("prefab")), scene->mRootNode);
				}
#endif
			}

			if (!materials.empty())
			{
				if (!std::filesystem::exists(materials_destination))
					std::filesystem::create_directories(materials_destination);

				for (auto& m : materials)
				{
					auto material = new MaterialT;
					material->color = m.color;
					material->color_map = m.color_map;
					material->alpha_map = m.alpha_map;
					material->textures = std::move(m.textures);

					material->save(Path::get(m.filename));

					delete material;
				}
			}

			if (!meshes.empty())
			{
				if (!std::filesystem::exists(meshes_destination))
					std::filesystem::create_directories(meshes_destination);

				for (auto& m : meshes)
				{
					auto mesh = new MeshT;
					mesh->positions = std::move(m.positions);
					mesh->uvs = std::move(m.uvs);
					mesh->normals = std::move(m.normals);
					mesh->tangents = std::move(m.tangents);
					mesh->colors = std::move(m.colors);
					mesh->bone_ids = std::move(m.bone_ids);
					mesh->bone_weights = std::move(m.bone_weights);
					mesh->indices = std::move(m.indices);
					mesh->calc_bounds();

					mesh->save(Path::get(m.filename));

					delete mesh;
				}
			}

			if (!armatures.empty())
			{
				if (!std::filesystem::exists(armatures_destination))
					std::filesystem::create_directories(armatures_destination);

				for (auto& a : armatures)
				{
					auto armature = new ArmatureT;
					armature->bones = std::move(a.bones);

					armature->save(Path::get(a.filename));

					delete armature;
				}
			}

			if (!animations.empty())
			{
				if (!std::filesystem::exists(animations_destination))
					std::filesystem::create_directories(animations_destination);

				for (auto& a : animations)
				{
					for (auto it = a.channels.begin(); it != a.channels.end(); )
					{
						if (!it->position_keys.empty())
						{
							if (it->position_keys.size() > 2)
							{
								auto dist = 0.f;
								for (auto i = 1; i < it->position_keys.size(); i++)
									dist += distance(it->position_keys[i].p, it->position_keys[i - 1].p);
								if (dist < 0.0001f)
									it->position_keys.erase(it->position_keys.begin() + 1, it->position_keys.end() - 1);
							}

							if (it->position_keys.size() == 2)
							{
								if (distance(it->position_keys[0].p, vec3(0.f)) < 0.0001f &&
									distance(it->position_keys[1].p, vec3(0.f)) < 0.0001f)
									it->position_keys.clear();
							}
							else if (it->position_keys.size() == 1)
							{
								if (distance(it->position_keys[0].p, vec3(0.f)) < 0.0001f)
									it->position_keys.clear();
							}
						}

						if (!it->rotation_keys.empty())
						{
							if (it->rotation_keys.size() > 2)
							{
								auto dist = 0.f;
								for (auto i = 1; i < it->rotation_keys.size(); i++)
								{
									auto qd = inverse(it->rotation_keys[i].q) * it->rotation_keys[i - 1].q;
									auto angle = abs(2.f * atan2(length(vec3(qd.x, qd.y, qd.z)), qd.w));
									dist += angle;
								}
								if (dist < 0.1f)
									it->rotation_keys.erase(it->rotation_keys.begin() + 1, it->rotation_keys.end() - 1);
							}

							if (it->rotation_keys.size() == 2)
							{
								auto qd = inverse(it->rotation_keys[1].q) * it->rotation_keys[0].q;
								auto angle = abs(2.f * atan2(length(vec3(qd.x, qd.y, qd.z)), qd.w));
								if (angle < 0.1f)
									it->rotation_keys.clear();
							}
							else if (it->rotation_keys.size() == 1)
							{
								if (it->rotation_keys[0].q == quat(1.f, 0.f, 0.f, 0.f))
									it->rotation_keys.clear();
							}
						}

						if (!it->scaling_keys.empty())
						{
							if (it->scaling_keys.size() > 2)
							{
								auto dist = 0.f;
								for (auto i = 1; i < it->scaling_keys.size(); i++)
									dist += distance(it->scaling_keys[i].s, it->scaling_keys[i - 1].s);
								if (dist < 0.001f)
									it->scaling_keys.erase(it->scaling_keys.begin() + 1, it->scaling_keys.end() - 1);
							}

							if (it->scaling_keys.size() == 2)
							{
								if (distance(it->scaling_keys[0].s, vec3(1.f)) < 0.001f &&
									distance(it->scaling_keys[1].s, vec3(1.f)) < 0.001f)
									it->scaling_keys.clear();
							}
							else if (it->scaling_keys.size() == 1)
							{
								if (distance(it->scaling_keys[0].s, vec3(1.f)) < 0.001f)
									it->scaling_keys.clear();
							}
						}

						if (it->position_keys.empty() && it->rotation_keys.empty() && it->scaling_keys.empty())
							it = a.channels.erase(it);
						else
							++it;
					}

				}

				if (armatures.empty())
				{
					auto has_repeat_channel = false;
					std::map<std::string, Channel*> global_channels;
					for (auto& a : animations)
					{
						for (auto& ch : a.channels)
						{
							auto it = global_channels.find(ch.node_name);
							if (it == global_channels.end())
								it = global_channels.emplace(ch.node_name, &ch).first;
							else
							{
								has_repeat_channel = true;
								break;
							}
						}
						if (has_repeat_channel)
							break;
					}
					if (!has_repeat_channel)
					{
						std::vector<Channel> global_channels_vec;
						for (auto& [name, ch] : global_channels)
							global_channels_vec.push_back(std::move(*ch));
						std::sort(global_channels_vec.begin(), global_channels_vec.end(), [](auto& a, auto& b) { return a.node_name < b.node_name; });
						auto duration = 0.f;
						for (auto& a : animations)
							duration = max(duration, a.duration);

						animations.clear();
						auto& a = new_animation("combined", duration);
						a.channels = std::move(global_channels_vec);
					}

					add_animator(doc_prefab.first_child().child("components"), nullptr);
				}

				for (auto& a : animations)
				{
					auto animation = new AnimationT;
					animation->duration = a.duration;
					animation->channels = std::move(a.channels);

					for (auto& ch : animation->channels)
					{
						if (!ch.position_keys.empty() && ch.position_keys.back().t < animation->duration)
						{
							auto& k = ch.position_keys.emplace_back();
							k.t = animation->duration;
							k.p = ch.position_keys.front().p;
						}
						if (!ch.rotation_keys.empty() && ch.rotation_keys.back().t < animation->duration)
						{
							auto& k = ch.rotation_keys.emplace_back();
							k.t = animation->duration;
							k.q = ch.rotation_keys.front().q;
						}
						if (!ch.scaling_keys.empty() && ch.scaling_keys.back().t < animation->duration)
						{
							auto& k = ch.scaling_keys.emplace_back();
							k.t = animation->duration;
							k.s = ch.scaling_keys.front().s;
						}
					}

					animation->save(Path::get(a.filename));

					delete animation;
				}
			}

			auto fn = destination / model_name;
			fn += L".prefab";
			doc_prefab.save_file(fn.c_str());
		}
	}
}
