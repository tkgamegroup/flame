#include "procedure_terrain_private.h"
#include "terrain_private.h"
#include "nav_obstacle_private.h"
#include "../entity_private.h"
#include "../../foundation/typeinfo_serialize.h"
#include "../../graphics/image.h"
#include "../../graphics/renderpass.h"
#include "../../graphics/extension.h"

#ifdef USE_FORTUNE_ALGORITHM
#include <FortuneAlgorithm/FortuneAlgorithm.h>
#endif

namespace flame
{
	cProcedureTerrainPrivate::~cProcedureTerrainPrivate()
	{
		terrain->data_listeners.remove("procedure_terrain"_h);

		delete height_map;
		delete splash_map;
	}


	void cProcedureTerrainPrivate::set_image_size(const uvec2& size)
	{
		if (image_size == size)
			return;
		image_size = size;

		build_terrain();
		terrain->node->mark_transform_dirty();
		data_changed("image_size"_h);
	}

	void cProcedureTerrainPrivate::set_voronoi_sites_count(uint count)
	{

	}

	void cProcedureTerrainPrivate::set_voronoi_layer1_precentage(uint precentage)
	{

	}

	void cProcedureTerrainPrivate::set_voronoi_layer2_precentage(uint precentage)
	{

	}

	void cProcedureTerrainPrivate::set_voronoi_layer3_precentage(uint precentage)
	{

	}

	void cProcedureTerrainPrivate::set_splash_layers(uint layers)
	{

	}

	void cProcedureTerrainPrivate::set_splash_bar1(float bar)
	{

	}

	void cProcedureTerrainPrivate::set_splash_bar2(float bar)
	{

	}

	void cProcedureTerrainPrivate::set_splash_bar3(float bar)
	{

	}

	void cProcedureTerrainPrivate::set_splash_transition(float transition)
	{

	}

	void cProcedureTerrainPrivate::set_spawn_settings(const std::vector<SpawnSetting>& settings)
	{

	}

	void cProcedureTerrainPrivate::build_terrain()
	{
#ifdef USE_FORTUNE_ALGORITHM
		graphics::Queue::get()->wait_idle();

		delete height_map;
		delete splash_map;

		height_map = graphics::Image::create(graphics::Format_R8_UNORM, uvec3(image_size, 1), graphics::ImageUsageSampled | graphics::ImageUsageTransferDst);
		splash_map = graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec3(image_size, 1), graphics::ImageUsageSampled | graphics::ImageUsageTransferDst);

		auto extent = terrain->extent;

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

		std::vector<vec3> site_positions;
		site_positions.resize(site_postions_xz.size());
		for (auto i = 0; i < site_positions.size(); i++)
		{
			auto& p = site_postions_xz[i];
			site_positions[i] = vec3(p.x, 0.f, p.y);
		}

