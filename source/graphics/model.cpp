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
		void ModelPrivate::save(const std::filesystem::path& filename, bool binary)
		{
			if (binary)
			{
				std::ofstream dst(filename, std::ios::binary);
				dst.write("fmodb", 5);

				SerializeBinarySpec spec;
				spec.excludes.emplace_back(th<graphics::Model>(), "filename"_h);
				spec.excludes.emplace_back(th<graphics::Model>(), "ref"_h);

				serialize_binary(this, dst, spec);
				dst.close();
			}
			else
			{
				std::ofstream dst(filename);
				dst << "model:" << std::endl;

				pugi::xml_document doc;
				auto n_model = doc.append_child("model");

				DataSoup data_soup;

				auto n_meshes = n_model.append_child("meshes");
				for (auto& m : meshes)
				{
					auto n_mesh = n_meshes.append_child("mesh");

					if (!m.positions.empty())
						data_soup.xml_append_v(m.positions, n_mesh.append_child("positions"));
					if (!m.uvs.empty())
						data_soup.xml_append_v(m.uvs, n_mesh.append_child("uvs"));
					if (!m.normals.empty())
						data_soup.xml_append_v(m.normals, n_mesh.append_child("normals"));
					if (!m.tangents.empty())
						data_soup.xml_append_v(m.tangents, n_mesh.append_child("tangents"));
					if (!m.colors.empty())
						data_soup.xml_append_v(m.colors, n_mesh.append_child("colors"));
					if (!m.bone_ids.empty())
						data_soup.xml_append_v(m.bone_ids, n_mesh.append_child("bone_ids"));
					if (!m.bone_weights.empty())
						data_soup.xml_append_v(m.bone_weights, n_mesh.append_child("bone_weights"));
					if (!m.indices.empty())
						data_soup.xml_append_v(m.indices, n_mesh.append_child("indices"));

					n_mesh.append_attribute("bounds").set_value(str((mat2x3&)m.bounds).c_str());
				}

				auto n_bones = n_model.append_child("bones");
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

		struct ModelCreate : Model::Create
		{
			ModelPtr operator()() override
			{
				return new ModelPrivate;
			}
		}Model_create;
		Model::Create& Model::create = Model_create;

		static ModelPtr standard_plane = nullptr;
		static ModelPtr standard_cube = nullptr;
		static ModelPtr standard_sphere = nullptr;
		static ModelPtr standard_cylinder = nullptr;
		static ModelPtr standard_tri_prism = nullptr;

		static std::vector<std::unique_ptr<ModelT>> models;

		struct ModelGet : Model::Get
		{
			ModelPtr operator()(const std::filesystem::path& _filename) override
			{
				auto wstr = _filename.wstring();
				if (SUW::strip_head_if(wstr, L"standard_"))
				{
					if (wstr == L"plane")
					{
						if (!standard_plane)
						{
							auto m = new ModelPrivate;
							m->filename = L"standard_plane";
							auto& mesh = m->meshes.emplace_back();
							mesh.model = m;
							{
								mesh.positions.push_back(vec3(0.f));
								mesh.normals.push_back(vec3(0.f, 1.f, 0.f));
								mesh.positions.push_back(vec3(10.f, 0.f, 0.f));
								mesh.normals.push_back(vec3(0.f, 1.f, 0.f));
								mesh.positions.push_back(vec3(10.f, 0.f, 10.f));
								mesh.normals.push_back(vec3(0.f, 1.f, 0.f));
								mesh.positions.push_back(vec3(0.f, 0.f, 10.f));
								mesh.normals.push_back(vec3(0.f, 1.f, 0.f));

								mesh.indices.push_back(0);
								mesh.indices.push_back(3);
								mesh.indices.push_back(1);
								mesh.indices.push_back(1);
								mesh.indices.push_back(3);
								mesh.indices.push_back(2);
							}
							mesh.calc_bounds();

							standard_plane = m;
						}
						return standard_plane;
					}
					else if (wstr == L"cube")
					{
						if (!standard_cube)
						{
							auto m = new ModelPrivate;
							m->filename = L"standard_cube";
							auto& mesh = m->meshes.emplace_back();
							mesh.model = m;
							mesh_add_cube(mesh, vec3(1.f), vec3(0.f, 0.5f, 0.f), mat3(1.f));
							mesh.calc_bounds();

							standard_cube = m;
						}
						return standard_cube;
					}
					else if (wstr == L"sphere")
					{
						if (!standard_sphere)
						{
							auto m = new ModelPrivate;
							m->filename = L"standard_sphere";
							auto& mesh = m->meshes.emplace_back();
							mesh.model = m;
							mesh_add_sphere(mesh, 0.5f, 12, 12, vec3(0.f, 0.5f, 0.f), mat3(1.f));
							mesh.calc_bounds();

							standard_sphere = m;
						}
						return standard_sphere;
					}
					else if (wstr == L"cylinder")
					{
						if (!standard_cylinder)
						{
							auto m = new ModelPrivate;
							m->filename = L"standard_cylinder";
							auto& mesh = m->meshes.emplace_back();
							mesh.model = m;
							mesh_add_cylinder(mesh, 0.5f, 1.f, 12, vec3(0.f, 0.5f, 0.f));
							mesh.calc_bounds();

							standard_cylinder = m;
						}
						return standard_cylinder;
					}
					else if (wstr == L"tri_prism")
					{
						if (!standard_tri_prism)
						{
							auto m = new ModelPrivate;
							m->filename = L"standard_tri_prism";
							auto& mesh = m->meshes.emplace_back();
							mesh.model = m;
							{
								auto lt_0 = vec3(0.f, 0.f, -0.5f);
								auto lt_1 = vec3(0.f, 1.f, -0.5f);
								auto lt_2 = vec3(1.f, 0.f, -0.5f);
								auto rt_0 = vec3(0.f, 0.f, +0.5f);
								auto rt_1 = vec3(0.f, 1.f, +0.5f);
								auto rt_2 = vec3(1.f, 0.f, +0.5f);

								mesh.positions.push_back(lt_0);
								mesh.normals.push_back(vec3(0.f, 0.f, -1.f));
								mesh.positions.push_back(lt_1);
								mesh.normals.push_back(vec3(0.f, 0.f, -1.f));
								mesh.positions.push_back(lt_2);
								mesh.normals.push_back(vec3(0.f, 0.f, -1.f));
								
								mesh.indices.push_back(0);
								mesh.indices.push_back(1);
								mesh.indices.push_back(2);

								mesh.positions.push_back(rt_0);
								mesh.normals.push_back(vec3(0.f, 0.f, +1.f));
								mesh.positions.push_back(rt_1);
								mesh.normals.push_back(vec3(0.f, 0.f, +1.f));
								mesh.positions.push_back(rt_2);
								mesh.normals.push_back(vec3(0.f, 0.f, +1.f));

								mesh.indices.push_back(3);
								mesh.indices.push_back(5);
								mesh.indices.push_back(4);

								mesh.positions.push_back(lt_0);
								mesh.normals.push_back(vec3(-1.f, 0.f, 0.f));
								mesh.positions.push_back(lt_1);
								mesh.normals.push_back(vec3(-1.f, 0.f, 0.f));
								mesh.positions.push_back(rt_0);
								mesh.normals.push_back(vec3(-1.f, 0.f, 0.f));
								mesh.positions.push_back(rt_1);
								mesh.normals.push_back(vec3(-1.f, 0.f, 0.f));

								mesh.indices.push_back(6);
								mesh.indices.push_back(9);
								mesh.indices.push_back(7);
								mesh.indices.push_back(6);
								mesh.indices.push_back(8);
								mesh.indices.push_back(9);

								auto n = normalize(cross(normalize(lt_2 - lt_1), vec3(0.f, 0.f, -1.f)));
								mesh.positions.push_back(lt_1);
								mesh.normals.push_back(n);
								mesh.positions.push_back(lt_2);
								mesh.normals.push_back(n);
								mesh.positions.push_back(rt_1);
								mesh.normals.push_back(n);
								mesh.positions.push_back(rt_2);
								mesh.normals.push_back(n);

								mesh.indices.push_back(10);
								mesh.indices.push_back(13);
								mesh.indices.push_back(11);
								mesh.indices.push_back(10);
								mesh.indices.push_back(12);
								mesh.indices.push_back(13);

								mesh.positions.push_back(lt_2);
								mesh.normals.push_back(vec3(0.f, -1.f, 0.f));
								mesh.positions.push_back(lt_0);
								mesh.normals.push_back(vec3(0.f, -1.f, 0.f));
								mesh.positions.push_back(rt_2);
								mesh.normals.push_back(vec3(0.f, -1.f, 0.f));
								mesh.positions.push_back(rt_0);
								mesh.normals.push_back(vec3(0.f, -1.f, 0.f));

								mesh.indices.push_back(14);
								mesh.indices.push_back(17);
								mesh.indices.push_back(15);
								mesh.indices.push_back(14);
								mesh.indices.push_back(16);
								mesh.indices.push_back(17);


							}
							mesh.calc_bounds();

							standard_tri_prism = m;
						}
						return standard_tri_prism;
					}
					return nullptr;
				}

				auto filename = Path::get(_filename);

				for (auto& m : models)
				{
					if (m->filename == filename)
					{
						m->ref++;
						return m.get();
					}
				}

				std::ifstream file(filename, std::ios::binary);
				if (!file.good())
				{
					wprintf(L"cannot find model: %s\n", _filename.c_str());
					return nullptr;
				}

				auto ret = new ModelPrivate();
				ret->filename = filename;

				if (char buf[5]; file.read(buf, 5).good() && strncmp(buf, "fmodb", 5) == 0)
				{
					UnserializeBinarySpec spec;
					spec.excludes.emplace_back(th<graphics::Model>(), "filename"_h);
					spec.excludes.emplace_back(th<graphics::Model>(), "ref"_h);

					unserialize_binary(file, ret, spec);
				}
				else
				{
					file.close();
					file.open(filename); // reopen with text mode

					LineReader src(file);
					src.read_block("model:");

					pugi::xml_document doc;
					pugi::xml_node doc_root;
					if (!doc.load_string(src.to_string().c_str()) || (doc_root = doc.first_child()).name() != std::string("model"))
					{
						printf("model format is incorrect: %s\n", filename.string().c_str());
						delete ret;
						return nullptr;
					}

					DataSoup data_soup;
					src.read_block("data:");
					data_soup.load(src);

					for (auto n_mesh : doc_root.child("meshes"))
					{
						auto& m = ret->meshes.emplace_back();
						m.model = ret;

						data_soup.xml_read_v(m.positions, n_mesh.child("positions"));
						if (auto n_uvs = n_mesh.child("uvs"); n_uvs)
							data_soup.xml_read_v(m.uvs, n_uvs);
						if (auto n_normals = n_mesh.child("normals"); n_normals)
							data_soup.xml_read_v(m.normals, n_normals);
						if (auto n_tangents = n_mesh.child("tangents"); n_tangents)
							data_soup.xml_read_v(m.tangents, n_tangents);
						if (auto n_colors = n_mesh.child("colors"); n_colors)
							data_soup.xml_read_v(m.colors, n_colors);
						if (auto n_bids = n_mesh.child("bone_ids"); n_bids)
							data_soup.xml_read_v(m.bone_ids, n_bids);
						if (auto n_wgts = n_mesh.child("bone_weights"); n_wgts)
							data_soup.xml_read_v(m.bone_weights, n_wgts);
						data_soup.xml_read_v(m.indices, n_mesh.child("indices"));

						m.bounds = (AABB&)s2t<2, 3, float>(n_mesh.attribute("bounds").value());
					}

					for (auto n_bone : doc_root.child("bones"))
					{
						auto& b = ret->bones.emplace_back();
						b.name = n_bone.attribute("name").value();
						data_soup.xml_read(&b.offset_matrix, n_bone.child("offset_matrix"));
					}

					for (auto& m : ret->meshes)
						ret->bounds.expand(m.bounds);
				}

				ret->ref = 1;
				models.emplace_back(ret);
				return ret;
			}
		}Model_get;
		Model::Get& Model::get = Model_get;

		struct ModelRelease : Model::Release
		{
			void operator()(ModelPtr model) override
			{
				if (model->filename.wstring().starts_with(L"standard:"))
					return;
				if (model->ref == 1)
				{
					std::erase_if(models, [&](const auto& i) {
						return i.get() == model;
					});
				}
				else
					model->ref--;
			}
		}Model_release;
		Model::Release& Model::release = Model_release;

		void import_scene(const std::filesystem::path& _filename, const std::filesystem::path& _destination, const vec3& rotation, const vec3& scaling, bool only_animation)
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
			auto animations_destination = destination / L"animations";
			auto ext = SUW::get_lowered(filename.extension().wstring());
			filename.replace_extension(L".fmod");
			auto model_name = filename.filename().string();

			std::vector<std::unique_ptr<MaterialT>> materials;
			std::vector<std::unique_ptr<AnimationT>> animations;
			std::unique_ptr<ModelT> model(new ModelT);

			auto format_res_name = [&](const std::string& name, const std::string& ext, int i) {
				auto ret = name;
				if (ret.empty())
					ret = str(i);
				else
				{
					for (auto& ch : ret)
					{
						if (ch == ' ' || ch == ':' || ch == '|')
							ch = '_';
					}
				}
				ret = std::format("{}.{}", ret, ext);
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

			auto need_wrap_root = rotation != vec3(0.f) || scaling != vec3(1.f);
			auto preprocess = [&](pugi::xml_node first_node)->pugi::xml_node {
				if (need_wrap_root)
				{
					first_node.append_attribute("file_id").set_value(generate_guid().to_string().c_str());
					auto n_node = first_node.append_child("components").append_child("item");
					n_node.append_attribute("type_name").set_value("flame::cNode");
					return first_node.append_child("children").append_child("item");
				}
				return first_node;
			};
			auto postprocess = [&](pugi::xml_node first_node) {
				auto dst = first_node;
				if (need_wrap_root)
					dst = dst.child("children").first_child();
				auto n_components = dst.child("components");
				auto n_node = n_components.first_child();
				if (rotation != vec3(0.f))
					n_node.append_attribute("eul").set_value(str(rotation).c_str());
				if (scaling != vec3(1.f))
					n_node.append_attribute("scl").set_value(str(scaling).c_str());
				if (!model->bones.empty())
				{
					auto n_armature = n_components.append_child("item");
					n_armature.append_attribute("type_name").set_value("flame::cArmature");
					n_armature.append_attribute("armature_name").set_value(model_name.c_str());
					for (auto& m : model->meshes)
					{
						for (auto i = 0; i < m.bone_ids.size(); i++)
						{
							auto n = 0;
							auto ids = m.bone_ids[i];
							for (auto j = 0; j < 4; j++)
							{
								if (ids[j] == -1)
									break;
								n++;
							}
							auto& w = m.bone_weights[i];
							switch (n)
							{
							case 1: w = vec4(1.f, 0.f, 0.f, 0.f);				break;
							case 2: w = vec4(w.xy() / (w.x + w.y), 0.f, 0.f);	break;
							case 3: w = vec4(w.xyz() / (w.x + w.y + w.z), 0.f);	break;
							case 4: w = w /= (w.x + w.y + w.z + w.w);			break;
							}
						}
					}

					auto& name = model->bones.front().name;
					auto dst_children = dst.child("children");
					for (auto n : dst_children.children())
					{
						if (auto n_children = n.child("children"); n_children)
						{
							if (auto nn = n_children.first_child(); nn && nn.attribute("name").value() == name)
							{
								dst_children.prepend_move(n);
								break;
							}
						}
					}
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
					printf("cannot import model %s: %s\n", _filename.string().c_str(), importer->GetStatus().GetErrorString());
					return;
				}

				if (!importer->Import(scene))
				{
					printf("cannot import model %s: %s\n", _filename.string().c_str(), importer->GetStatus().GetErrorString());
					return;
				}

				importer->Destroy();

				FbxAxisSystem our_axis_system(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);
				if (scene->GetGlobalSettings().GetAxisSystem() != our_axis_system)
					our_axis_system.ConvertScene(scene);

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

					auto animation = new AnimationT;
					animation->duration = stop_time - start_time;

					std::vector<std::pair<fbxsdk::FbxNode*, Channel*>> channels;
					std::function<void(fbxsdk::FbxNode*, float)> get_bone_animation;
					get_bone_animation = [&](fbxsdk::FbxNode* node, float time) {
						FbxTime fbx_time;
						fbx_time.SetMilliSeconds(time * 1000.f);
						auto transform = to_glm(node->EvaluateLocalTransform(fbx_time) * get_geometry(node));

						if (auto node_attr = node->GetNodeAttribute(); node_attr && node_attr->GetAttributeType() == FbxNodeAttribute::eSkeleton)
						{
							auto channel = (Channel*)node->GetUserDataPtr();
							if (!channel)
							{
								channel = new Channel();
								channel->node_name = node->GetName();
								node->SetUserDataPtr(channel);
								channels.emplace_back(node, channel);
							}

							vec3 pos; quat qut; vec3 scl; vec3 skew; vec4 perspective;
							decompose(transform, scl, qut, pos, skew, perspective);
							channel->position_keys.push_back({ time, pos });
							channel->rotation_keys.push_back({ time, qut });
						}

						auto children_count = node->GetChildCount();
						for (auto i = 0; i < children_count; i++)
							get_bone_animation(node->GetChild(i), time);
					};

					for (auto t = start_time; t < stop_time; t += 1.f / 24.f)
						get_bone_animation(scene->GetRootNode(), t);

					for (auto& ch : channels)
						animation->channels.push_back(*ch.second);
					for (auto& ch : animation->channels)
					{
						for (auto& k : ch.position_keys)
							k.t -= start_time;
						for (auto& k : ch.rotation_keys)
							k.t -= start_time;
					}

					for (auto& ch : channels)
					{
						ch.first->SetUserDataPtr(nullptr);
						delete ch.second;
					}

					if (!std::filesystem::exists(animations_destination))
						std::filesystem::create_directories(animations_destination);
					auto fn = animations_destination / format_res_name(anim_stack->GetName(), "fani", i);
					animation->save(fn);
					animation->filename = Path::reverse(fn);
					animations.emplace_back(animation);
				}
				scene->SetCurrentAnimationStack(nullptr);

				if (only_animation)
				{
					scene->Destroy();
					return;
				}

				FbxGeometryConverter geom_converter(sdk_manager);
				try
				{
					geom_converter.Triangulate(scene, true);
				}
				catch (std::runtime_error)
				{
					printf("FBX SDK: cannot triangulate %s\n", _filename.string().c_str());
					return;
				}

				pugi::xml_document doc_prefab;

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
						if (!fbx_mat->GetUserDataPtr())
						{
							auto material = new MaterialT;
							auto map_id = 0;
							if (auto prop = fbx_mat->FindProperty(fbxsdk::FbxSurfaceMaterial::sDiffuse); prop.IsValid())
							{
								auto res = prop.Get<fbxsdk::FbxDouble3>();
								material->color[0] = res[0];
								material->color[1] = res[1];
								material->color[2] = res[2];
								if (prop.GetSrcObjectCount<FbxFileTexture>())
								{
									if (auto tex = prop.GetSrcObject<FbxFileTexture>(); tex)
									{
										std::string tex_name = tex->GetFileName();
										if (!tex_name.empty())
										{
											auto fn = find_file(parent_path, tex_name);
											if (std::filesystem::exists(fn))
											{
												if (!std::filesystem::exists(textures_destination))
													std::filesystem::create_directories(textures_destination);
												auto dst = fn.filename().stem();
												dst += L"%s%m";
												dst += fn.extension();
												dst = textures_destination / dst;
												if (dst != fn)
												{
													std::filesystem::copy_file(fn, dst, std::filesystem::copy_options::overwrite_existing);
													fn = dst;
												}
											}
											if (material->textures.size() <= map_id)
												material->textures.resize(map_id + 1);
											auto& texture = material->textures[map_id];
											texture.filename = Path::rebase(materials_destination, fn);
											material->color_map = map_id++;
										}
									}
								}
							}

							if (!std::filesystem::exists(materials_destination))
								std::filesystem::create_directories(materials_destination);
							auto fn = materials_destination / format_res_name(fbx_mat->GetName(), "fmat", i);
							material->save(fn);
							material->filename = Path::reverse(fn);
							materials.emplace_back(material);
							fbx_mat->SetUserDataPtr(material);
						}
					}

					if (auto node_attr = src->GetNodeAttribute(); node_attr)
					{
						auto node_type = node_attr->GetAttributeType();
						if (node_type == fbxsdk::FbxNodeAttribute::eMesh)
						{
							auto fbx_mesh = src->GetMesh();
							if (!fbx_mesh->GetUserDataPtr())
							{
								auto base_mesh_idx = model->meshes.size();

								auto polygon_count = fbx_mesh->GetPolygonCount();

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
								auto skin_count = fbx_mesh->GetDeformerCount(FbxDeformer::eSkin);
								if (skin_count)
								{
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
											if (!link) continue;

											std::string name = link->GetName();
											auto bone_idx = model->find_bone(name);
											if (bone_idx == -1)
											{
												bone_idx = model->bones.size();
												model->bones.emplace_back().name = name;
											}

											auto& bone = model->bones[bone_idx];

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
								auto vertex_count = 0;
								for (auto i = 0; i < polygon_count; i++)
								{
									auto sub_mesh_idx = 0;
									if (element_mat && mat_mapping_mode == FbxGeometryElement::eByPolygon)
										sub_mesh_idx = mat_index_array.GetAt(i);

									if (base_mesh_idx + sub_mesh_idx + 1 > model->meshes.size())
										model->meshes.resize(base_mesh_idx + sub_mesh_idx + 1);

									auto& m = model->meshes[base_mesh_idx + sub_mesh_idx];
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

											if (skin_count)
											{
												m.bone_ids.push_back(control_point_bone_ids[control_point_idx]);
												m.bone_weights.push_back(control_point_bone_weights[control_point_idx]);
											}

											vertex_count++;
										}
									}
								}

								for (auto it = model->meshes.begin(); it != model->meshes.end();)
								{
									if (it->indices.empty())
										it = model->meshes.erase(it);
									else
									{
										it->calc_bounds();
										it++;
									}
								}
								fbx_mesh->SetUserDataPtr((void*)((base_mesh_idx << 16) + (int)model->meshes.size() - base_mesh_idx));
							}

							auto user_data = (int)fbx_mesh->GetUserDataPtr();
							auto base_mesh_idx = user_data >> 16;
							auto mesh_count = user_data & 0x0000ffff;
							if (mesh_count > 0)
							{
								if (mesh_count == 1)
								{
									auto n_mesh = n_components.append_child("item");
									n_mesh.append_attribute("type_name").set_value("flame::cMesh");
									n_mesh.append_attribute("mesh_name").set_value(("models\\" + model_name + "#mesh" + str(base_mesh_idx)).c_str());
									std::string material_name = "default";
									if (auto fbx_mat = src->GetMaterial(0); fbx_mat)
									{
										if (auto mat = (MaterialPtr)fbx_mat->GetUserDataPtr(); mat)
											material_name = "materials\\" + mat->filename.filename().string();
									}
									n_mesh.append_attribute("material_name").set_value(material_name.c_str());
								}
								else
								{
									if (!n_children)
										n_children = dst.append_child("children");
									for (auto i = 0; i < mesh_count; i++)
									{
										auto n_sub = n_children.append_child("item");
										n_sub.append_attribute("name").set_value(i);
										auto n_components = n_sub.append_child("components");
										auto n_node = n_components.append_child("item");
										n_node.append_attribute("type_name").set_value("flame::cNode");
										auto n_mesh = n_components.append_child("item");
										n_mesh.append_attribute("type_name").set_value("flame::cMesh");
										n_mesh.append_attribute("mesh_name").set_value(("models\\" + model_name + "#mesh" + str(base_mesh_idx + i)).c_str());
										std::string material_name = "default";
										if (auto fbx_mat = src->GetMaterial(i); fbx_mat)
										{
											if (auto mat = (MaterialPtr)fbx_mat->GetUserDataPtr(); mat)
												material_name = "materials\\" + mat->filename.filename().string();
										}
										n_mesh.append_attribute("material_name").set_value(material_name.c_str());
									}
								}
							}
						}
						else if (node_type == FbxNodeAttribute::eSkeleton)
						{
							std::string name = src->GetName();
							auto bone_idx = model->find_bone(name);
							if (bone_idx == -1)
							{
								bone_idx = model->bones.size();
								auto& b = model->bones.emplace_back();
								b.name = name;
								b.offset_matrix = mat4(1.f);
							}
						}
					}

					auto child_count = src->GetChildCount();
					if (child_count > 0)
					{
						if (!n_children)
							n_children = dst.append_child("children");
						for (auto i = 0; i < child_count; i++)
							process_node(n_children.append_child("item"), src->GetChild(i));
					}
				};
				process_node(preprocess(doc_prefab.append_child("prefab")), scene->GetRootNode());
				postprocess(doc_prefab.first_child());

				if (!std::filesystem::exists(models_destination))
					std::filesystem::create_directories(models_destination);
				auto fn = models_destination / model_name;
				model->save(fn, false);
				fn = destination / model_name;
				fn.replace_extension(L".prefab");
				doc_prefab.save_file(fn.c_str());

				scene->Destroy(true);
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
					printf("cannot import model %s: %s\n", _filename.string().c_str(), importer.GetErrorString());
					return;
				}

				for (auto i = 0; i < scene->mNumAnimations; i++)
				{
					auto ai_ani = scene->mAnimations[i];

					auto animation = new AnimationT;
					animation->duration = (float)ai_ani->mDuration;

					for (auto j = 0; j < ai_ani->mNumChannels; j++)
					{
						auto ai_ch = ai_ani->mChannels[j];

						auto& ch = animation->channels.emplace_back();
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
					}

					auto fn = parent_path / format_res_name(ai_ani->mName.C_Str(), "fani", i);
					animation->save(fn);
					animation->filename = Path::reverse(fn);
					animations.emplace_back(animation);
				}

				if (only_animation)
					return;

				for (auto i = 0; i < scene->mNumMaterials; i++)
				{
					aiString ai_name;
					auto ai_mat = scene->mMaterials[i];
					auto material = new MaterialT;
					auto map_id = 0;

					{
						ai_name.Clear();
						ai_mat->GetTexture(aiTextureType_DIFFUSE, 0, &ai_name);
						auto name = std::string(ai_name.C_Str());
						if (!name.empty())
						{
							if (material->textures.size() <= map_id)
								material->textures.resize(map_id + 1);
							material->textures[map_id].filename = Path::reverse(find_file(parent_path, name));
							material->color_map = map_id++;
						}
					}

					{
						ai_name.Clear();
						ai_mat->GetTexture(aiTextureType_OPACITY, 0, &ai_name);
						auto name = std::string(ai_name.C_Str());
						if (!name.empty())
						{
							if (material->textures.size() <= map_id)
								material->textures.resize(map_id + 1);
							material->textures[map_id].filename = Path::reverse(find_file(parent_path, name));
							material->alpha_map = map_id++;
						}
					}

					auto fn = destination / format_res_name(ai_mat->GetName().C_Str(), "fmat", i);
					material->save(fn);
					material->filename = Path::reverse(fn);

					materials.emplace_back(material);
				}

				for (auto i = 0; i < scene->mNumMeshes; i++)
				{
					auto ai_mesh = scene->mMeshes[i];
					auto& mesh = model->meshes.emplace_back();
					mesh.model = model.get();

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
							auto bone_idx = model->find_bone(name);
							if (bone_idx == -1)
							{
								bone_idx = model->bones.size();
								auto& m = ai_bone->mOffsetMatrix;
								auto& b = model->bones.emplace_back();
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
					}

					mesh.indices.resize(ai_mesh->mNumFaces * 3);
					for (auto j = 0; j < ai_mesh->mNumFaces; j++)
					{
						mesh.indices[j * 3 + 0] = ai_mesh->mFaces[j].mIndices[0];
						mesh.indices[j * 3 + 1] = ai_mesh->mFaces[j].mIndices[1];
						mesh.indices[j * 3 + 2] = ai_mesh->mFaces[j].mIndices[2];
					}

					mesh.calc_bounds();
				}

				pugi::xml_document doc_prefab;

				std::function<void(pugi::xml_node, aiNode*)> process_node;
				process_node = [&](pugi::xml_node dst, aiNode* src) {
					auto name = std::string(src->mName.C_Str());
					dst.append_attribute("name").set_value(name.c_str());

					auto n_components = dst.append_child("components");

					{
						auto n_node = n_components.append_child("item");
						n_node.append_attribute("type_name").set_value("flame::cNode");

						aiVector3D s;
						aiVector3D r;
						ai_real a;
						aiVector3D p;
						src->mTransformation.Decompose(s, r, a, p);
						a *= 0.5f;
						auto q = normalize(vec4(sin(a) * vec3(r.x, r.y, r.z), cos(a)));

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
						if (name == "trigger")
						{
							auto n_rigid = n_components.append_child("item");
							n_rigid.append_attribute("type_name").set_value("flame::cRigid");
							n_rigid.append_attribute("dynamic").set_value(false);
							auto n_shape = n_components.append_child("item");
							n_shape.append_attribute("type_name").set_value("flame::cShape");
							n_shape.append_attribute("type").set_value("Cube");
							n_shape.append_attribute("size").set_value("2,2,0.01");
							n_shape.append_attribute("trigger").set_value(true);
						}
						else
						{
							auto n_mesh = n_components.append_child("item");
							n_mesh.append_attribute("type_name").set_value("flame::cMesh");
							auto mesh_idx = src->mMeshes[0];
							n_mesh.append_attribute("mesh_name").set_value(("models\\" + model_name + "#mesh" + str(mesh_idx)).c_str());
							n_mesh.append_attribute("material_name").set_value(("materials\\" + (materials[scene->mMeshes[mesh_idx]->mMaterialIndex]->filename).string()).c_str());
							if (name == "mesh_collider")
							{
								auto n_rigid = n_components.append_child("item");
								n_rigid.append_attribute("type_name").set_value("flame::cRigid");
								n_rigid.append_attribute("dynamic").set_value(false);
								auto n_shape = n_components.append_child("item");
								n_shape.append_attribute("type_name").set_value("flame::cShape");
								n_shape.append_attribute("type").set_value("Mesh");
							}
						}
					}

					if (src->mNumChildren > 0)
					{
						auto n_children = dst.append_child("children");
						for (auto i = 0; i < src->mNumChildren; i++)
							process_node(n_children.append_child("item"), src->mChildren[i]);
					}
				};
				process_node(preprocess(doc_prefab.append_child("prefab")), scene->mRootNode);
				postprocess(doc_prefab.first_child());

				if (!std::filesystem::exists(models_destination))
					std::filesystem::create_directories(models_destination);
				auto fn = models_destination / model_name;
				model->save(fn, false);
				fn = destination / model_name;
				fn.replace_extension(L".prefab");
				doc_prefab.save_file(fn.c_str());
#endif
			}
		}
	}
}
