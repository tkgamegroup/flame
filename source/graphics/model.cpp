#include "../serialize_extension.h"
#include "../foundation/typeinfo.h"
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
		void ModelPrivate::save(const std::filesystem::path& filename)
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

		void Model::convert(const std::filesystem::path& _filename, const vec3& rotation, const vec3& scaling, bool only_animation)
		{
			auto parent_path = _filename.parent_path();
			auto filename = Path::reverse(_filename);
			auto model_name = filename.filename().stem().string();
			auto ext = filename.extension();
			filename.replace_extension(L".fmod");

			std::vector<std::unique_ptr<MaterialT>> materials;
			std::vector<std::unique_ptr<AnimationT>> animations;
			std::unique_ptr<ModelT> model(new ModelT);

			auto format_res_name = [&](const std::string& name, const std::string& ext, int i) {
				auto ret = name;
				if (ret.empty())
					ret = str(i);
				else
					for (auto& ch : ret) if (ch == ' ' || ch == ':') ch = '_';
				ret = std::format("{}_{}.{}", model_name, ret, ext);
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
					auto n_node = first_node.append_child("components").append_child("item");
					n_node.append_attribute("type_name").set_value("flame::cNode");
					return first_node.append_child("children").append_child("item");
				}
				return first_node;
			};
			auto postprocess = [&](pugi::xml_node first_node) {
				if (!model->bones.empty())
				{
					auto dst = first_node;
					if (need_wrap_root)
						dst = dst.child("children").first_child();
					auto n_components = dst.child("components");
					auto n_node = n_components.first_child();
					if (rotation != vec3(0.f))
						n_node.append_attribute("eul").set_value(str(rotation).c_str());
					if (scaling != vec3(1.f))
						n_node.append_attribute("scl").set_value(str(scaling).c_str());
					auto n_armature = n_components.append_child("item");
					n_armature.append_attribute("type_name").set_value("flame::cArmature");
					n_armature.append_attribute("armature_name").set_value(filename.string().c_str());
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
				}
			};

