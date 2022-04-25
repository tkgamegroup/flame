#include "app.h"
#include "selection.h"
#include "view_inspector.h"
#include "dialog.h"

#include <flame/foundation/typeinfo.h>
#include <flame/graphics/extension.h>
#include <flame/graphics/material.h>
#include <flame/graphics/model.h>
#include <flame/graphics/animation.h>
#include <flame/universe/components/armature.h>
#include <flame/universe/components/terrain.h>

View_Inspector view_inspector;

View_Inspector::View_Inspector() :
	View("Inspector")
{
}

struct EditingVector
{
	const void* id;
	std::vector<char> v;
	TypeInfo* type = nullptr;
	uint item_size = 0;

	void clear()
	{
		id = nullptr;
		if (type)
		{
			auto n = count();
			for (auto i = 0; i < n; i++)
				type->destroy(v.data() + i * item_size, false);
			v.clear();
			type = nullptr;
			item_size = 0;
		}
	}

	void set(const void* _id, TypeInfo* _type, void* _vec)
	{
		id = _id;
		type = _type;
		item_size = type->size;
		assign(nullptr, _vec);
	}

	uint count()
	{
		return v.size() / item_size;
	}

	void resize(void* _vec, uint size)
	{
		if (!_vec)
			_vec = &v;
		auto& vec = *(std::vector<char>*)_vec;
		auto old_size = vec.size() / item_size;
		for (auto i = 0; i < old_size; i++)
			type->destroy(vec.data() + i * item_size, false);
		editing_vector.v.resize(size * item_size);
		for (auto i = 0; i < size; i++)
			type->create(vec.data() + i * item_size);
	}

	void assign(void* _dst, void* _src)
	{
		if (!_dst)
			_dst = &v;
		if (!_src)
			_src = &v;
		auto& dst = *(std::vector<char>*)_dst;
		auto& src = *(std::vector<char>*)_src;
		dst.resize(src.size());
		auto count = src.size() / item_size;
		for (auto i = 0; i < count; i++)
		{
			auto p = dst.data() + i * item_size;
			type->create(p);
			type->copy(p, src.data() + i * item_size);
		}
	}
}editing_vector;

std::string show_udt(const UdtInfo& ui, void* src);

