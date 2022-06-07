#include "app.h"
#include "selection.h"
#include "view_inspector.h"

#include <flame/foundation/typeinfo.h>
#include <flame/foundation/typeinfo_serialize.h>
#include <flame/graphics/extension.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/material.h>
#include <flame/graphics/model.h>
#include <flame/graphics/animation.h>
#include <flame/graphics/debug.h>
#include <flame/universe/components/node.h>
#include <flame/universe/components/mesh.h>
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
				selection.select(Path::get(path));
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
		if (e->prefab)
		{
			auto& path = e->prefab->filename;
			auto str = path.string();
			ImGui::InputText("prefab", str.data(), ImGuiInputTextFlags_ReadOnly);
			ImGui::SameLine();
			if (ImGui::Button("P"))
				selection.select(Path::get(path));
		}
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
				ImGui::Checkbox("enable", &c->enable);
				auto changed_name = show_udt(ui, c.get());
				if (!changed_name.empty())
				{
					if (auto ins = get_prefab_instance(e); ins)
						ins->mark_modifier(e->file_id, ui.name, changed_name);
				}

				if (ui.name == "flame::cNode")
				{
					auto node = (cNodePtr)c.get();
					ImGui::InputFloat4("qut", (float*)&node->qut, "%.3f", ImGuiInputTextFlags_ReadOnly);
					ImGui::InputFloat3("g_pos", (float*)&node->g_pos, "%.3f", ImGuiInputTextFlags_ReadOnly);
					ImGui::InputFloat3("g_rot.x", (float*)&node->g_rot[0], "%.3f", ImGuiInputTextFlags_ReadOnly);
					ImGui::InputFloat3("g_rot.y", (float*)&node->g_rot[1], "%.3f", ImGuiInputTextFlags_ReadOnly);
					ImGui::InputFloat3("g_rot.z", (float*)&node->g_rot[2], "%.3f", ImGuiInputTextFlags_ReadOnly);
					ImGui::InputFloat3("g_scl", (float*)&node->g_scl, "%.3f", ImGuiInputTextFlags_ReadOnly);
				}
				else if (ui.name == "flame::cArmature")
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
					ImGui::InputFloat("Time", &armature->playing_time, 0.f, 0.f, "%.3f", ImGuiInputTextFlags_ReadOnly);
					ImGui::DragFloat("Speed", &armature->playing_speed, 0.01f);
				}
				else if (ui.name == "flame::cTerrain")
				{
					auto terrain = (cTerrainPtr)c.get();
					if (ImGui::Button("Procedure Terrain"))
					{
						struct ProcedureTerrainDialog : ImGui::Dialog
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
										add_event([this]() {
											generate();
											return false;
										});
									}
									ImGui::SameLine();
									if (ImGui::Button("Close"))
										close();

									ImGui::End();
								}
								if (!open)
									close();
							}

							void generate()
							{
								graphics::Queue::get()->wait_idle();

								auto height_map = terrain->height_map;
								auto splash_map = terrain->splash_map;

								if (update_height)
								{
									auto seed = std::chrono::system_clock::now().time_since_epoch().count();
									std::default_random_engine generator(seed);
									std::uniform_real_distribution<float> distribution(0.0, 1.0);

									auto to_glm = [](const Vector2& v) {
										return vec2(v.x, v.y);
									};

									std::vector<Vector2> site_postions;
									for (int i = 0; i < voronoi_sites_count; ++i)
										site_postions.push_back(Vector2{ distribution(generator), distribution(generator) });

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

									auto get_site_vertices = [&](VoronoiDiagram::Site* site) {
										std::vector<vec2> ret;
										auto edges = get_site_edges(site);
										for (auto edge : edges)
										{
											if (edge->origin != nullptr && edge->destination != nullptr)
											{
												if (ret.empty())
													ret.push_back(to_glm(edge->origin->point));
												ret.push_back(to_glm(edge->destination->point));
											}
										}
										return ret;
									};

									auto diagram = get_diagram(site_postions);
									for (auto t = 0; t < 3; t++)
									{
										for (auto i = 0; i < diagram.getNbSites(); i++)
										{
											auto vertices = get_site_vertices(diagram.getSite(i));
											auto centroid = convex_centroid(vertices);
											site_postions[i] = Vector2(centroid.x, centroid.y);
										}
										diagram = get_diagram(site_postions);
									}

									std::vector<float> site_height;
									site_height.resize(site_postions.size());
									for (auto i = 0; i < site_postions.size(); i++)
									{
										auto value = 0.f;
										if (distribution(generator) * 100.f > voronoi_layer1_precentage)
										{
											value += 0.25f;
											if (distribution(generator) * 100.f > voronoi_layer2_precentage)
											{
												value += 0.25f;
												if (distribution(generator) * 100.f > voronoi_layer3_precentage)
													value += 0.25f;
											}
										}
										site_height[i] = value;
									}
									for (auto i = 0; i < site_postions.size(); i++)
									{
										auto self_height = site_height[i];
										auto max_height = 0.f;
										for (auto edge : get_site_edges(diagram.getSite(i)))
										{
											if (auto oth_edge = edge->twin; oth_edge)
											{
												auto oth_idx = oth_edge->incidentFace->site->index;
												auto height = site_height[oth_idx];
												if (height < self_height)
													max_height = max(max_height, height);
											}
										}
										site_height[i] = min(site_height[i] + 0.25f, site_height[i]);
									}

									const auto MaxVertices = 10000;

									graphics::InstanceCommandBuffer cb;
									auto fb = height_map->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear);
									auto pt = graphics::PrimitiveTopologyTriangleFan;
									auto pl = graphics::GraphicsPipeline::get(L"flame\\shaders\\noise\\fbm.pipeline",
										{ "vs=fbm.vert",
										  "all_shader:USE_VERTEX_VAL_BASE",
										  "rp=" + str(fb->renderpass),
										  "pt=" + TypeInfo::serialize_t(&pt) });

									graphics::PipelineResourceManager<FLAME_UID> prm;
									prm.init(pl->layout);

									graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageVertex, false> buf_vtx;
									buf_vtx.create(pl->vi_ui(), MaxVertices);
									buf_vtx.upload_whole(cb.get());

									cb->image_barrier(height_map, {}, graphics::ImageLayoutAttachment);
									cb->set_viewport_and_scissor(Rect(vec2(0.f), vec2(height_map->size)));
									cb->begin_renderpass(nullptr, fb, { vec4(0.f) });
									cb->bind_pipeline(pl);
									prm.set_pc_var<"uv_off"_h>(vec2(19.7f, 43.3f));
									prm.set_pc_var<"uv_scl"_h>(32.f);
									prm.set_pc_var<"val_base"_h>(0.f);
									prm.set_pc_var<"val_scl"_h>(0.125f);
									prm.set_pc_var<"falloff"_h>(0.f);
									prm.set_pc_var<"power"_h>(1.f);
									prm.push_constant(cb.get());
									cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);

									for (auto i = 0; i < site_postions.size(); i++)
									{
										auto vertices = get_site_vertices(diagram.getSite(i));
										auto vtx_off = buf_vtx.item_offset();
										if (!vertices.empty() && vtx_off + vertices.size() <= MaxVertices)
										{
											for (auto j = 0; j < vertices.size(); j++)
											{
												buf_vtx.set_var<"i_pos"_h>(vertices[j] * 2.f - 1.f);
												buf_vtx.set_var<"i_uv"_h>(vertices[j]);
												buf_vtx.set_var<"i_val_base"_h>(site_height[i]);
												buf_vtx.next_item();
											}

											cb->draw(vertices.size(), 1, vtx_off, 0);
										}
									}

									// slopes
									{
										auto slope_width = 8.f / terrain->extent.x;
										auto slope_length = 6.f / terrain->extent.x;

										std::vector<bool> site_seen(site_postions.size(), false);
										std::vector<std::vector<VoronoiDiagram::HalfEdge*>> regions;
										std::function<void(int, int, float)> form_region;
										form_region = [&](int site_idx, int region_idx, float height) {
											if (site_seen[site_idx])
												return;
											site_seen[site_idx] = true;
											auto& region = regions[region_idx];
											for (auto edge : get_site_edges(diagram.getSite(site_idx)))
											{
												if (auto oth_edge = edge->twin; oth_edge)
												{
													auto oth_idx = oth_edge->incidentFace->site->index;
													auto oth_height = site_height[oth_idx];
													if (oth_height < height && height - oth_height < 0.3f)
														region.push_back(edge);
													else if (oth_height == height)
														form_region(oth_idx, region_idx, height);
												}
											}
										};
										for (auto i = 0; i < site_postions.size(); i++)
										{
											if (!site_seen[i])
											{
												regions.emplace_back();
												form_region(i, regions.size() - 1, site_height[i]);
											}
										}
										for (auto& r : regions)
										{
											if (!r.empty())
											{
												int n = rand() % r.size();
												if (n > 0)
													n = rand() % n;
												n += 1;
												int off = rand() % r.size();
												while (n > 0)
												{
													auto idx = off % r.size();
													auto edge = r[idx];
													auto pa = to_glm(edge->origin->point);
													auto pb = to_glm(edge->destination->point);
													if (auto w = distance(pa, pb); w > slope_width)
													{
														auto dir = normalize(pb - pa);
														auto perbi = vec2(dir.y, -dir.x);
														pa = pa + dir * (w - slope_width) * 0.5f;
														pb = pa + dir * slope_width;
														auto pc = pb + perbi * slope_length;
														auto pd = pa + perbi * slope_length;

														auto hi_height = site_height[edge->incidentFace->site->index];
														auto lo_height = site_height[edge->twin->incidentFace->site->index];

														auto vtx_off = buf_vtx.item_offset();
														if (vtx_off + 4 <= MaxVertices)
														{
															buf_vtx.set_var<"i_pos"_h>(pa * 2.f - 1.f);
															buf_vtx.set_var<"i_uv"_h>(pa);
															buf_vtx.set_var<"i_val_base"_h>(hi_height);
															buf_vtx.next_item();
															buf_vtx.set_var<"i_pos"_h>(pb * 2.f - 1.f);
															buf_vtx.set_var<"i_uv"_h>(pb);
															buf_vtx.set_var<"i_val_base"_h>(hi_height);
															buf_vtx.next_item();
															buf_vtx.set_var<"i_pos"_h>(pc * 2.f - 1.f);
															buf_vtx.set_var<"i_uv"_h>(pc);
															buf_vtx.set_var<"i_val_base"_h>(lo_height);
															buf_vtx.next_item();
															buf_vtx.set_var<"i_pos"_h>(pd * 2.f - 1.f);
															buf_vtx.set_var<"i_uv"_h>(pd);
															buf_vtx.set_var<"i_val_base"_h>(lo_height);
															buf_vtx.next_item();

															cb->draw(4, 1, vtx_off, 0);
														}
													}
													r.erase(r.begin() + idx);
													off += rand() % max(1, (int)r.size() / n) + 1;
													n--;
												}
											}
										}
									}

									cb->end_renderpass();
									cb->image_barrier(height_map, {}, graphics::ImageLayoutShaderReadOnly);
									graphics::Debug::start_capture_frame();
									cb.excute();
									graphics::Debug::end_capture_frame();

									height_map->clear_staging_data();
									terrain->update_normal_map();

									height_map->save(height_map->filename);
									auto asset = AssetManagemant::find(Path::get(height_map->filename));
									if (asset)
										asset->active = false;

									if (update_cliff)
									{
										std::vector<std::pair<std::unique_ptr<Entity>, AABB>> e_objs;
										for (auto it : std::filesystem::directory_iterator(Path::get(L"assets/rocks")))
										{
											if (it.path().extension() == L".prefab")
											{
												auto e = Entity::create();
												e->load(Path::reverse(it.path()));
												AABB bounds;
												e->forward_traversal([&](EntityPtr e) {
													auto mesh = e->get_component_t<cMesh>();
													if (mesh && mesh->model)
													{
														auto node = mesh->node;
														node->update_rot();
														bounds.expand(AABB(mesh->mesh->bounds.get_points(mesh->node->rot)));
													}
													});
												e_objs.emplace_back(e, bounds);
											}
										}

										auto e_cliff = terrain->entity->find_child("cliff");
										if (!e_cliff)
										{
											e_cliff = Entity::create();
											e_cliff->name = "cliff";
											e_cliff->add_component(th<cNode>());
											terrain->entity->add_child(e_cliff);
										}
										e_cliff->remove_all_children();

										if (false) // TODO: find a good way to generate cliff
										{
											auto ext = terrain->extent;
											for (auto i = 0; i < site_postions.size(); i++)
											{
												auto self_height = site_height[i];
												if (self_height > 0.f)
												{
													auto site = diagram.getSite(i);
													auto edges = get_site_edges(site);
													for (auto edge : edges)
													{
														if (auto oth_edge = edge->twin; oth_edge)
														{
															auto oth_height = site_height[oth_edge->incidentFace->site->index];
															if (self_height > oth_height + 0.0001f)
															{
																auto pa = to_glm(edge->origin->point);
																auto pb = to_glm(edge->destination->point);
																auto spawn_area_height = self_height - oth_height;
																auto spawn_area_width = 2.f / height_map->size.x;
																auto spawn_area_slope = atan2(spawn_area_height, spawn_area_width);
																auto spawn_area_cx = distance(pa, pb);
																auto spawn_area_cy = spawn_area_height / sin(spawn_area_slope);
																for (auto x = 0.f; x < 1.f; )
																{
																	auto& obj = e_objs[int(distribution(generator) * e_objs.size())];
																	auto scl = distribution(generator) * 0.4f + 1.8f;
																	auto quat = angleAxis(radians(distribution(generator) * 360.f), vec3(0.f, 1.f, 0.f));
																	auto pos = vec3(mix(pa.x, pb.x, x), 0.f, mix(pa.y, pb.y, x)) * ext;
																	pos.y = self_height * ext.y - obj.second.b.y;
																	x += obj.second.b.x * 1.5f * scl * (distribution(generator) * 2.f + 1.f) / ext.x / spawn_area_cx;

																	auto e_obj = obj.first->copy();
																	auto node = e_obj->get_component_i<cNode>(0);
																	node->set_pos(pos);
																	if (distribution(generator) > 0.5f)
																		quat = quat * angleAxis(radians(180.f), vec3(1.f, 0.f, 0.f));
																	node->set_qut(quat);
																	node->set_scl(vec3(scl));
																	e_cliff->add_child(e_obj);
																}

																//auto try_num = int(spawn_area_cx * ext.x * spawn_area_cy * ext.y) / 10;
																//std::vector<AABB> local_objs;
																//for (auto t = 0; t < try_num; t++)
																//{
																//	auto idx = int(distribution(generator) * e_rocks.size());
																//	auto coord = vec2(distribution(generator), distribution(generator));
																//	auto scl = distribution(generator) * 0.2f + 0.9f;
																//	auto quat = angleAxis(radians(distribution(generator) * 90.f), vec3(0.f, 1.f, 0.f));
																//	auto bounds = e_rocks[idx].second;
																//	bounds.a *= scl / ext; bounds.b *= scl / ext;
																//	bounds = AABB(bounds.get_points(mat3(quat)));
																//	bounds.a.x += coord.x * spawn_area_cx;
																//	bounds.a.z += coord.y * spawn_area_cy;
																//	bounds.b.x += coord.x * spawn_area_cx;
																//	bounds.b.z += coord.y * spawn_area_cy;
																//	if (bounds.a.x > 0.f && bounds.b.x < spawn_area_cx &&
																//		bounds.a.z > 0.f && bounds.b.z < spawn_area_cy)
																//	{
																//		auto ok = true;
																//		for (auto& oth : local_objs)
																//		{
																//			if (oth.intersects(bounds))
																//			{
																//				ok = false;
																//				break;
																//			}
																//		}
																//		if (!ok)
																//			continue;

																//		auto dir = normalize(pa - pb);
																//		auto pos = vec3(mix(pa.x, pb.x, coord.x),
																//			mix(self_height, oth_height, coord.y),
																//			mix(pa.y, pb.y, coord.x)) + vec3(-dir.y, 0.f, dir.x) * coord.y * spawn_area_width;
																//		pos *= ext;
																//		quat = angleAxis(spawn_area_slope, normalize(vec3(pa.x, 0.f, pa.y) - vec3(pb.x, 0.f, pb.y))) * quat;

																//		auto e_rock = e_rocks[idx].first->copy();
																//		auto node = e_rock->get_component_i<cNode>(0);
																//		node->set_pos(pos);
																//		node->set_qut(quat);
																//		node->set_scl(vec3(scl));
																//		e_cliff->add_child(e_rock);

																//		local_objs.push_back(bounds);
																//	}
																//}

															}
														}
													}
												}
											}
										}
									}
								}
								if (update_splash)
								{
									graphics::InstanceCommandBuffer cb(nullptr);
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
									cb.excute();

									splash_map->save(splash_map->filename);
									auto asset = AssetManagemant::find(Path::get(splash_map->filename));
									if (asset)
										asset->active = false;
								}
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
				app.open_message_dialog("[RestructurePrefabInstanceWarnning]", "");
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
					image->save(path);
					auto asset = AssetManagemant::find(path);
					if (asset)
						asset->active = false;
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
					material->save(path);
					auto asset = AssetManagemant::find(path);
					if (asset)
						asset->active = false;
				}
			}
		}
		else if (ext == L".fani")
		{
			if (selection.frame != last_sel_ref_frame)
			{
				auto animation = graphics::Animation::get(path);
				if (animation)
				{
					last_sel_ref_type = th<graphics::Animation>();
					last_sel_ref_obj = animation;
				}

				last_sel_ref_frame = selection.frame;
			}

			if (last_sel_ref_obj)
			{
				auto animation = (graphics::AnimationPtr)last_sel_ref_obj;
				ImGui::Text("Duration: %f", animation->duration);
				if (ImGui::TreeNode(std::format("Channels ({})", (int)animation->channels.size()).c_str()))
				{
					ImGui::BeginChild("channels", ImVec2(0, 0), true);
					for (auto& ch : animation->channels)
					{
						if (ImGui::TreeNode(ch.node_name.c_str()))
						{
							ImGui::TextUnformatted("  pos:");
							for (auto& k : ch.position_keys)
								ImGui::Text("    %f: %s", k.t, str(k.p).c_str());
							ImGui::TextUnformatted("  qut:");
							for (auto& k : ch.rotation_keys)
								ImGui::Text("    %f: %s", k.t, str((vec4&)k.q).c_str());
							ImGui::TreePop();
						}
					}
					ImGui::EndChild();
					ImGui::TreePop();
				}
			}
		}
	}
		break;
	}
}