#ifdef USE_FBXSDK
			if (ext == L".fbx")
			{
				using namespace fbxsdk;

				auto get_matrix = [](FbxNode* pNode) {
					const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
					const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
					const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);

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
					auto anim_stack = scene->FindMember<FbxAnimStack>(anim_names[i]->Buffer());
					auto layer = anim_stack->GetMember<FbxAnimLayer>(0);
					scene->SetCurrentAnimationStack(anim_stack);

					auto animation = new AnimationT;
					animation->duration = 0.f;

					std::function<void(FbxNode*)> get_node_curves;
					get_node_curves = [&](FbxNode* node) {
						FbxAnimCurve* curve = nullptr;
						std::vector<std::pair<uint, vec3>> position_keys;
						std::vector<std::pair<uint, vec3>> rotation_keys;
						auto set_key = [](std::vector<std::pair<uint, vec3>>& dst, uint t, int idx, float v) {
							if (dst.empty() && v == 0.f)
								return;
							auto it = std::lower_bound(dst.begin(), dst.end(), t, [](const auto& i, auto v) {
								return v < i.first;
							});
							if (it == dst.end() || it->first != t)
								it = dst.emplace(it, std::make_pair(t, vec3(0.f)));
							it->second[idx] = v;
						};
						auto read_curve = [&](int type, int idx) {
							const char* names[] = {
								FBXSDK_CURVENODE_COMPONENT_X,
								FBXSDK_CURVENODE_COMPONENT_Y,
								FBXSDK_CURVENODE_COMPONENT_Z
							};
							auto name = names[idx];
							curve = type == 0 ? node->LclTranslation.GetCurve(layer, name) :
								node->LclRotation.GetCurve(layer, name);
							auto& dst = type == 0 ? position_keys : rotation_keys;
							if (curve)
							{
								auto keys_count = curve->KeyGetCount();
								for (auto i = 0; i < keys_count; i++)
								{
									auto t = (uint)curve->KeyGetTime(i).GetMilliSeconds();
									auto v = (float)curve->KeyGetValue(i);
									set_key(dst, t, idx, v);
								}
							}
						};
						read_curve(0, 0); read_curve(0, 1); read_curve(0, 2);
						read_curve(1, 0); read_curve(1, 1); read_curve(1, 2);

						if (!position_keys.empty() || !rotation_keys.empty())
						{
							std::reverse(position_keys.begin(), position_keys.end());
							std::reverse(rotation_keys.begin(), rotation_keys.end());

							auto& ch = animation->channels.emplace_back();
							ch.node_name = node->GetName();
							ch.position_keys.resize(position_keys.size());
							for (auto i = 0; i < position_keys.size(); i++)
							{
								FbxTime time;
								time.SetMilliSeconds(position_keys[i].first);
								auto mat = to_glm(node->EvaluateGlobalTransform(time));
								if (auto parent = node->GetParent(); parent)
									mat = inverse(to_glm(parent->EvaluateGlobalTransform(time))) * mat;
								vec3 pos; quat qut; vec3 scl; vec3 skew; vec4 perspective;
								decompose(mat, scl, qut, pos, skew, perspective);

								ch.position_keys[i].t = (float)position_keys[i].first / 1000.f;
								ch.position_keys[i].p = pos;
							}
							ch.rotation_keys.resize(rotation_keys.size());
							for (auto i = 0; i < rotation_keys.size(); i++)
							{
								FbxTime time;
								time.SetMilliSeconds(rotation_keys[i].first);
								auto mat = to_glm(node->EvaluateGlobalTransform(time));
								if (auto parent = node->GetParent(); parent)
									mat = inverse(to_glm(parent->EvaluateGlobalTransform(time))) * mat;
								vec3 pos; quat qut; vec3 scl; vec3 skew; vec4 perspective;
								decompose(mat, scl, qut, pos, skew, perspective);

								ch.rotation_keys[i].t = (float)rotation_keys[i].first / 1000.f;
								ch.rotation_keys[i].q = qut;
							}

							if (!ch.position_keys.empty())
								animation->duration = max(animation->duration, ch.position_keys.back().t);
							if (!ch.rotation_keys.empty())
								animation->duration = max(animation->duration, ch.rotation_keys.back().t);
						}

						auto children_count = node->GetChildCount();
						for (auto i = 0; i < children_count; i++)
							get_node_curves(node->GetChild(i));
					};

					get_node_curves(scene->GetRootNode());

					auto fn = parent_path / format_res_name(anim_stack->GetName(), "fani", i);
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

				std::function<void(pugi::xml_node, FbxNode*)> process_node;
				process_node = [&](pugi::xml_node dst, FbxNode* src) {
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
							{
								auto prop = fbx_mat->FindProperty(FbxSurfaceMaterial::sDiffuse);
								if (prop.IsValid())
								{
									auto res = prop.Get<FbxDouble3>();
									material->color[0] = res[0];
									material->color[1] = res[1];
									material->color[2] = res[2];
									if (prop.GetSrcObjectCount<FbxFileTexture>()) 
									{
										if (auto tex = prop.GetSrcObject<FbxFileTexture>(); tex)
										{
											 material->textures[map_id].filename = Path::reverse(find_file(parent_path, tex->GetFileName()));
											 material->color_map = map_id++;
										}
									}
								}
							}

							auto fn = parent_path / format_res_name(fbx_mat->GetName(), "fmat", i);
							material->save(fn);
							material->filename = Path::reverse(fn);
							materials.emplace_back(material);
							fbx_mat->SetUserDataPtr(material);
						}
					}

					if (auto node_attr = src->GetNodeAttribute(); node_attr)
					{
						auto node_type = node_attr->GetAttributeType();
						if (node_type == FbxNodeAttribute::eMesh)
						{
							auto fbx_mesh = src->GetMesh();
							if (!fbx_mesh->GetUserDataPtr())
							{
								auto base_mesh_idx = model->meshes.size();

								auto polygon_count = fbx_mesh->GetPolygonCount();
								auto element_mat = fbx_mesh->GetElementMaterial();
								FbxLayerElementArrayTemplate<int>* mat_indices = NULL;
								auto material_mapping_polygon = false;
								if (element_mat)
								{
									mat_indices = &element_mat->GetIndexArray();
									material_mapping_polygon = element_mat->GetMappingMode() == FbxGeometryElement::eByPolygon;
								}

								auto has_uv = fbx_mesh->GetElementUVCount() > 0;
								auto has_normal = fbx_mesh->GetElementNormalCount() > 0;
								auto uv_mapping_mode = FbxGeometryElement::eNone;
								auto normal_mapping_mode = FbxGeometryElement::eNone;
								if (has_uv)
								{
									uv_mapping_mode = fbx_mesh->GetElementUV(0)->GetMappingMode();
									if (uv_mapping_mode == FbxGeometryElement::eNone)
										has_uv = false;
								}
								if (has_normal)
								{
									normal_mapping_mode = fbx_mesh->GetElementNormal(0)->GetMappingMode();
									if (normal_mapping_mode == FbxGeometryElement::eNone)
										has_normal = false;
								}

								std::string uv_name;
								if (has_uv)
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
											reference_init *= get_matrix(src);
											cluster->GetTransformLinkMatrix(cluster_init);
											bone.offset_matrix = to_glm(cluster_init.Inverse() * reference_init);

											int vertex_idx_count = cluster->GetControlPointIndicesCount();
											for (int k = 0; k < vertex_idx_count; k++)
											{
												auto vi = cluster->GetControlPointIndices()[k];
												if (vi >= control_points_count)
													continue;

												auto weight = (float)cluster->GetControlPointWeights()[k];
												if (weight <= 0.f)
													continue;

												add_bone_weight(control_point_bone_ids[vi], control_point_bone_weights[vi], bone_idx, weight);
											}
										}
									}
								}
								for (auto i = 0; i < polygon_count; i++)
								{
									auto sub_mesh_idx = 0;
									if (mat_indices && material_mapping_polygon)
										sub_mesh_idx = mat_indices->GetAt(i);

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
											if (has_uv)
											{
												FbxVector2 uv;
												bool unmapped;
												fbx_mesh->GetPolygonVertexUV(i, j, uv_name.c_str(), uv, unmapped);
												m.uvs.push_back(vec2(uv[0], 1.f - uv[1]));
											}
											if (has_normal)
											{
												FbxVector4 nor;
												fbx_mesh->GetPolygonVertexNormal(i, j, nor);
												m.normals.push_back(vec3(nor[0], nor[1], nor[2]));
											}
											if (skin_count)
											{
												m.bone_ids.push_back(control_point_bone_ids[control_point_idx]);
												m.bone_weights.push_back(control_point_bone_weights[control_point_idx]);
											}
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
									n_mesh.append_attribute("mesh_name").set_value((filename.string() + "#mesh" + str(base_mesh_idx)).c_str());
									auto fbx_mat = src->GetMaterial(0);
									if (fbx_mat)
									{
										auto mat = (MaterialPtr)fbx_mat->GetUserDataPtr();
										if (mat)
											n_mesh.append_attribute("material_name").set_value(mat->filename.string().c_str());
									}
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
										n_mesh.append_attribute("mesh_name").set_value((filename.string() + "#mesh" + str(base_mesh_idx + i)).c_str());
										n_mesh.append_attribute("material_name").set_value(((MaterialPtr)src->GetMaterial(i)->GetUserDataPtr())->filename.string().c_str());
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
				model->save(replace_ext(_filename, L".fmod"));
				doc_prefab.save_file(replace_ext(_filename, L".prefab").c_str());

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
							material->textures[map_id].filename = Path::reverse(find_file(parent_path, name));
							material->alpha_map = map_id++;
						}
					}

					auto fn = parent_path / format_res_name(ai_mat->GetName().C_Str(), "fmat", i);
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
							n_mesh.append_attribute("mesh_name").set_value((filename.string() + "#mesh" + str(mesh_idx)).c_str());
							n_mesh.append_attribute("material_name").set_value(materials[scene->mMeshes[mesh_idx]->mMaterialIndex]->filename.string().c_str());
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
				model->save(replace_ext(_filename, L".fmod"));
				doc_prefab.save_file(replace_ext(_filename, L".prefab").c_str());