bool show_variable(const UdtInfo& ui, TypeInfo* type, const std::string& name, int offset, int getter_idx, int setter_idx, void* src, const void* id) 
{
	auto changed = false;
	auto direct_io = getter_idx == -1 && setter_idx == -1;

	ImGui::PushID(id);
	switch (type->tag)
	{
	case TagD:
	{
		auto data = ui.get_value(type, src, offset, getter_idx, !direct_io);
		auto ti = (TypeInfo_Data*)type;
		switch (ti->data_type)
		{
		case DataBool:
			changed = ImGui::Checkbox(name.c_str(), (bool*)data);
			break;
		case DataInt:
			switch (ti->vec_size)
			{
			case 1:
				changed = ImGui::DragInt(name.c_str(), (int*)data);
				break;
			}
			break;
		case DataFloat:
			switch (ti->vec_size)
			{
			case 1:
				changed = ImGui::DragFloat(name.c_str(), (float*)data, 0.01f);
				break;
			case 2:
				changed = ImGui::DragFloat2(name.c_str(), (float*)data, 0.01f);
				break;
			case 3:
				changed = ImGui::DragFloat3(name.c_str(), (float*)data, 0.01f);
				break;
			case 4:
				changed = ImGui::DragFloat4(name.c_str(), (float*)data, 0.01f);
				break;
			}
			break;
		case DataString:
			changed = ImGui::InputText(name.c_str(), (std::string*)data);
			break;
		case DataWString:
			break;
		case DataPath:
		{
			auto& path = *(std::filesystem::path*)data;
			auto s = path.string();
			ImGui::InputText(name.c_str(), s.data(), ImGuiInputTextFlags_ReadOnly);
			if (ImGui::BeginDragDropTarget())
			{
				if (auto payload = ImGui::AcceptDragDropPayload("File"); payload)
				{
					path = Path::reverse(std::wstring((wchar_t*)payload->Data));
					changed = true;
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::SameLine();
			if (ImGui::Button("P"))
			{
				auto p = path;
				selection.select(Path::get(p));
			}
		}
			break;
		}
		if (changed && !direct_io)
			ui.set_value(type, src, offset, setter_idx, nullptr);
	}
		break;
	case TagVD:
		if (ImGui::TreeNode(name.c_str()))
		{
			auto ti = ((TypeInfo_VectorOfData*)type)->ti;
			if (editing_vector.id == id)
			{
				if (ImGui::Button("Save"))
				{
					if (setter_idx == -1)
						editing_vector.assign((char*)src + offset, nullptr);
					else
						ui.set_value(type, src, offset, setter_idx, &editing_vector.v);
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
					editing_vector.clear();
				if (editing_vector.id)
				{
					int n = editing_vector.count();
					if (ImGui::InputInt("size", &n, 1, 1))
					{
						n = clamp(n, 0, 16);
						editing_vector.resize(nullptr, n);
					}
					for (auto i = 0; i < n; i++)
					{
						if (show_variable(ui, ti, str(i), i * ti->size, -1, -1, editing_vector.v.data(), id))
							changed = true;
					}
				}
			}
			else
			{
				if (ImGui::Button("Edit"))
					editing_vector.set(id, ti, (char*)src + offset);
			}
			ImGui::TreePop();
		}
		break;
	case TagVU:
		if (ImGui::TreeNode(name.c_str()))
		{
			auto ti = ((TypeInfo_VectorOfUdt*)type)->ti;
			if (editing_vector.id == id)
			{
				if (ImGui::Button("Save"))
				{
					if (setter_idx == -1)
						editing_vector.assign((char*)src + offset, nullptr);
					else
						ui.set_value(type, src, offset, setter_idx, &editing_vector.v);
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
					editing_vector.clear();
				if (editing_vector.id)
				{
					auto& ui = *ti->retrive_ui();
					int n = editing_vector.count();
					if (ImGui::InputInt("size", &n, 1, 1))
					{
						n = clamp(n, 0, 16);
						editing_vector.resize(nullptr, n);
					}
					for (auto i = 0; i < n; i++)
					{
						if (ImGui::TreeNode(str(i).c_str()))
						{
							show_udt(ui, editing_vector.v.data() + ui.size * i);
							ImGui::TreePop();
						}
					}
				}
			}
			else
			{
				if (ImGui::Button("Edit"))
					editing_vector.set(id, ti, (char*)src + offset);
			}
			ImGui::TreePop();
		}
		break;
	case TagVR:
		if (ImGui::TreeNode(name.c_str()))
		{
			auto ti = ((TypeInfo_VectorOfPair*)type)->ti;
			if (editing_vector.id == id)
			{
				if (ImGui::Button("Save"))
				{
					if (setter_idx == -1)
						editing_vector.assign((char*)src + offset, nullptr);
					else
						ui.set_value(type, src, offset, setter_idx, &editing_vector.v);
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
					editing_vector.clear();
				if (editing_vector.id)
				{
					int n = editing_vector.count();
					if (ImGui::InputInt("size", &n, 1, 1))
					{
						n = clamp(n, 0, 16);
						editing_vector.resize(nullptr, n);
					}
					for (auto i = 0; i < n; i++)
					{
						if (ImGui::TreeNode(str(i).c_str()))
						{
							auto p = editing_vector.v.data() + ti->size * i;
							show_variable(ui, ti->ti1, "first", 0, -1, -1, ti->first(p), id);
							show_variable(ui, ti->ti2, "second", 0, -1, -1, ti->second(p), id);
							ImGui::TreePop();
						}
					}
				}
			}
			else
			{
				if (ImGui::Button("Edit"))
					editing_vector.set(id, ti, (char*)src + offset);
			}
			ImGui::TreePop();
		}
		break;
	}
	ImGui::PopID();

	return changed;
}

std::string show_udt(const UdtInfo& ui, void* src)
{
	std::string changed_name;

	if (ui.attributes.empty())
	{
		for (auto& v : ui.variables)
		{
			if (show_variable(ui, v.type, v.name, v.offset, -1, -1, src, &v))
				changed_name = v.name;
		}
	}
	else
	{
		for (auto& a : ui.attributes)
		{
			if (show_variable(ui, a.type, a.name, a.var_off(), a.getter_idx, a.setter_idx, src, &a))
				changed_name = a.name;
		}
	}

	return changed_name;
}

static std::unordered_map<uint, UdtInfo*> com_udts;
void get_com_udts()
{
	for (auto& ui : tidb.udts)
	{
		if (ui.second.base_class_name == "flame::Component")
			com_udts.emplace(ui.first, &ui.second);
	}
}

void View_Inspector::on_draw()
{
	if (com_udts.empty())
		get_com_udts();

	if (ImGui::Button(graphics::FontAtlas::icon_s("arrow-left"_h).c_str()))
		selection.backward();
	ImGui::SameLine();
	if (ImGui::Button(graphics::FontAtlas::icon_s("arrow-right"_h).c_str()))
		selection.forward();

	static uint last_sel_ref_frame = 0;
	static uint last_sel_ref_type = 0;
	static void* last_sel_ref_obj = nullptr;
	if (selection.frame != last_sel_ref_frame)
	{
		editing_vector.clear();

		switch (last_sel_ref_type)
		{
		case th<graphics::Image>():
			graphics::Image::release((graphics::ImagePtr)last_sel_ref_obj);
			break;
		case th<graphics::Material>():
			graphics::Material::release((graphics::MaterialPtr)last_sel_ref_obj);
			break;
		}
		last_sel_ref_type = 0;
		last_sel_ref_obj = nullptr;
	}

	switch (selection.type)
	{
	case Selection::tEntity:
	{
		auto e = selection.entity();

		ImGui::PushID(e);
		auto changed_name = show_udt(*TypeInfo::get<Entity>()->retrive_ui(), e);
		ImGui::PopID();
		if (!changed_name.empty())
		{
			if (auto ins = get_prefab_instance(e); ins)
				ins->mark_modifier(e->file_id, "", changed_name);
		}

		static ComponentPtr target_component = nullptr;
		auto open_component_menu = false;
		for (auto& c : e->components)
		{
			ImGui::PushID(c.get());
			auto& ui = *com_udts[c->type_hash];
			auto open = ImGui::CollapsingHeader(ui.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap);
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
			if (ImGui::Button("..."))
			{
				target_component = c.get();
				open_component_menu = true;
			}
			if (open)
			{
				auto changed_name = show_udt(ui, c.get());
				if (!changed_name.empty())
				{
					if (auto ins = get_prefab_instance(e); ins)
						ins->mark_modifier(e->file_id, ui.name, changed_name);
				}

				if (ui.name == "flame::cArmature")
				{
					auto armature = (cArmaturePtr)c.get();
					static char name[100];
					ImGui::InputText("name", name, countof(name));
					ImGui::SameLine();
					if (ImGui::Button("Play"))
						armature->play(sh(name));
					ImGui::SameLine();
					if (ImGui::Button("Stop"))
						armature->stop();
				}
				else if (ui.name == "flame::cTerrain")
				{
					auto terrain = (cTerrainPtr)c.get();
					if (ImGui::Button("Auto Splash"))
					{
						struct AutoSplashDialog : Dialog
						{
							cTerrainPtr terrain;
							uint layers = 0;
							float bar1 = 1.f; 
							float bar2 = 1.f;
							float bar3 = 1.f;
							float transition = 0.05f;

							static void open(cTerrainPtr terrain)
							{
								auto material = terrain->material;
								if (material)
								{
									auto dialog = new AutoSplashDialog;
									dialog->title = "Splash";
									dialog->terrain = terrain;
									for (auto& d : material->shader_defines)
									{
										auto sp = SUS::split(d, '=');
										auto _sp = SUS::split(sp.front(), ':');
										if (_sp.back() == "LAYERS")
										{
											if (sp.size() > 1)
												dialog->layers = s2t<uint>(sp[1]);
											break;
										}
									}
									Dialog::open(dialog);
								}
							}

							void draw() override
							{
								if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
								{
									switch (layers)
									{
									case 2:
										ImGui::DragFloat("Bar1", &bar1, 0.05f, 0.f, 1.f);
										break;
									}
									ImGui::DragFloat("Transition", &transition, 0.01f, 0.f, 0.5f);
									if (ImGui::Button("OK"))
									{
										graphics::Queue::get()->wait_idle();
										auto& mat_res = app.renderer->get_material_res_info(terrain->material_res_id);
										auto splash_map = mat_res.texs[0].second;
										auto normal_map = terrain->normal_map;

										{
											auto splash_map_sz = splash_map->size;
											auto normal_map_sz = normal_map->size;
											graphics::StagingBuffer stag(sizeof(cvec4) * splash_map_sz.x * splash_map_sz.y);

											auto data = (cvec4*)stag->mapped;

											for (auto y = 0; y < splash_map_sz.y; y++)
											{
												for (auto x = 0; x < splash_map_sz.x; x++)
												{
													auto nor = vec3(normal_map->linear_sample(vec2((float)x / splash_map_sz.x, (float)y / splash_map_sz.y)));
													nor = nor * 2.f - 1.f;
													float ndoty = dot(nor, vec3(0, 1, 0));

													auto interpolate = [](float v, float off, float len, float transition)
													{
														float a0 = off - transition * 0.5;
														float a1 = off + transition * 0.5;
														float b0 = off + len - transition * 0.5;
														float b1 = off + len + transition * 0.5;
														if (v <= a0 || v >= b1)
															return 0.f;
														if (v >= a1 && v <= b0)
															return 1.f;
														if (v < a1)
														{
															if (a0 < 0)
																return 1.f;
															return 1.f - (a1 - v) / transition;
														}
														else if (v > b0)
														{
															if (b1 > 1)
																return 1.f;
															return 1.f - (v - b0) / transition;
														}
														return 0.f;
													};

													switch (layers)
													{
													case 2:
													{
														vec2 weight;
														weight[0] = interpolate(ndoty, 0.f, bar1, transition);
														weight[1] = interpolate(ndoty, bar1, 1.f - bar1, transition);
														weight /= weight[0] + weight[1];
														*data = cvec4(weight[0] * 255.f, weight[1] * 255.f, 0.f, 0.f);
													}
													break;
													default:
														*data = cvec4(0);
													}
													data++;
												}
											}

											graphics::InstanceCB cb(nullptr);
											graphics::BufferImageCopy cpy;
											cpy.img_ext = splash_map_sz;

											cb->image_barrier(splash_map, cpy.img_sub, graphics::ImageLayoutTransferDst);
											cb->copy_buffer_to_image(stag.get(), splash_map, { &cpy, 1 });
											cb->image_barrier(splash_map, cpy.img_sub, graphics::ImageLayoutShaderReadOnly);
										}

										close();
										ImGui::CloseCurrentPopup();
									}
									ImGui::SameLine();
									if (ImGui::Button("Cancel"))
									{
										close();
										ImGui::CloseCurrentPopup();
									}

									ImGui::EndPopup();
								}
							}
						};

						AutoSplashDialog::open(terrain);
					}
					if (ImGui::Button("Auto Spawn"))
					{

					}
				}
			}
			ImGui::PopID();
		}

		ImGui::Dummy(vec2(0.f, 10.f));
		const float ButtonWidth = 100.f;
		ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ButtonWidth) * 0.5f);
		ImGui::SetNextItemWidth(ButtonWidth);
		if (ImGui::Button("Add Component"))
		{
			if (get_prefab_instance(e))
				MessageDialog::open("[RestructurePrefabInstanceWarnning]", "");
			else
				ImGui::OpenPopup("add_component");
		}

		if (open_component_menu)
		{
			ImGui::OpenPopup("component_menu");
			open_component_menu = false;
		}
		if (ImGui::BeginPopup("component_menu"))
		{
			if (ImGui::Selectable("Remove"))
				e->remove_component(target_component->type_hash);
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopup("add_component"))
		{
			for (auto ui : com_udts)
			{
				if (ImGui::Selectable(ui.second->name.c_str()))
					e->add_component(ui.first);
			}
			ImGui::EndPopup();
		}

		last_sel_ref_frame = selection.frame;
	}
		break;
	case Selection::tPath:
	{
		auto& path = selection.path();
		ImGui::TextUnformatted(Path::reverse(path).string().c_str());
		auto ext = path.extension();
		if (ext == L".obj" || ext == L".fbx" || ext == L".gltf" || ext == L".glb")
		{
			static vec3 rotation = vec3(0, 0, 0);
			static vec3 scaling = vec3(0.01f, 0.01f, 0.01f);
			static bool only_animation = false;
			ImGui::DragFloat3("rotation", (float*)&rotation);
			ImGui::DragFloat3("scaling", (float*)&scaling);
			ImGui::Checkbox("only animation", &only_animation);
			if (ImGui::Button("Convert"))
				graphics::Model::convert(path, rotation, scaling, only_animation);
		}
		else if (is_image_file(ext))
		{
			if (selection.frame != last_sel_ref_frame)
			{
				auto image = graphics::Image::get(path);
				if (image)
				{
					last_sel_ref_type = th<graphics::Image>();
					last_sel_ref_obj = image;
				}

				last_sel_ref_frame = selection.frame;
			}

			if (last_sel_ref_obj)
			{
				auto image = (graphics::ImagePtr)last_sel_ref_obj;
				ImGui::Text("format: %s", TypeInfo::serialize_t(&image->format).c_str());
				ImGui::Text("size: %s", str(image->size).c_str());
				static int view_type = ImGui::ImageViewRGBA;
				static const char* types[] = {
					"RGBA",
					"R",
					"G",
					"B",
					"A",
					"RGB",
				};
				ImGui::Combo("view", &view_type, types, countof(types));
				if (view_type != 0)
					ImGui::PushImageViewType((ImGui::ImageViewType)view_type);
				ImGui::Image(last_sel_ref_obj, (vec2)image->size);
				if (view_type != 0)
					ImGui::PopImageViewType();
				if (ImGui::Button("Save"))
				{
					auto asset = AssetManagemant::find(path);
					if (asset)
						asset->active = false;
					image->save(path);
					if (asset)
						asset->active = true;
				}
			}
		}
		else if (ext == L".fmat")
		{
			if (selection.frame != last_sel_ref_frame)
			{
				auto material = graphics::Material::get(path);
				if (material)
				{
					last_sel_ref_type = th<graphics::Material>();
					last_sel_ref_obj = material;
				}

				last_sel_ref_frame = selection.frame;
			}

			if (last_sel_ref_obj)
			{
				auto material = (graphics::MaterialPtr)last_sel_ref_obj;
				if (!show_udt(*TypeInfo::get<graphics::Material>()->retrive_ui(), material).empty())
				{
					auto id = app.renderer->get_material_res(material, -2);
					if (id > 0)
						app.renderer->update_res(id, "material"_h, "parameters"_h);
				}
				if (ImGui::Button("Save"))
				{
					auto asset = AssetManagemant::find(path);
					if (asset)
						asset->active = false;
					material->save(path);
					if (asset)
						asset->active = true;
				}
			}
		}
	}
		break;
	}
}
