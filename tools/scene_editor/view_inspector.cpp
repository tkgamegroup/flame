#include "app.h"
#include "selection.h"
#include "view_inspector.h"
#include "dialog.h"

#include <flame/foundation/typeinfo.h>
#include <flame/foundation/typeinfo_serialize.h>
#include <flame/graphics/extension.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/material.h>
#include <flame/graphics/model.h>
#include <flame/graphics/animation.h>
#include <flame/universe/components/node.h>
#include <flame/universe/components/armature.h>
#include <flame/universe/components/terrain.h>

#include <FortuneAlgorithm/FortuneAlgorithm.h>

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
		editing_vector.v.resize(size * item_size);
		if (old_size < size)
		{
			for (auto i = old_size; i < size; i++)
				type->create(vec.data() + i * item_size);
		}
		else if (old_size > size)
		{
			for (auto i = size; i < old_size; i++)
				type->destroy(vec.data() + i * item_size, false);
		}
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
					if (ImGui::Button("Procedure Terrain"))
					{
						struct ProcedureTerrainDialog : Dialog
						{
							cTerrainPtr terrain;
							bool update_height = true;
							int voronoi_sites_count = 20;
							int voronoi_layer1_precentage = 50;
							int voronoi_layer2_precentage = 50;
							int voronoi_layer3_precentage = 50;
							bool update_cliff = true;
							std::vector<std::filesystem::path> cliff_models;

							bool update_splash = true;
							uint splash_layers = 0;
							float splash_bar1 = 45.f;
							float splash_bar2 = 80.f;
							float splash_bar3 = 90.f;
							float splash_transition = 4.f;

							static void open(cTerrainPtr terrain)
							{
								auto material = terrain->material;
								if (material)
								{
									auto dialog = new ProcedureTerrainDialog;
									dialog->title = "Procedure Terrain";
									dialog->terrain = terrain;
									for (auto& d : material->shader_defines)
									{
										auto sp = SUS::split(d, '=');
										auto _sp = SUS::split(sp.front(), ':');
										if (_sp.back() == "LAYERS")
										{
											if (sp.size() > 1)
												dialog->splash_layers = s2t<uint>(sp[1]);
											break;
										}
									}
									Dialog::open(dialog);
								}
							}

							void draw() override
							{
								auto open = true;
								if (ImGui::Begin(title.c_str(), &open))
								{
									if (ImGui::CollapsingHeader("Height"))
									{
										ImGui::Checkbox("Update##height", &update_height);
										ImGui::InputInt("Sites Count", &voronoi_sites_count);
										ImGui::InputInt("Layer1 Precentage", &voronoi_layer1_precentage);
										if (ImGui::CollapsingHeader("Cliff"))
										{
											ImGui::Checkbox("Update##cliff", &update_cliff);
										}
									}
									if (ImGui::CollapsingHeader("Splash"))
									{
										ImGui::Checkbox("Update##splash", &update_splash);
										switch (splash_layers)
										{
										case 2:
											ImGui::DragFloat("Bar1", &splash_bar1, 1.f, 0.f, 90.f);
											break;
										case 3:
											ImGui::DragFloat("Bar1", &splash_bar1, 1.f, 0.f, 90.f);
											ImGui::DragFloat("Bar2", &splash_bar2, 1.f, splash_bar1, 90.f);
											break;
										case 4:
											ImGui::DragFloat("Bar1", &splash_bar1, 1.f, 0.f, 90.f);
											ImGui::DragFloat("Bar2", &splash_bar2, 1.f, splash_bar1, splash_bar3);
											ImGui::DragFloat("Bar3", &splash_bar3, 1.f, splash_bar2, 90.f);
											break;
										}
										ImGui::DragFloat("Transition", &splash_transition, 1.f, 0.f, 90.f);
									}
									if (ImGui::CollapsingHeader("Spawn"))
									{

									}
									if (ImGui::Button("Generate"))
									{
										auto height_map = terrain->height_map;
										auto splash_map = terrain->splash_map;

										if (update_height)
										{
											graphics::Queue::get()->wait_idle();

											auto seed = std::chrono::system_clock::now().time_since_epoch().count();
											std::default_random_engine generator(seed);
											std::uniform_real_distribution<double> distribution_d(0.0, 1.0);

											std::vector<Vector2> points;
											for (int i = 0; i < voronoi_sites_count; ++i)
												points.push_back(Vector2{ distribution_d(generator), distribution_d(generator) });

											auto get_diagram = [](const std::vector<Vector2>& points) {
												FortuneAlgorithm fortune_algorithm(points);
												fortune_algorithm.construct();
												fortune_algorithm.bound(Box{ -0.05, -0.05, 1.05, 1.05 });
												auto diagram = fortune_algorithm.getDiagram(); 
												diagram.intersect(Box{ 0.0, 0.0, 1.0, 1.0 });
												return diagram;
											};

											auto get_site_edges = [](VoronoiDiagram::Site* site) {
												std::vector<VoronoiDiagram::HalfEdge*> ret;
												auto face = site->face;
												auto half_edge = face->outerComponent;
												if (half_edge)
												{
													while (half_edge->prev != nullptr)
													{
														half_edge = half_edge->prev;
														if (half_edge == face->outerComponent)
															break;
													}

													auto start_edge = half_edge;
													while (half_edge != nullptr)
													{
														ret.push_back(half_edge);
														half_edge = half_edge->next;
														if (half_edge == start_edge)
															break;
													}
												}
												return ret;
											};

											auto to_vec2 = [](const Vector2& src) {
												return vec2(src.x, src.y);
											};

											auto get_site_vertices = [&](VoronoiDiagram::Site* site) {
												std::vector<vec2> ret;
												auto edges = get_site_edges(site);
												for (auto edge : edges)
												{
													if (edge->origin != nullptr && edge->destination != nullptr)
													{
														if (ret.empty())
															ret.push_back(to_vec2(edge->origin->point));
														ret.push_back(to_vec2(edge->destination->point));
													}
												}
												return ret;
											};

											for (auto t = 0; t < 3; t++)
											{
												auto diagram = get_diagram(points);
												for (auto i = 0; i < diagram.getNbSites(); i++)
												{
													auto vertices = get_site_vertices(diagram.getSite(i));
													auto centroid = convex_centroid(vertices);
													points[i] = Vector2(centroid.x, centroid.y);
												}
											}

											std::vector<float> site_height;
											site_height.resize(points.size());
											for (auto i = 0; i < points.size(); i++)
											{
												auto value = 0.f;
												if (distribution_d(generator) * 100.f > voronoi_layer1_precentage)
												{
													value += 0.25f;
													if (distribution_d(generator) * 100.f > voronoi_layer2_precentage)
													{
														value += 0.25f;
														if (distribution_d(generator) * 100.f > voronoi_layer3_precentage)
															value += 0.25f;
													}
												}
												site_height[i] = value;
											}

											auto diagram = get_diagram(points);
											{
												const auto MaxVertices = 10000;
												graphics::StagingBuffer vtx_buf(sizeof(vec2) * MaxVertices, nullptr, graphics::BufferUsageVertex);
												graphics::InstanceCB cb(nullptr);

												auto fb = height_map->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear);
												auto pt = graphics::PrimitiveTopologyTriangleFan;
												auto pl = graphics::GraphicsPipeline::get(L"flame\\shaders\\noise\\fbm.pipeline",
													{ "vs=uv_as_pos.vert",
													  "rp=" + str(fb->renderpass),
													  "pt=" + TypeInfo::serialize_t(&pt) });
												graphics::PipelineResourceManager<FLAME_UID> prm;
												prm.init(pl->layout);

												cb->image_barrier(height_map, {}, graphics::ImageLayoutAttachment);
												cb->set_viewport_and_scissor(Rect(vec2(0.f), vec2(height_map->size)));
												cb->begin_renderpass(nullptr, fb, { vec4(0.f) });
												cb->bind_pipeline(pl);
												prm.set_pc_var<"falloff"_h>(0.f);
												prm.set_pc_var<"power"_h>(1.f);
												prm.set_pc_var<"uv_off"_h>(vec2(19.7f, 43.3f));
												prm.set_pc_var<"uv_scl"_h>(32.f);
												prm.push_constant(cb.get());
												cb->bind_vertex_buffer(vtx_buf.get(), 0);

												auto pvtx = (vec2*)vtx_buf->mapped;
												auto vtx_cnt = 0;
												for (auto i = 0; i < points.size(); i++)
												{
													auto vertices = get_site_vertices(diagram.getSite(i));
													if (!vertices.empty() && vtx_cnt + vertices.size() <= MaxVertices)
													{
														for (auto j = 0; j < vertices.size(); j++)
															pvtx[vtx_cnt + j] = vertices[j];
														auto base = site_height[i];
														prm.set_pc_var<"val_base"_h>(base);
														prm.set_pc_var<"val_scl"_h>(0.25f);
														prm.push_constant(cb.get(), prm.vu_pc.var_off<"uv_off"_h>(), sizeof(float) * 5);
														cb->draw(vertices.size(), 1, vtx_cnt, 0);
														vtx_cnt += vertices.size();
													}
												}

												cb->end_renderpass();
												cb->image_barrier(height_map, {}, graphics::ImageLayoutShaderReadOnly);
											}

											height_map->clear_staging_data();
											terrain->update_normal_map();

											auto asset = AssetManagemant::find(height_map->filename);
											if (asset)
												asset->active = false;
											height_map->save(height_map->filename);
											if (asset)
												asset->active = true;

											if (update_cliff)
											{
												auto e_cliff = terrain->entity->find_child("cliff");
												if (!e_cliff)
												{
													e_cliff = Entity::create();
													e_cliff->name = "cliff";
													e_cliff->add_component(th<cNode>());
													terrain->entity->add_child(e_cliff);
												}
												e_cliff->remove_all_children();
												auto ext = terrain->extent;
												for (auto i = 0; i < points.size(); i++)
												{
													auto self_height = site_height[i];
													if (self_height > 0.f)
													{
														auto site = diagram.getSite(i);
														auto edges = get_site_edges(site);
														for (auto edge : edges)
														{
															auto oth_edge = edge->twin;
															if (oth_edge)
															{
																auto oth_height = site_height[oth_edge->incidentFace->site->index];
																if (oth_height < self_height)
																{
																	auto pa = to_vec2(edge->origin->point);
																	auto pb = to_vec2(edge->destination->point);
																	auto try_num = 20;
																	auto spawn_area_length = distance(pa, pb);
																	auto spawn_area_height = ext.y * (self_height - oth_height);
																	for (auto t = 0; t < try_num; t++)
																	{

																	}
																	auto e_rock = Entity::create();
																	e_rock->load(L"assets/rocks/1.prefab");
																	auto node = e_rock->get_component_i<cNode>(0);
																	auto quat = angleAxis(atan2(spawn_area_height, ext.x * 2.f / height_map->size.x),
																		normalize(vec3(pa.x, 0.f, pa.y) - vec3(pb.x, 0.f, pb.y)));
																	auto axis = mat3(quat);
																	node->set_pos((vec3(pa.x + pb.x, self_height + oth_height, pa.y + pb.y) * 0.5f) * ext + 
																		axis[1] * 0.5f);
																	node->set_qut(quat);
																	node->set_scl(vec3(4.f));
																	e_cliff->add_child(e_rock);
																}
															}
														}
													}
												}
											}
										}
										if (update_splash)
										{
											graphics::Queue::get()->wait_idle();

											{
												graphics::InstanceCB cb(nullptr);

												auto fb = splash_map->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear);
												auto pl = graphics::GraphicsPipeline::get(L"flame\\shaders\\terrain\\auto_splash.pipeline",
													{ "rp=" + str(fb->renderpass),
													  "frag:LAYERS=" + str(splash_layers) });
												graphics::PipelineResourceManager<FLAME_UID> prm;
												prm.init(pl->layout);

												cb->image_barrier(splash_map, {}, graphics::ImageLayoutAttachment);
												cb->set_viewport_and_scissor(Rect(vec2(0.f), vec2(splash_map->size)));
												cb->begin_renderpass(nullptr, fb, { vec4(0.f) });
												cb->bind_pipeline(pl);
												prm.set_pc_var<"bar"_h>(vec4(splash_bar1, splash_bar2, splash_bar3, 0.f));
												prm.set_pc_var<"transition"_h>(splash_transition);
												prm.push_constant(cb.get());
												cb->bind_descriptor_set(0, terrain->normal_map->get_shader_read_src());
												cb->draw(3, 1, 0, 0);
												cb->end_renderpass();
												cb->image_barrier(splash_map, {}, graphics::ImageLayoutShaderReadOnly);
											}

											auto asset = AssetManagemant::find(splash_map->filename);
											if (asset)
												asset->active = false;
											splash_map->save(splash_map->filename);
											if (asset)
												asset->active = true;
										}
									}
									ImGui::SameLine();
									if (ImGui::Button("Close"))
										close();

									ImGui::End();
								}
								if (!open)
									close();
							}
						};

						ProcedureTerrainDialog::open(terrain);
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
					"R", "G", "B", "A",
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