		for (auto i = 0; i < site_positions.size(); i++)
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
			site_positions[i].y = value;
		}
		for (auto i = 0; i < site_positions.size(); i++)
		{
			auto self_height = site_positions[i].y;
			auto max_height = 0.f;
			for (auto edge : get_site_edges(diagram.getSite(i)))
			{
				if (auto oth_edge = edge->twin; oth_edge)
				{
					auto oth_idx = oth_edge->incidentFace->site->index;
					auto oth_height = site_positions[oth_idx].y;
					if (oth_height < self_height)
						max_height = max(max_height, oth_height);
				}
			}
			site_positions[i].y = min(site_positions[i].y + 0.25f, site_positions[i].y);
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

		graphics::PipelineResourceManager prm;
		prm.init(pl->layout, graphics::PipelineGraphics);

		graphics::VertexBuffer buf_vtx;
		buf_vtx.create(pl->vi_ui(), MaxVertices);

		std::vector<std::pair<uint, uint>> height_draws;

		for (auto i = 0; i < site_positions.size(); i++)
		{
			auto vertices = get_site_vertices(diagram.getSite(i));
			auto vtx_off = buf_vtx.stag_top;
			if (!vertices.empty() && vtx_off + vertices.size() <= MaxVertices)
			{
				for (auto j = 0; j < vertices.size(); j++)
				{
					auto pv = buf_vtx.add();
					pv.item("i_pos"_h).set(vertices[j] * 2.f - 1.f);
					pv.item("i_uv"_h).set(vertices[j]);
					pv.item("i_val_base"_h).set(site_positions[i].y);
				}

				height_draws.emplace_back((uint)vertices.size(), vtx_off);
			}
		}

		auto slope_width = 8.f / extent.x;
		auto slope_length = 6.f / extent.x;

		std::vector<bool> site_seen(site_positions.size(), false);
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
					auto oth_height = site_positions[oth_idx].y;
					if (oth_height < height && height - oth_height < 0.3f)
						region.push_back(edge);
					else if (oth_height == height)
						form_region(oth_idx, region_idx, height);
				}
			}
		};
		for (auto i = 0; i < site_positions.size(); i++)
		{
			if (!site_seen[i])
			{
				regions.emplace_back();
				form_region(i, regions.size() - 1, site_positions[i].y);
			}
		}
		std::vector<std::pair<uint, uint>> slopes;
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

						auto src_id = (uint)edge->incidentFace->site->index;
						auto dst_id = (uint)edge->twin->incidentFace->site->index;
						slopes.emplace_back(src_id, dst_id);
						auto hi_height = site_positions[src_id].y;
						auto lo_height = site_positions[dst_id].y;

						auto vtx_off = buf_vtx.stag_top;
						if (vtx_off + 4 <= MaxVertices)
						{
							auto pv = buf_vtx.add();
							pv.item("i_pos"_h).set(pa * 2.f - 1.f);
							pv.item("i_uv"_h).set(pa);
							pv.item("i_val_base"_h).set(hi_height);

							pv = buf_vtx.add();
							pv.item("i_pos"_h).set(pb * 2.f - 1.f);
							pv.item("i_uv"_h).set(pb);
							pv.item("i_val_base"_h).set(hi_height);

							pv = buf_vtx.add();
							pv.item("i_pos"_h).set(pc * 2.f - 1.f);
							pv.item("i_uv"_h).set(pc);
							pv.item("i_val_base"_h).set(lo_height);

							pv = buf_vtx.add();
							pv.item("i_pos"_h).set(pd * 2.f - 1.f);
							pv.item("i_uv"_h).set(pd);
							pv.item("i_val_base"_h).set(lo_height);

							height_draws.emplace_back(4, vtx_off);
						}
					}
					r.erase(r.begin() + idx);
					off += rand() % max(1, (int)r.size() / n) + 1;
					n--;
				}
			}
		}

		buf_vtx.upload(cb.get());

		cb->image_barrier(height_map, {}, graphics::ImageLayoutAttachment);
		cb->set_viewport_and_scissor(Rect(vec2(0.f), vec2(height_map->extent)));
		cb->begin_renderpass(nullptr, fb, { vec4(0.f) });
		cb->bind_pipeline(pl);
		prm.pc.item_d("uv_off"_h).set(vec2(19.7f, 43.3f));
		prm.pc.item_d("uv_scl"_h).set(32.f);
		prm.pc.item_d("val_base"_h).set(0.f);
		prm.pc.item_d("val_scl"_h).set(0.125f);
		prm.pc.item_d("falloff"_h).set(0.f);
		prm.pc.item_d("power"_h).set(1.f);
		prm.push_constant(cb.get());
		cb->bind_vertex_buffer(buf_vtx.buf.get(), 0);
		for (auto& d : height_draws)
			cb->draw(d.first, 1, d.second, 0);
		cb->end_renderpass();
		cb->image_barrier(height_map, {}, graphics::ImageLayoutShaderReadOnly);
		cb.excute();

		height_map->clear_staging_data();
		terrain->update_normal_map();

		auto height_map_info_fn = height_map->filename;
		height_map_info_fn += L".info";
		std::ofstream height_map_info(Path::get(height_map_info_fn));
		height_map_info << "sites:" << std::endl;
		serialize_text(&site_positions, height_map_info);
		height_map_info.close();

		{
			graphics::InstanceCommandBuffer cb(nullptr);
			auto fb = splash_map->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear);
			auto pl = graphics::GraphicsPipeline::get(L"flame\\shaders\\terrain\\splash_by_normal.pipeline",
				{ "rp=" + str(fb->renderpass),
				  "frag:LAYERS=" + str(splash_layers) });
			graphics::PipelineResourceManager prm;
			prm.init(pl->layout, graphics::PipelineGraphics);

			cb->image_barrier(splash_map, {}, graphics::ImageLayoutAttachment);
			cb->set_viewport_and_scissor(Rect(vec2(0.f), vec2(splash_map->extent)));
			cb->begin_renderpass(nullptr, fb, { vec4(0.f) });
			cb->bind_pipeline(pl);
			prm.pc.item_d("bar"_h).set(vec4(splash_bar1, splash_bar2, splash_bar3, 0.f));
			prm.pc.item_d("transition"_h).set(splash_transition);
			prm.push_constant(cb.get());
			cb->bind_descriptor_set(0, terrain->normal_map->get_shader_read_src());
			cb->draw(3, 1, 0, 0);
			cb->end_renderpass();
			cb->image_barrier(splash_map, {}, graphics::ImageLayoutShaderReadOnly);
			cb.excute();
		}

		{
			auto sp_nearest = graphics::Sampler::get(graphics::FilterNearest, graphics::FilterNearest, false, graphics::AddressClampToEdge);
			graphics::InstanceCommandBuffer cb(nullptr);
			std::unique_ptr<graphics::Image> temp_splash(graphics::Image::create(splash_map->format, splash_map->extent, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
			auto fb = temp_splash->get_shader_write_dst(0, 0, graphics::AttachmentLoadDontCare);
			auto pl = graphics::GraphicsPipeline::get(L"flame\\shaders\\terrain\\splash_by_region.pipeline",
				{ "rp=" + str(fb->renderpass) });
			graphics::PipelineResourceManager prm;
			prm.init(pl->layout, graphics::PipelineGraphics);

			graphics::StorageBuffer buf_sdf;
			auto dsl = prm.pll->dsls[0];
			buf_sdf.create(graphics::BufferUsageStorage, dsl->get_buf_ui("SDF"_h));
			std::unique_ptr<graphics::DescriptorSet> ds(graphics::DescriptorSet::create(nullptr, dsl));
			ds->set_buffer("SDF"_h, 0, buf_sdf.buf.get());
			ds->set_image("img_src"_h, 0, splash_map->get_view(), sp_nearest);
			ds->update();

			auto ext_xz = extent.xz();
			for (auto i = 0; i < site_positions.size(); i++)
			{
				auto pi = buf_sdf.item_d("circles"_h, i);
				pi.item("coord"_h).set(site_positions[i].xz() * ext_xz);
				pi.item("radius"_h).set(5.f);
			}
			buf_sdf.item_d("circles_count"_h).set((int)site_positions.size());
			auto n_ori_rects = 0;
			for (auto i = 0; i < diagram.getNbSites(); i++)
			{
				auto self_height = site_positions[i].y;
				auto edges = get_site_edges(diagram.getSite(i));
				for (auto edge : edges)
				{
					if (auto oth_edge = edge->twin; oth_edge)
					{
						auto oth_idx = oth_edge->incidentFace->site->index;
						auto oth_height = site_positions[oth_idx].y;
						auto ok = false;
						if (self_height == oth_height)
							ok = true;
						if (!ok)
						{
							for (auto& s : slopes)
							{
								if ((s.first == i && s.second == oth_idx) ||
									(s.second == i && s.first == oth_idx))
								{
									ok = true;
									break;
								}
							}
						}
						if (ok)
						{
							auto pi = buf_sdf.item_d("ori_rects"_h, n_ori_rects);
							pi.item("point_a"_h).set(site_positions[i].xz() * ext_xz);
							pi.item("point_b"_h).set((to_glm(edge->origin->point) + to_glm(edge->destination->point)) * 0.5f * ext_xz);
							pi.item("thickness"_h).set(2.f);
							n_ori_rects++;
						}
					}
				}
			}
			buf_sdf.item_d("ori_rects_count"_h).set(n_ori_rects);
			buf_sdf.upload(cb.get());

			cb->image_barrier(splash_map, {}, graphics::ImageLayoutShaderReadOnly);
			cb->set_viewport_and_scissor(Rect(vec2(0.f), vec2(splash_map->extent)));
			cb->begin_renderpass(nullptr, fb);
			cb->bind_pipeline(pl);
			prm.pc.item_d("screen_size"_h).set(ext_xz);
			prm.pc.item_d("channel"_h).set(2U);
			prm.pc.item_d("distance"_h).set(1.f);
			prm.pc.item_d("merge_k"_h).set(0.2f);
			prm.push_constant(cb.get());
			prm.set_ds(""_h, ds.get());
			prm.bind_dss(cb.get());
			cb->draw(3, 1, 0, 0);
			cb->end_renderpass();
			cb->image_barrier(splash_map, {}, graphics::ImageLayoutAttachment);
			cb->image_barrier(temp_splash.get(), {}, graphics::ImageLayoutShaderReadOnly);
			cb->begin_renderpass(nullptr, splash_map->get_shader_write_dst());
			cb->bind_pipeline(graphics::GraphicsPipeline::get(L"flame\\shaders\\blit.pipeline", { "rp=" + str(fb->renderpass) }));
			cb->bind_descriptor_set(0, temp_splash->get_shader_read_src(0, 0, sp_nearest));
			cb->draw(3, 1, 0, 0);
			cb->end_renderpass();
			cb->image_barrier(splash_map, {}, graphics::ImageLayoutShaderReadOnly);
			cb.excute();
		}

		terrain->set_height_map_name(L"0x" + wstr_hex((uint64)height_map));
		terrain->set_splash_map_name(L"0x" + wstr_hex((uint64)splash_map));

		std::vector<std::pair<EntityPtr, cNavObstaclePtr>> spawn_prefabs;
		for (auto& setting : spawn_settings)
		{
			auto e = Entity::create();
			e->load(setting.prefab_name);
			new PrefabInstance(e, setting.prefab_name);
			spawn_prefabs.emplace_back(e, e->get_component_t<cNavObstacleT>());
		}
		auto e_dst_parent = terrain->entity->parent;
		auto node_name = "terrain_auto_spawn";
		auto e_dst = e_dst_parent->find_child(node_name);
		if (!e_dst)
		{
			e_dst = Entity::create();
			e_dst->name = node_name;
			e_dst->add_component_t<cNode>();
			e_dst_parent->add_child(e_dst);
		}
		e_dst->remove_all_children();
		std::vector<vec3> spawned_objects;
		auto spawn_offset = terrain->node->pos;
		for (auto i = 0; i < spawn_settings.size(); i++)
		{
			auto& setting = spawn_settings[i];
			auto& prefab = spawn_prefabs[i];
			if (!prefab.second)
				continue;
			auto r = prefab.second->radius;
			auto r_uv = r / extent.x;
			for (auto i = 0; i < setting.count; i++)
			{
				auto p = vec2(linearRand(0.f, 1.f), linearRand(0.f, 1.f));
				if (splash_map->linear_sample(p)[setting.channel] < 0.5f)
					continue;
				if (splash_map->linear_sample(p + vec2(+r_uv, 0.f))[setting.channel] < 0.5f)
					continue;
				if (splash_map->linear_sample(p + vec2(-r_uv, 0.f))[setting.channel] < 0.5f)
					continue;
				if (splash_map->linear_sample(p + vec2(0.f, +r_uv))[setting.channel] < 0.5f)
					continue;
				if (splash_map->linear_sample(p + vec2(0.f, -r_uv))[setting.channel] < 0.5f)
					continue;
				auto ok = true;
				for (auto obj : spawned_objects)
				{
					if (distance(obj.xy(), p) < obj.z + r_uv)
					{
						ok = false;
						break;
					}
				}
				if (ok)
				{
					auto e = prefab.first->copy();
					new PrefabInstance(e, prefab.first->prefab_instance->filename);
					e->file_id = prefab.first->file_id;
					auto n = e->node();
					n->set_pos(spawn_offset + vec3(p.x * extent.x, height_map->linear_sample(p).r * extent.y, p.y * extent.z));
					n->set_eul(vec3(linearRand(0.f, 360.f), 0.f, 0.f));
					e->prefab_instance->mark_modifier(prefab.first->file_id, "flame::cNode", "pos");
					e->prefab_instance->mark_modifier(prefab.first->file_id, "flame::cNode", "eul");
					e_dst->add_child(e);
					spawned_objects.push_back(vec3(p, r_uv));
				}
			}
		}
		for (auto& c : spawn_prefabs)
			delete c.first;
#endif
	}

	void cProcedureTerrainPrivate::on_init()
	{
		build_terrain();

		terrain->data_listeners.add([this](uint hash) {
			if (hash == "extent"_h || hash == "blocks"_h)
			build_terrain();
		}, "procedure_terrain"_h);
	}

	struct cProcedureTerrainCreate : cProcedureTerrain::Create
	{
		cProcedureTerrainPtr operator()(EntityPtr e) override
		{
			return new cProcedureTerrainPrivate();
		}
	}cProcedureTerrain_create;
	cProcedureTerrain::Create& cProcedureTerrain::create = cProcedureTerrain_create;
}
