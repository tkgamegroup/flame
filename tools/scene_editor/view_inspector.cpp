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

const Attribute* show_udt(const UdtInfo& ui, void* src);

bool show_variable(TypeInfo* type, const std::string& name, void* data, const void* id)
{
	auto changed = false;

	ImGui::PushID(id);
	switch (type->tag)
	{
	case TagD:
	{
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
				changed = ImGui::DragFloat(name.c_str(), (float*)data);
				break;
			case 2:
				changed = ImGui::DragFloat2(name.c_str(), (float*)data);
				break;
			case 3:
				changed = ImGui::DragFloat3(name.c_str(), (float*)data);
				break;
			case 4:
				changed = ImGui::DragFloat4(name.c_str(), (float*)data);
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
	}
		break;
	case TagVD:
		break;
	case TagVU:
		if (ImGui::TreeNode(name.c_str()))
		{
			auto ti = (TypeInfo_VectorOfUdt*)type;
			auto& ui = *ti->retrive_ui();
			auto& vec = *(std::vector<uchar>*)data;
			auto size = (int)vec.size() / (int)ui.size;
			ImGui::InputInt("size", &size, 1, 1);
			for (auto i = 0; i < size; i++)
			{
				if (ImGui::TreeNode(str(i).c_str()))
				{
					show_udt(ui, vec.data() + ui.size * i);
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}
		break;
	}
	ImGui::PopID();

	return changed;
}

const Attribute* show_udt(const UdtInfo& ui, void* src)
{
	const Attribute* changed_attribute = nullptr;

	if (ui.attributes.empty())
	{
		for (auto& v : ui.variables)
			show_variable(v.type, v.name, (char*)src + v.offset, &v);
	}
	else
	{
		for (auto& a : ui.attributes)
		{
			auto direct_io = a.getter_idx == -1 && a.setter_idx == -1;
			if (show_variable(a.type, a.name, a.get_value(src, !direct_io), &a))
			{
				if (!direct_io)
					a.set_value(src);
				changed_attribute = &a;
			}
		}
	}

	return changed_attribute;
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
		switch (last_sel_ref_type)
		{
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
		auto changed_attribute = show_udt(*TypeInfo::get<Entity>()->retrive_ui(), e);
		ImGui::PopID();
		if (changed_attribute)
		{
			if (auto ins = get_prefab_instance(e); ins)
				ins->set_modifier(e->file_id, "", changed_attribute->name, changed_attribute->serialize(e));
		}

		ComponentPtr com_menu_tar = nullptr;
		for (auto& c : e->components)
		{
			ImGui::PushID(c.get());
			auto& ui = *com_udts[c->type_hash];
			auto open = ImGui::CollapsingHeader(ui.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap);
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
			if (ImGui::Button("..."))
				com_menu_tar = c.get();
			if (open)
			{
				auto changed_attribute = show_udt(ui, c.get());
				if (changed_attribute)
				{
					if (auto ins = get_prefab_instance(e); ins)
						ins->set_modifier(e->file_id, ui.name, changed_attribute->name, changed_attribute->serialize(c.get()));
				}

				if (ui.name == "flame::cArmature")
				{
					auto armature = (cArmaturePtr)c.get();
					if (ImGui::Button("Bind Animation"))
					{
						SelectResourceDialog::open("Select animation", [armature](bool ok, const std::filesystem::path& path) {
							if (ok && !path.empty())
							{
								InputDialog::open("Name to bind", [armature, path](bool ok, const std::string& text) {
									if (ok && !text.empty())
										armature->bind_animation(sh(text.c_str()), path);
								});
							}
						});
					}
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
					if (ImGui::Button("Splash By Normal"))
					{
						struct SplashDialog : Dialog
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
									auto dialog = new SplashDialog;
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

						SplashDialog::open(terrain);
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

		if (com_menu_tar)
			ImGui::OpenPopup("component_menu");
		if (ImGui::BeginPopup("component_menu"))
		{
			if (ImGui::Selectable("Remove"))
				e->remove_component(com_menu_tar->type_hash);
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
		else if (ext == L".fmat")
		{
			if (selection.frame != last_sel_ref_frame)
			{
				auto material = graphics::Material::get(path);
				if (material)
				{
					last_sel_ref_frame = selection.frame;
					last_sel_ref_type = th<graphics::Material>();
					last_sel_ref_obj = material;
				}
			}

			if (last_sel_ref_obj)
				show_udt(*TypeInfo::get<graphics::Material>()->retrive_ui(), last_sel_ref_obj);
		}
	}
		break;
	}
}