#endif
			}
		}

		struct ModelCreate : Model::Create
		{
			ModelPtr operator()() override
			{
				return new ModelPrivate;
			}
		}Model_create;
		Model::Create& Model::create = Model_create;

		static ModelPtr standard_cube = nullptr;
		static ModelPtr standard_sphere = nullptr;

		static std::vector<std::unique_ptr<ModelT>> models;

		struct ModelGet : Model::Get
		{
			ModelPtr operator()(const std::filesystem::path& _filename) override
			{
				auto wstr = _filename.wstring();
				if (SUW::strip_head_if(wstr, L"standard_"))
				{
					if (wstr == L"cube")
					{
						if (!standard_cube)
						{
							auto m = new ModelPrivate;
							m->filename = L"standard_cube";
							auto& mesh = m->meshes.emplace_back();
							mesh.model = m;
							mesh_add_cube(mesh, vec3(1.f), vec3(0.f), mat3(1.f));
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
							mesh_add_sphere(mesh, 0.5f, 12, 12, vec3(0.f), mat3(1.f));
							mesh.calc_bounds();

							standard_sphere = m;
						}
						return standard_sphere;
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

				std::ifstream file(filename);
				if (!file.good())
				{
					wprintf(L"cannot find model: %s\n", _filename.c_str());
					return nullptr;
				}
				LineReader src(file);
				src.read_block("model:");

				pugi::xml_document doc;
				pugi::xml_node doc_root;
				if (!doc.load_string(src.to_string().c_str()) || (doc_root = doc.first_child()).name() != std::string("model"))
				{
					printf("model format is incorrect: %s\n", filename.string().c_str());
					return nullptr;
				}

				DataSoup data_soup;
				src.read_block("data:");
				data_soup.load(src);

				auto ret = new ModelPrivate();
				ret->filename = filename;

				for (auto n_mesh : doc_root.child("meshes"))
				{
					auto& m = ret->meshes.emplace_back();
					m.model = ret;

					data_soup.xml_read_v(m.positions, n_mesh.child("positions"));
					if (auto n_uvs = n_mesh.child("uvs"); n_uvs)
						data_soup.xml_read_v(m.uvs, n_uvs);
					if (auto n_normals = n_mesh.child("normals"); n_normals)
						data_soup.xml_read_v(m.normals, n_normals);
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
	}
}
