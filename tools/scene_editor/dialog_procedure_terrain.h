#pragma once

#include <flame/foundation/typeinfo_serialize.h>
#include <flame/graphics/extension.h>
#include <flame/graphics/material.h>
#include <flame/graphics/model.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/debug.h>
#include <flame/universe/components/node.h>
#include <flame/universe/components/mesh.h>
#include <flame/universe/components/terrain.h>

#include <FortuneAlgorithm/FortuneAlgorithm.h>

using namespace flame;

struct ProcedureTerrainDialog : ImGui::Dialog
{
	cTerrainPtr terrain;
	std::vector<vec3> site_postions;
	bool update_height = true;
	int voronoi_sites_count = 20;
	int voronoi_layer1_precentage = 50;
	int voronoi_layer2_precentage = 50;
	int voronoi_layer3_precentage = 50;
	bool update_cliff = true;
	std::vector<std::filesystem::path> cliff_models;

	bool update_splash = true;
	int splash_layers = 2;
	float splash_bar1 = 45.f;
	float splash_bar2 = 80.f;
	float splash_bar3 = 90.f;
	float splash_transition = 4.f;

	static void open(cTerrainPtr terrain)
	{
		auto dialog = new ProcedureTerrainDialog;
		dialog->title = "Procedure Terrain";
		dialog->terrain = terrain;
		Dialog::open(dialog);
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
				ImGui::InputInt("Layers", &splash_layers);
				splash_layers = clamp(splash_layers, 1, 4);
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

			std::vector<Vector2> site_postions_xz;
			for (int i = 0; i < voronoi_sites_count; ++i)
				site_postions_xz.push_back(Vector2{ distribution(generator), distribution(generator) });

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

			auto diagram = get_diagram(site_postions_xz);
			for (auto t = 0; t < 3; t++)
			{
				for (auto i = 0; i < diagram.getNbSites(); i++)
				{
					auto vertices = get_site_vertices(diagram.getSite(i));
					auto centroid = convex_centroid(vertices);
					site_postions_xz[i] = Vector2(centroid.x, centroid.y);
				}
				diagram = get_diagram(site_postions_xz);
			}

			site_postions.resize(site_postions_xz.size());
			for (auto i = 0; i < site_postions.size(); i++)
			{
				auto& p = site_postions_xz[i];
				site_postions[i] = vec3(p.x, 0.f, p.y);
			}

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
				site_postions[i].y = value;
			}
			for (auto i = 0; i < site_postions.size(); i++)
			{
				auto self_height = site_postions[i].y;
				auto max_height = 0.f;
				for (auto edge : get_site_edges(diagram.getSite(i)))
				{
					if (auto oth_edge = edge->twin; oth_edge)
					{
						auto oth_idx = oth_edge->incidentFace->site->index;
						auto height = site_postions[oth_idx].y;
						if (height < self_height)
							max_height = max(max_height, height);
					}
				}
				site_postions[i].y = min(site_postions[i].y + 0.25f, site_postions[i].y);
			}

			const auto MaxVertices = 10000;

			graphics::InstanceCommandBuffer cb;
			auto fb = height_map->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear);
			auto pt = graphics::PrimitiveTopologyTriangleFan;
			auto pl = graphics::GraphicsPipeline::get(L"flame\\shaders\\noise\\fbm.pipeline",
				{ "vs=fbm.vert",
				  "all_shader:USE_VERTEX_VAL_BASE",
				  "rp=" + str(fb->renderpass),
				  "pt=" + TypeInfo::serialize_t(pt) });

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
						buf_vtx.set_var<"i_val_base"_h>(site_postions[i].y);
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
							auto oth_height = site_postions[oth_idx].y;
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
						form_region(i, regions.size() - 1, site_postions[i].y);
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

								auto hi_height = site_postions[edge->incidentFace->site->index].y;
								auto lo_height = site_postions[edge->twin->incidentFace->site->index].y;

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

			auto height_map_info_fn = height_map->filename;
			height_map_info_fn += L".info";
			std::ofstream height_map_info(Path::get(height_map_info_fn));
			height_map_info << "sites:" << std::endl;
			serialize_text(&site_postions, height_map_info);
			height_map_info.close();

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
						auto self_height = site_postions[i].y;
						if (self_height > 0.f)
						{
							auto site = diagram.getSite(i);
							auto edges = get_site_edges(site);
							for (auto edge : edges)
							{
								if (auto oth_edge = edge->twin; oth_edge)
								{
									auto oth_height = site_postions[oth_edge->incidentFace->site->index].y;
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
			{
				graphics::InstanceCommandBuffer cb(nullptr);
				auto fb = splash_map->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear);
				auto pl = graphics::GraphicsPipeline::get(L"flame\\shaders\\terrain\\splash_by_normal.pipeline",
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
			}

			{
				graphics::InstanceCommandBuffer cb(nullptr);
				std::unique_ptr<graphics::Image> temp_splash(graphics::Image::create(splash_map->format, splash_map->size, graphics::ImageUsageAttachment | graphics::ImageUsageTransferSrc));
				auto fb = temp_splash->get_shader_write_dst(0, 0, graphics::AttachmentLoadDontCare);
				auto pl = graphics::GraphicsPipeline::get(L"flame\\shaders\\terrain\\splash_by_region.pipeline",
					{ "rp=" + str(fb->renderpass) });
				graphics::PipelineResourceManager<FLAME_UID> prm;
				prm.init(pl->layout);

				graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage> buf_sd_circles;
				graphics::StorageBuffer<FLAME_UID, graphics::BufferUsageStorage> buf_sd_ori_rects;
				auto dsl = prm.pll->dsls[0];
				buf_sd_circles.create_with_array_type(dsl->get_buf_ui("SdCircles"_h));
				buf_sd_ori_rects.create_with_array_type(dsl->get_buf_ui("SdOriRects"_h));
				std::unique_ptr<graphics::DescriptorSet> ds(graphics::DescriptorSet::create(nullptr, dsl));
				ds->set_buffer("SdCircles"_h, 0, buf_sd_circles.buf.get());
				ds->set_buffer("SdOriRects"_h, 0, buf_sd_ori_rects.buf.get());
				ds->set_image("img_src"_h, 0, splash_map->get_view(), nullptr);

				for (auto& pos : site_postions)
				{
					buf_sd_circles.set_var<"coord"_h>(pos.xz());
					buf_sd_circles.set_var<"radius"_h>(5.f);
					buf_sd_circles.next_item();
				}
				buf_sd_circles.upload(cb.get());

				cb->image_barrier(splash_map, {}, graphics::ImageLayoutShaderReadOnly);
				cb->set_viewport_and_scissor(Rect(vec2(0.f), vec2(splash_map->size)));
				cb->begin_renderpass(nullptr, fb);
				cb->bind_pipeline(pl);
				prm.set_pc_var<"screen_size"_h>(terrain->extent.xz());
				prm.set_pc_var<"channel"_h>(2U);
				prm.set_pc_var<"distance"_h>(1.f);
				prm.set_pc_var<"merge_k"_h>(0.2f);
				prm.set_pc_var<"sd_circles_count"_h>((uint)site_postions.size());
				prm.set_pc_var<"sd_ori_rect_count"_h>((uint)0);
				prm.push_constant(cb.get());
				prm.set_ds(""_h, ds.get());
				prm.bind_dss(cb.get());
				cb->draw(3, 1, 0, 0);
				cb->end_renderpass();
				cb.excute();
			}

			splash_map->save(splash_map->filename);
			auto asset = AssetManagemant::find(Path::get(splash_map->filename));
			if (asset)
				asset->active = false;
		}
	}
};
