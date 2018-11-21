#include <flame/engine/core/core.h>
#include <flame/engine/graphics/synchronization.h>
#include <flame/engine/graphics/buffer.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/graphics/material.h>
#include <flame/engine/graphics/renderpass.h>
#include <flame/engine/graphics/framebuffer.h>
#include <flame/engine/graphics/descriptor.h>
#include <flame/engine/graphics/pipeline.h>
#include <flame/engine/graphics/sampler.h>
#include <flame/engine/graphics/command_buffer.h>
#include <flame/engine/graphics/renderer.h>
#include <flame/engine/entity/node.h>
#include <flame/engine/entity/camera.h>
#include <flame/engine/entity/light.h>
#include <flame/engine/entity/model_instance.h>
#include <flame/engine/entity/terrain.h>
#include <flame/engine/entity/water.h>
#include <flame/engine/entity/model.h>
#include <flame/engine/entity/animation.h>
#include <flame/engine/entity/scene.h>

namespace flame
{
	void PlainRenderer::DrawData::ObjData::fill_with_model(Model *m)
	{
		geo_data.resize(1);
		geo_data[0].index_count = m->indices.size();
		geo_data[0].first_index = m->indice_base;
		geo_data[0].vertex_offset = m->vertex_base;
		geo_data[0].instance_count = 1;
		geo_data[0].first_instance = 0;
	}

	void PlainRenderer::DrawData::ObjData::fill_with_model_texture_mode(Model *m)
	{
		for (int i = 0; i < m->geometries.size(); i++)
		{
			auto &g = m->geometries[i];
			if (g->material->get_albedo_alpha_map())
			{
				GeoData data;
				auto &g = m->geometries[i];
				data.index_count = g->indiceCount;
				data.first_index = m->indice_base + g->indiceBase;
				data.vertex_offset = m->vertex_base;
				data.instance_count = 1;
				data.first_instance = g->material->get_albedo_alpha_map()->material_index;
				geo_data.push_back(data);
			}
		}
	}

	static Pipeline *pipeline_plain;
	static Pipeline *pipeline_plain_anim;
	static Pipeline *pipeline_frontlight;
	static Pipeline *pipeline_material;
	static Pipeline *pipeline_material_anim;
	static Pipeline *pipeline_wireframe;
	static Pipeline *pipeline_wireframe_anim;
	bool PlainRenderer::first = true;
	Buffer *PlainRenderer::last_bone_buffer_mode0;
	Buffer *PlainRenderer::last_bone_buffer_mode2;
	Buffer *PlainRenderer::last_bone_buffer_mode3;
	std::shared_ptr<RenderPass> PlainRenderer::renderpass_color;
	std::shared_ptr<RenderPass> PlainRenderer::renderpass_color_and_depth;
	std::shared_ptr<RenderPass> PlainRenderer::renderpass_color_clear;
	std::shared_ptr<RenderPass> PlainRenderer::renderpass_color_clear_and_depth;
	PlainRenderer::PlainRenderer()
	{
		if (first)
		{
			renderpass_color = get_renderpass(RenderPassInfo()
				.add_attachment(VK_FORMAT_R8G8B8A8_UNORM, false)
				.add_subpass({0}, -1)
			);
			renderpass_color_and_depth = get_renderpass(RenderPassInfo()
				.add_attachment(VK_FORMAT_R8G8B8A8_UNORM, false)
				.add_attachment(VK_FORMAT_D16_UNORM, true)
				.add_subpass({0}, 1)
			);
			renderpass_color_clear = get_renderpass(RenderPassInfo()
				.add_attachment(VK_FORMAT_R8G8B8A8_UNORM, true)
				.add_subpass({0}, -1)
			);
			renderpass_color_clear_and_depth = get_renderpass(RenderPassInfo()
				.add_attachment(VK_FORMAT_R8G8B8A8_UNORM, true)
				.add_attachment(VK_FORMAT_D16_UNORM, true)
				.add_subpass({0}, 1)
			);

			pipeline_plain = new Pipeline(PipelineInfo()
				.set_vertex_input_state({ { TokenF32V3, 0 }, { TokenF32V2, 0 }, { TokenF32V3, 0 }, { TokenF32V3, 0 } })
				.set_depth_test(true)
				.set_depth_write(true)
				.add_shader("plain3d/plain3d.vert", {})
				.add_shader("plain3d/plain3d.frag", {}),
				renderpass_color_and_depth.get(), 0);
			pipeline_plain_anim = new Pipeline(PipelineInfo()
				.set_vertex_input_state({ { TokenF32V3, 0 }, { TokenF32V2, 0 }, { TokenF32V3, 0 }, { TokenF32V3, 0 }, { TokenF32V4, 1 }, { TokenF32V4, 1 } })
				.set_depth_test(true)
				.set_depth_write(true)
				.add_shader("plain3d/plain3d.vert", { "ANIM" })
				.add_shader("plain3d/plain3d.frag", { "ANIM" }),
				renderpass_color_and_depth.get(), 0, true);
			pipeline_frontlight = new Pipeline(PipelineInfo()
				.set_vertex_input_state({ { TokenF32V3, 0 }, { TokenF32V2, 0 }, { TokenF32V3, 0 }, { TokenF32V3, 0 } })
				.set_depth_test(true)
				.set_depth_write(true)
				.add_shader("plain3d/plain3d.vert", { "USE_NORMAL" })
				.add_shader("plain3d/plain3d.frag", { "USE_NORMAL" }),
				renderpass_color_and_depth.get(), 0);
			pipeline_material = new Pipeline(PipelineInfo()
				.set_vertex_input_state({ { TokenF32V3, 0 }, { TokenF32V2, 0 }, { TokenF32V3, 0 }, { TokenF32V3, 0 } })
				.set_depth_test(true)
				.set_depth_write(true)
				.add_shader("plain3d/plain3d.vert", { "USE_MATERIAL" })
				.add_shader("plain3d/plain3d.frag", { "USE_MATERIAL" }),
				renderpass_color_and_depth.get(), 0);
			pipeline_material_anim = new Pipeline(PipelineInfo()
				.set_vertex_input_state({ { TokenF32V3, 0 }, { TokenF32V2, 0 }, { TokenF32V3, 0 }, { TokenF32V3, 0 }, { TokenF32V4, 1 }, { TokenF32V4, 1 } })
				.set_depth_test(true)
				.set_depth_write(true)
				.add_shader("plain3d/plain3d.vert", { "ANIM", "USE_MATERIAL" })
				.add_shader("plain3d/plain3d.frag", { "ANIM", "USE_MATERIAL" }),
				renderpass_color_and_depth.get(), 0, true);
			pipeline_wireframe = new Pipeline(PipelineInfo()
				.set_vertex_input_state({ { TokenF32V3, 0 }, { TokenF32V2, 0 }, { TokenF32V3, 0 }, { TokenF32V3, 0 } })
				.set_polygon_mode(VK_POLYGON_MODE_LINE)
				.set_cull_mode(VK_CULL_MODE_NONE)
				.add_shader("plain3d/plain3d.vert", {})
				.add_shader("plain3d/plain3d.frag", {}),
				renderpass_color.get(), 0);
			pipeline_wireframe_anim = new Pipeline(PipelineInfo()
				.set_vertex_input_state({ { TokenF32V3, 0 }, { TokenF32V2, 0 }, { TokenF32V3, 0 }, { TokenF32V3, 0 }, { TokenF32V4, 1 }, { TokenF32V4, 1 } })
				.set_polygon_mode(VK_POLYGON_MODE_LINE)
				.set_cull_mode(VK_CULL_MODE_NONE)
				.add_shader("plain3d/plain3d.vert", { "ANIM" })
				.add_shader("plain3d/plain3d.frag", { "ANIM" }),
				renderpass_color.get(), 0, true);

			first = false;
		}

		cb = std::make_unique<CommandBuffer>();
	}

	void PlainRenderer::render(Framebuffer *framebuffer, bool clear, CameraComponent *camera, DrawData *data)
	{
		cb->begin();
		RenderPass *rp;
		if (data->mode == mode_wireframe)
		{
			if (clear)
				rp = renderpass_color_clear.get();
			else
				rp = renderpass_color.get();
		}
		else
		{
			if (clear)
				rp = renderpass_color_clear_and_depth.get();
			else
				rp = renderpass_color_and_depth.get();
		}
		cb->begin_renderpass(rp, framebuffer);
		do_render(cb.get(), camera, data);
		cb->end_renderpass();
		cb->end();
	}

	void PlainRenderer::do_render(CommandBuffer *cb, CameraComponent *camera, DrawData *data)
	{
		cb->set_viewport_and_scissor(resolution.x(), resolution.y());

		if (data->vbuffer0)
		{
			if (data->vbuffer1)
				cb->bind_vertex_buffer2(data->vbuffer0, data->vbuffer1);
			else
				cb->bind_vertex_buffer(data->vbuffer0);
			cb->bind_index_buffer(data->ibuffer);
		}
		else
		{
			cb->bind_vertex_buffer2(vertex_static_buffer.get(), vertex_skeleton_Buffer.get());
			cb->bind_index_buffer(index_buffer.get());
		}

		struct
		{
			glm::mat4 modelview;
			glm::mat4 proj;
			glm::vec4 color;
		}pc;
		pc.proj = camera->get_proj_matrix();

		for (int i = 0; i < data->obj_data.size(); i++)
		{
			auto &d = data->obj_data[i];

			switch (data->mode)
			{
				case mode_just_color:
					if (!d.bone_buffer)
						cb->bind_pipeline(pipeline_plain);
					else
					{
						cb->bind_pipeline(pipeline_plain_anim);
						if (last_bone_buffer_mode0 != d.bone_buffer)
						{
							updateDescriptorSets(1, &pipeline_plain_anim->descriptor_set->get_write(0, 0, &get_buffer_info(d.bone_buffer)));
							last_bone_buffer_mode0 = d.bone_buffer;
						}
						cb->bind_descriptor_set();
					}
					break;
				case mode_color_and_front_light:
					cb->bind_pipeline(pipeline_frontlight);
					break;
				case mode_just_texture:
				{
					if (!d.bone_buffer)
					{
						cb->bind_pipeline(pipeline_material);
						cb->bind_descriptor_set(&ds_material->v, 1, 1);
					}
					else
					{
						cb->bind_pipeline(pipeline_material_anim);
						if (last_bone_buffer_mode2 != d.bone_buffer)
						{
							updateDescriptorSets(1, &pipeline_material_anim->descriptor_set->get_write(0, 0, &get_buffer_info(d.bone_buffer)));
							last_bone_buffer_mode2 = d.bone_buffer;
						}
						VkDescriptorSet sets[] = {
							pipeline_material->descriptor_set->v,
							ds_material->v
						};
						cb->bind_descriptor_set(sets, 0, TK_ARRAYSIZE(sets));
					}
					break;
				}
				case mode_wireframe:
					if (!d.bone_buffer)
						cb->bind_pipeline(pipeline_wireframe);
					else
					{
						cb->bind_pipeline(pipeline_wireframe_anim);
						if (last_bone_buffer_mode3 != d.bone_buffer)
						{
							updateDescriptorSets(1, &pipeline_wireframe_anim->descriptor_set->get_write(0, 0, &get_buffer_info(d.bone_buffer)));
							last_bone_buffer_mode3 = d.bone_buffer;
						}
						cb->bind_descriptor_set();
					}
					break;
			}

			pc.modelview = camera->get_view_matrix() * d.mat;
			pc.color = d.color;
			cb->push_constant(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
			for (auto &d : d.geo_data)
				cb->draw_index(d.index_count, d.first_index, d.vertex_offset, d.instance_count, d.first_instance);
		}
	}

	void PlainRenderer::add_to_drawlist()
	{
		add_to_draw_list(cb->v);
	}

	static Pipeline *pipeline_lines;
	bool LinesRenderer::first = true;
	std::shared_ptr<RenderPass> LinesRenderer::renderpass_color;
	LinesRenderer::LinesRenderer()
	{
		if (first)
		{
			renderpass_color = get_renderpass(RenderPassInfo()
				.add_attachment(VK_FORMAT_R8G8B8A8_UNORM, false)
				.add_subpass({0}, -1)
			);

			pipeline_lines = new Pipeline(PipelineInfo()
					.set_vertex_input_state({ { TokenF32V3, 0 }, { TokenF32V3, 0 } })
					.set_primitive_topology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
					.set_polygon_mode(VK_POLYGON_MODE_LINE)
					.set_cull_mode(VK_CULL_MODE_NONE)
					.add_shader("plain3d/plain3d_line.vert", {})
					.add_shader("plain3d/plain3d_line.frag", {}),
					renderpass_color.get(), 0);

			first = false;
		}

		cb = std::make_unique<CommandBuffer>();
	}

	void LinesRenderer::render(Framebuffer *framebuffer, bool clear, CameraComponent *camera, DrawData *data)
	{
		cb->begin();

		cb->begin_renderpass(renderpass_color.get(), framebuffer);

		cb->set_viewport_and_scissor(resolution.x(), resolution.y());

		cb->bind_vertex_buffer(data->vertex_buffer);
		cb->bind_pipeline(pipeline_lines);

		glm::mat4 mvp = camera->get_proj_matrix() * camera->get_view_matrix();
		cb->push_constant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &mvp);
		cb->draw(data->vertex_count);

		cb->end_renderpass();

		cb->end();
	}

	void LinesRenderer::add_to_drawlist()
	{
		add_to_draw_list(cb->v);
	}

	struct ConstantBufferStruct
	{
		float depth_near;
		float depth_far;
		float cx;
		float cy;
		float aspect;
		float fovy;
		float tanHfFovy;
		float envrCx;
		float envrCy;
	};

	long long constant_buffer_updated_frame = -1;

	struct MatrixBufferShaderStruct
	{
		glm::mat4 proj;
		glm::mat4 projInv;
		glm::mat4 view;
		glm::mat4 viewInv;
		glm::mat4 projView;
		glm::mat4 projViewRotate;
		glm::vec4 frustumPlanes[6];
		glm::vec2 viewportDim;
	};

	struct AmbientBufferShaderStruct
	{
		glm::vec3 color;
		glm::uint envr_max_mipmap;
		glm::vec4 fogcolor;
	};

	struct LightShaderStruct
	{
		glm::vec4 coord;    // xyz - coord(point/spot)/dir(parallax), w - the light type
		glm::vec4 color;    // rgb - color, a - shadow index(-1 is no shadow)
		glm::vec4 spotData; // xyz - spot direction, a - spot range
	};

	struct LightBufferShaderStruct
	{
		unsigned int count;
		unsigned int dummy0;
		unsigned int dummy1;
		unsigned int dummy2;

		LightShaderStruct lights[MaxLightCount];
	};

	struct TerrainShaderStruct
	{
		glm::vec3 coord;
		int block_cx;
		int block_cy;
		float block_size;
		float terrain_height;
		float displacement_height;
		float tessellation_factor;
		float tiling_scale;
		unsigned int material_count;
		union
		{
			struct
			{
				unsigned char x;
				unsigned char y;
				unsigned char z;
				unsigned char w;
			};
			unsigned char v[4];
			unsigned int packed;
		}material_index;
	};

	struct WaterShaderStruct
	{
		glm::vec3 coord;
		int block_cx;
		int block_cy;
		float block_size;
		float height;
		float tessellation_factor;
		float tiling_scale;
		float mapDimension;
		unsigned int dummy0;
		unsigned int dummy1;
	};

	VkPipelineVertexInputStateCreateInfo vertexStatInputState;
	VkPipelineVertexInputStateCreateInfo vertexAnimInputState;
	VkPipelineVertexInputStateCreateInfo terrian_vertex_input_state;

	static Pipeline *scattering_pipeline;
	static Pipeline *output_debug_panorama_pipeline;
	static Pipeline *downsample_pipeline;
	static Pipeline *convolve_pipeline;
	static Pipeline *copy_pipeline;
	static Pipeline *mrt_pipeline;
	static Pipeline *mrt_anim_pipeline;
	static Pipeline *terrain_pipeline;
	static Pipeline *water_pipeline;
	static Pipeline *deferred_pipeline;
	static Pipeline *compose_pipeline;
	static Pipeline *esm_pipeline;
	static Pipeline *esm_anim_pipeline;
	static Texture *envr_image_downsample[3] = {};
	bool DeferredRenderer::defe_inited = false;
	bool DeferredRenderer::shad_inited = false;
	std::shared_ptr<RenderPass> DeferredRenderer::renderpass_color;
	std::shared_ptr<RenderPass> DeferredRenderer::renderpass_color16;
	std::shared_ptr<RenderPass> DeferredRenderer::renderpass_color_and_depth;
	std::shared_ptr<RenderPass> DeferredRenderer::renderpass_color32_and_depth;
	std::shared_ptr<RenderPass> DeferredRenderer::renderpass_defe;

	bool DeferredRenderer::on_message(Object *sender, Message msg)
	{
		switch (msg)
		{
			case MessageResolutionChange:
				create_resolution_related();
				return true;
			case MessageSkyDirty:
				sky_dirty = true;
				return true;
			case MessageAmbientDirty:
				ambient_dirty = true;
				return true;
			case MessageComponentAdd:
			{
				auto c = (Component*)sender;
				switch (c->get_type())
				{
					case ComponentTypeLight:
					{
						auto l = (LightComponent*)c;
						auto index = lights.add(l);
						if (index != -2)
						{
							l->set_light_index(index);
							light_auxes[index].attribute_updated_frame = -1;
							light_auxes[index].shadow_updated_frame = -1;
							light_count_dirty = true;
							if (l->is_enable_shadow())
							{
								auto index = shadow_lights.add(l);
								if (index != -2)
									l->set_shadow_index(index);
							}
						}
						break;
					}
					case ComponentTypeModelInstance:
					{
						auto i = (ModelInstanceComponent*)c;
						auto index = i->get_model()->vertexes_skeleton.size() > 0 ? 
							animated_model_instances.add(i) :
							static_model_instances.add(i);
						if (index != -2)
						{
							i->set_instance_index(index);
							if (i->get_model()->vertexes_skeleton.size() > 0)
							{
								animated_model_instance_auxes[index].matrix_updated_frame = -1;
								animated_model_instance_count_dirty = true;
							}
							else
							{
								static_model_instance_auxes[index].matrix_updated_frame = -1;
								static_model_instance_count_dirty = true;
							}
						}
						break;
					}
					case ComponentTypeTerrain:
					{
						auto t = (TerrainComponent*)c;
						auto index = terrains.add(t);
						if (index != -2)
						{
							t->set_terrain_index(index);
							terrain_auxes[index].attribute_updated_frame = -1;
							terrain_auxes[index].blend_map_updated_frame = -1;
							terrain_count_dirty = true;
						}
						break;
					}
					case ComponentTypeWater:
					{
						auto w = (WaterComponent*)c;
						auto index = waters.add(w);
						if (index != -2)
						{
							w->set_water_index(index);
							water_auxes[index].attribute_updated_frame = -1;
							water_count_dirty = true;
						}
						break;
					}
				}
				return true;
			}
			case MessageComponentRemove:
			{
				auto c = (Component*)sender;
				switch (c->get_type())
				{
					case ComponentTypeLight:
					{
						auto l = (LightComponent*)c;
						auto index = l->get_light_index();
						if (index != -1)
						{
							lights.remove(l);
							light_count_dirty = true;
							if (l->is_enable_shadow() && l->get_shadow_index() != -1)
								shadow_lights.remove(l);
						}
						break;
					}
					case ComponentTypeModelInstance:
					{
						auto i = (ModelInstanceComponent*)c;
						auto index = i->get_instance_index();
						if (index != -1)
						{
							if (i->get_model()->vertexes_skeleton.size() > 0)
								animated_model_instances.remove(i);
							else
								static_model_instances.remove(i);
							if (i->get_model()->vertexes_skeleton.size() > 0)
								animated_model_instance_count_dirty = true;
							else
								static_model_instance_count_dirty = true;
						}
						break;
					}
					case ComponentTypeTerrain:
					{
						auto t = (TerrainComponent*)c;
						auto index = t->get_terrain_index();
						if (index != -1)
						{
							terrains.remove(t);
							terrain_count_dirty = true;
						}
						break;
					}
					case ComponentTypeWater:
					{
						auto w = (WaterComponent*)c;
						auto index = w->get_water_index();
						if (index != -1)
						{
							waters.remove(w);
							water_count_dirty = true;
						}
						break;
					}
				}
				return true;
			}
			case MessageToggleShaodw:
			{
				auto l = (LightComponent*)sender;
				if (l->is_enable_shadow())
				{
					auto index = shadow_lights.add(l);
					if (index != -2)
						l->set_shadow_index(index);
				}
				else
				{
					if (l->get_shadow_index() != -1)
						shadow_lights.remove(l);
				}
				return true;
			}
			case MessageChangeModel:
			{
				auto i = (ModelInstanceComponent*)sender;
				if (i->get_model()->vertexes_skeleton.size() > 0)
					animated_model_instance_count_dirty = true;
				else
					static_model_instance_count_dirty = true;
				return true;
			}
		}
		return false;
	}

	DeferredRenderer::DeferredRenderer(bool _enable_shadow, DisplayLayer *_dst) :
		sky_dirty(true),
		ambient_dirty(true),
		light_count_dirty(true),
		static_model_instance_count_dirty(true),
		animated_model_instance_count_dirty(true),
		terrain_count_dirty(true),
		static_indirect_count(0),
		animated_indirect_count(0),
		enable_shadow(_enable_shadow),
		dst(_dst),
		resource(&globalResource),

		lights(MaxLightCount),
		static_model_instances(MaxStaticModelInstanceCount),
		animated_model_instances(MaxAnimatedModelInstanceCount),
		terrains(MaxTerrainCount),
		waters(MaxWaterCount),
		shadow_lights(MaxShadowCount)
	{
		follow_to(root_node);
		follow_to(&resolution);

		if (!defe_inited)
		{
			renderpass_color = get_renderpass(RenderPassInfo()
				.add_attachment(VK_FORMAT_R8G8B8A8_UNORM, false)
				.add_subpass({ 0 }, -1)
			);

			renderpass_color16 = get_renderpass(RenderPassInfo()
				.add_attachment(VK_FORMAT_R16G16B16A16_SFLOAT, false)
				.add_subpass({ 0 }, -1)
			);

			renderpass_defe = get_renderpass(RenderPassInfo()
				.add_attachment(VK_FORMAT_R16G16B16A16_SFLOAT, false)    // main
				.add_attachment(VK_FORMAT_D32_SFLOAT, true)              // depth
				.add_attachment(VK_FORMAT_R16G16B16A16_UNORM, true)      // albedo alpha
				.add_attachment(VK_FORMAT_R16G16B16A16_UNORM, true)      // normal height
				.add_attachment(VK_FORMAT_R16G16B16A16_UNORM, true)      // spec roughness
				.add_attachment(VK_FORMAT_R8G8B8A8_UNORM, false)   // dst
				.add_subpass({2, 3, 4}, 1)
				.add_subpass({0}, -1)
				.add_subpass({5}, -1)
				.add_dependency(0, 1)
				.add_dependency(1, 2)
			);

			scattering_pipeline = new Pipeline(PipelineInfo()
				.set_cx(512).set_cy(256)
				.set_cull_mode(VK_CULL_MODE_NONE)
				.add_shader("fullscreen.vert", { "USE_UV" })
				.add_shader("sky/scattering.frag", {}),
				renderpass_color16.get(), 0);
			output_debug_panorama_pipeline = new Pipeline(PipelineInfo()
				.set_cx(512).set_cy(256)
				.set_cull_mode(VK_CULL_MODE_NONE)
				.add_shader("fullscreen.vert", { "USE_UV" })
				.add_shader("sky/output_debug_panorama.frag", {}),
				renderpass_color.get(), 0);
			downsample_pipeline = new Pipeline(PipelineInfo()
				.set_cull_mode(VK_CULL_MODE_NONE)
				.add_shader("fullscreen.vert", { "USE_UV" })
				.add_shader("sky/downsample.frag", {})
				, renderpass_color16.get(), 0, true);
			convolve_pipeline = new Pipeline(PipelineInfo()
				.set_cull_mode(VK_CULL_MODE_NONE)
				.add_shader("fullscreen.vert", { "USE_UV" })
				.add_shader("sky/convolve.frag", {}),
				renderpass_color16.get(), 0, true);
			copy_pipeline = new Pipeline(PipelineInfo()
				.set_cull_mode(VK_CULL_MODE_NONE)
				.add_shader("fullscreen.vert", { "USE_UV" })
				.add_shader("copy.frag", {}),
				renderpass_color16.get(), 0, true);
			mrt_pipeline = new Pipeline(PipelineInfo()
				.set_vertex_input_state({ { TokenF32V3, 0 },{ TokenF32V2, 0 },{ TokenF32V3, 0 },{ TokenF32V3, 0 } })
				.set_depth_test(true)
				.set_depth_write(true)
				.add_blend_attachment_state(false)
				.add_blend_attachment_state(false)
				.add_blend_attachment_state(false)
				.add_shader("deferred/mrt.vert", {})
				.add_shader("deferred/mrt.frag", {})
				.add_uniform_buffer_link("ubo_matrix_", "Matrix.UniformBuffer")
				.add_uniform_buffer_link("ubo_object_static_", "StaticObjectMatrix.UniformBuffer"),
				renderpass_defe.get(), 0);
			mrt_anim_pipeline = new Pipeline(PipelineInfo()
				.set_vertex_input_state({ { TokenF32V3, 0 },{ TokenF32V2, 0 },{ TokenF32V3, 0 },{ TokenF32V3, 0 },{ TokenF32V4, 1 },{ TokenF32V4, 1 } })
				.set_depth_test(true)
				.set_depth_write(true)
				.add_blend_attachment_state(false)
				.add_blend_attachment_state(false)
				.add_blend_attachment_state(false)
				.add_shader("deferred/mrt.vert", { "ANIM" })
				.add_shader("deferred/mrt.frag", { "ANIM" })
				.add_uniform_buffer_link("ubo_matrix_", "Matrix.UniformBuffer")
				.add_uniform_buffer_link("ubo_object_animated_", "AnimatedObjectMatrix.UniformBuffer"),
				renderpass_defe.get(), 0);
			terrain_pipeline = new Pipeline(PipelineInfo()
				.set_vertex_input_state({ { TokenF32V3, 0 },{ TokenF32V2, 0 } })
				.set_patch_control_points(4)
				.set_depth_test(true)
				.set_depth_write(true)
				.set_primitive_topology(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)
				.add_blend_attachment_state(false)
				.add_blend_attachment_state(false)
				.add_blend_attachment_state(false)
				.add_shader("deferred/terrain.vert", {})
				.add_shader("deferred/terrain.tesc", {})
				.add_shader("deferred/terrain.tese", {})
				.add_shader("deferred/terrain.frag", {})
				.add_uniform_buffer_link("ubo_matrix_", "Matrix.UniformBuffer")
				.add_uniform_buffer_link("ubo_terrain_", "Terrain.UniformBuffer"),
				renderpass_defe.get(), 0);
			water_pipeline = new Pipeline(PipelineInfo()
				.set_patch_control_points(4)
				.set_depth_test(true)
				.set_depth_write(true)
				.set_primitive_topology(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)
				.add_blend_attachment_state(false)
				.add_blend_attachment_state(false)
				.add_blend_attachment_state(false)
				.add_shader("deferred/water.vert", {})
				.add_shader("deferred/water.tesc", {})
				.add_shader("deferred/water.tese", {})
				.add_shader("deferred/water.frag", {})
				.add_uniform_buffer_link("ubo_matrix_", "Matrix.UniformBuffer")
				.add_uniform_buffer_link("ubo_water_", "Water.UniformBuffer"),
				renderpass_defe.get(), 0);
			deferred_pipeline = new Pipeline(PipelineInfo()
				.set_cull_mode(VK_CULL_MODE_NONE)
				.add_shader("fullscreen.vert", { "USE_VIEW" })
				.add_shader("deferred/deferred.frag", { "USE_PBR" })
				.add_uniform_buffer_link("ubo_constant_", "Constant.UniformBuffer")
				.add_uniform_buffer_link("ubo_matrix_", "Matrix.UniformBuffer")
				.add_texture_link("img_depth", "Depth.Image", 0, plainUnnormalizedSampler)
				.add_texture_link("img_albedo_alpha", "AlbedoAlpha.Image", 0, plainUnnormalizedSampler)
				.add_texture_link("img_normal_height", "NormalHeight.Image", 0, plainUnnormalizedSampler)
				.add_texture_link("img_spec_roughness", "SpecRoughness.Image", 0, plainUnnormalizedSampler)
				.add_uniform_buffer_link("ubo_light_", "Light.UniformBuffer")
				.add_texture_link("img_envr", "Envr.Image", 0, colorSampler)
				.add_uniform_buffer_link("ubo_ambient_", "Ambient.UniformBuffer"),
				//.add_link("img_shadow", "Shadow.Image")
				//.add_link("ubo_shadow_", "Shadow.UniformBuffer"),
				renderpass_defe.get(), 1);
			compose_pipeline = new Pipeline(PipelineInfo()
				.set_cull_mode(VK_CULL_MODE_NONE)
				.add_shader("fullscreen.vert", {})
				.add_shader("compose/compose.frag", {})
				.add_texture_link("img_source", "Main.Image", 0, plainUnnormalizedSampler),
				renderpass_defe.get(), 2);

			defe_inited = true;
		}

		cb_defe = std::make_unique<CommandBuffer>();

		constantBuffer = std::make_unique<Buffer>(BufferTypeUniform, sizeof ConstantBufferStruct);
		matrixBuffer = std::make_unique<Buffer>(BufferTypeUniform, sizeof MatrixBufferShaderStruct);
		staticModelInstanceMatrixBuffer = std::make_unique<Buffer>(BufferTypeUniform, sizeof(glm::mat4) * MaxStaticModelInstanceCount);
		animatedModelInstanceMatrixBuffer = std::make_unique<Buffer>(BufferTypeUniform, sizeof(glm::mat4) * MaxAnimatedModelInstanceCount);
		terrainBuffer = std::make_unique<Buffer>(BufferTypeUniform, sizeof(TerrainShaderStruct) * MaxTerrainCount);
		waterBuffer = std::make_unique<Buffer>(BufferTypeUniform, sizeof(WaterShaderStruct) * MaxWaterCount);
		lightBuffer = std::make_unique<Buffer>(BufferTypeUniform, sizeof(LightBufferShaderStruct));
		ambientBuffer = std::make_unique<Buffer>(BufferTypeUniform, sizeof AmbientBufferShaderStruct);
		staticObjectIndirectBuffer = std::make_unique<Buffer>(BufferTypeIndirectIndex, sizeof(VkDrawIndexedIndirectCommand) * MaxStaticModelInstanceCount);
		animatedObjectIndirectBuffer = std::make_unique<Buffer>(BufferTypeIndirectIndex, sizeof(VkDrawIndexedIndirectCommand) * MaxAnimatedModelInstanceCount);

		envrImage = std::make_unique<Texture>(TextureTypeAttachment, EnvrSizeCx, EnvrSizeCy, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_DST_BIT, 4);
		for (int i = 0; i < 3; i++)
			envr_image_downsample[i] = new Texture(TextureTypeAttachment, EnvrSizeCx >> (i + 1), EnvrSizeCy >> (i + 1), VK_FORMAT_R16G16B16A16_SFLOAT, 0);

		resource.setBuffer(constantBuffer.get(), "Constant.UniformBuffer");
		resource.setBuffer(matrixBuffer.get(), "Matrix.UniformBuffer");
		resource.setBuffer(staticModelInstanceMatrixBuffer.get(), "StaticObjectMatrix.UniformBuffer");
		resource.setBuffer(animatedModelInstanceMatrixBuffer.get(), "AnimatedObjectMatrix.UniformBuffer");
		resource.setBuffer(terrainBuffer.get(), "Terrain.UniformBuffer");
		resource.setBuffer(waterBuffer.get(), "Water.UniformBuffer");
		resource.setBuffer(lightBuffer.get(), "Light.UniformBuffer");
		resource.setBuffer(ambientBuffer.get(), "Ambient.UniformBuffer");
		resource.setBuffer(staticObjectIndirectBuffer.get(), "Scene.Static.IndirectBuffer");
		resource.setBuffer(animatedObjectIndirectBuffer.get(), "Scene.Animated.IndirectBuffer");

		if (enable_shadow)
		{
			if (!shad_inited)
			{
				renderpass_color_and_depth = get_renderpass(RenderPassInfo()
					.add_attachment(VK_FORMAT_R8G8B8A8_UNORM, true)
					.add_attachment(VK_FORMAT_D16_UNORM, true)
					.add_subpass({ 0 }, 1)
				);

				renderpass_color32_and_depth = get_renderpass(RenderPassInfo()
					.add_attachment(VK_FORMAT_R32_SFLOAT, true)
					.add_attachment(VK_FORMAT_D16_UNORM, true)
					.add_subpass({ 0 }, 1)
				);

				esm_pipeline = new Pipeline(PipelineInfo()
					.set_cx(2048).set_cy(2048)
					.set_vertex_input_state({ { TokenF32V3, 0 },{ TokenF32V2, 0 },{ TokenF32V3, 0 },{ TokenF32V3, 0 } })
					.set_depth_test(true)
					.set_depth_write(true)
					.add_shader("esm/esm.vert", {})
					.add_shader("esm/esm.frag", {})
					.add_uniform_buffer_link("ubo_constant_", "Constant.UniformBuffer")
					.add_uniform_buffer_link("ubo_object_static_", "StaticObjectMatrix.UniformBuffer")
					.add_uniform_buffer_link("u_shadow_", "Shadow.UniformBuffer"),
					renderpass_color_and_depth.get(), 0);
				esm_anim_pipeline = new Pipeline(PipelineInfo()
					.set_cx(2048).set_cy(2048)
					.set_vertex_input_state({ { TokenF32V3, 0 },{ TokenF32V2, 0 },{ TokenF32V3, 0 },{ TokenF32V3, 0 },{ TokenF32V4, 1 },{ TokenF32V4, 1 } })
					.set_depth_test(true)
					.set_depth_write(true)
					.add_shader("esm/esm.vert", { "ANIM" })
					.add_shader("esm/esm.frag", { "ANIM" })
					.add_uniform_buffer_link("ubo_constant_", "Constant.UniformBuffer")
					.add_uniform_buffer_link("ubo_object_animated_", "AnimatedObjectMatrix.UniformBuffer")
					.add_uniform_buffer_link("u_shadow_", "Shadow.UniformBuffer"),
					renderpass_color_and_depth.get(), 0);

				shad_inited = true;
			}

			cb_shad = std::make_unique<CommandBuffer>();

			shadowBuffer = std::make_unique<Buffer>(BufferTypeUniform, sizeof(glm::mat4) * MaxShadowCount);

			esmImage = std::make_unique<Texture>(TextureTypeAttachment, ShadowMapCx, ShadowMapCy, VK_FORMAT_R32_SFLOAT, 0, 1, MaxShadowCount * 6);
			esmDepthImage = std::make_unique<Texture>(TextureTypeAttachment, ShadowMapCx, ShadowMapCy, VK_FORMAT_D16_UNORM, 0);

			resource.setImage(esmImage.get(), "Shadow.Image");

			for (int i = 0; i < MaxShadowCount * 6; i++)
			{
				VkImageView views[] = {
					esmImage->get_view(VK_IMAGE_VIEW_TYPE_2D, 0, 1, i),
					esmDepthImage->get_view()
				};
				fb_esm[i] = get_framebuffer(ShadowMapCx, ShadowMapCy, renderpass_color32_and_depth.get(), TK_ARRAYSIZE(views), views);
			}

			ds_esm = std::make_unique<DescriptorSet>(esm_pipeline);
			ds_esmAnim = std::make_unique<DescriptorSet>(esm_anim_pipeline);

			resource.setBuffer(shadowBuffer.get(), "Shadow.UniformBuffer");

			esm_pipeline->link_descriptors(ds_esm.get(), &resource);
			esm_anim_pipeline->link_descriptors(ds_esmAnim.get(), &resource);
		}

		ds_mrt = std::make_unique<DescriptorSet>(mrt_pipeline);
		ds_mrtAnim = std::make_unique<DescriptorSet>(mrt_anim_pipeline);
		ds_mrtAnim_bone = std::make_unique<DescriptorSet>(mrt_anim_pipeline, 2);
		ds_terrain = std::make_unique<DescriptorSet>(terrain_pipeline);
		ds_water = std::make_unique<DescriptorSet>(water_pipeline);
		ds_defe = std::make_unique<DescriptorSet>(deferred_pipeline);
		ds_comp = std::make_unique<DescriptorSet>(compose_pipeline);

		create_resolution_related();
	}

	DeferredRenderer::~DeferredRenderer()
	{
		break_link(root_node, this);
	}

	void DeferredRenderer::create_resolution_related()
	{
		if (mainImage && mainImage->get_cx() == resolution.x() && mainImage->get_cy() == resolution.y())
			return;

		mainImage = std::make_unique<Texture>(TextureTypeAttachment, resolution.x(), resolution.y(), VK_FORMAT_R16G16B16A16_SFLOAT, 0);
		depthImage = std::make_unique<Texture>(TextureTypeAttachment, resolution.x(), resolution.y(), VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_SAMPLED_BIT);
		albedoAlphaImage = std::make_unique<Texture>(TextureTypeAttachment, resolution.x(), resolution.y(), VK_FORMAT_R16G16B16A16_UNORM, 0);
		normalHeightImage = std::make_unique<Texture>(TextureTypeAttachment, resolution.x(), resolution.y(), VK_FORMAT_R16G16B16A16_UNORM, 0);
		specRoughnessImage = std::make_unique<Texture>(TextureTypeAttachment, resolution.x(), resolution.y(), VK_FORMAT_R16G16B16A16_UNORM, 0);

		resource.setImage(envrImage.get(), "Envr.Image");
		resource.setImage(mainImage.get(), "Main.Image");
		resource.setImage(depthImage.get(), "Depth.Image");
		resource.setImage(albedoAlphaImage.get(), "AlbedoAlpha.Image");
		resource.setImage(normalHeightImage.get(), "NormalHeight.Image");
		resource.setImage(specRoughnessImage.get(), "SpecRoughness.Image");

		mrt_pipeline->link_descriptors(ds_mrt.get(), &resource);
		mrt_anim_pipeline->link_descriptors(ds_mrtAnim.get(), &resource);
		terrain_pipeline->link_descriptors(ds_terrain.get(), &resource);
		water_pipeline->link_descriptors(ds_water.get(), &resource);
		deferred_pipeline->link_descriptors(ds_defe.get(), &resource);
		compose_pipeline->link_descriptors(ds_comp.get(), &resource);

		{
			VkImageView views[] = {
				mainImage->get_view(),
				depthImage->get_view(),
				albedoAlphaImage->get_view(),
				normalHeightImage->get_view(),
				specRoughnessImage->get_view(),
				dst->image->get_view(),
			};
			framebuffer = get_framebuffer(resolution.x(), resolution.y(), renderpass_defe.get(), ARRAYSIZE(views), views);
		}
	}

	void DeferredRenderer::render(Scene *scene, CameraComponent *camera)
	{
		if (constant_buffer_updated_frame < resolution.dirty_frame())
		{
			ConstantBufferStruct stru;
			stru.depth_near = near_plane;
			stru.depth_far = far_plane;
			stru.cx = resolution.x();
			stru.cy = resolution.y();
			stru.aspect = resolution.aspect();
			stru.fovy = fovy;
			stru.tanHfFovy = std::tan(glm::radians(fovy * 0.5f));
			stru.envrCx = EnvrSizeCx;
			stru.envrCy = EnvrSizeCy;
			constantBuffer->update(&stru);

			constant_buffer_updated_frame = total_frame_count;
		}
		{ // always update the matrix buffer
			MatrixBufferShaderStruct stru;
			stru.proj = camera->get_proj_matrix();
			stru.projInv = camera->get_proj_matrix_inverse();
			stru.view = camera->get_view_matrix();
			stru.viewInv = camera->get_parent()->get_matrix();
			stru.projView = stru.proj * stru.view;
			stru.projViewRotate = stru.proj * glm::mat4(glm::mat3(stru.view));
			memcpy(stru.frustumPlanes, camera->get_frustum_planes(), sizeof(MatrixBufferShaderStruct::frustumPlanes));
			stru.viewportDim = glm::vec2(resolution.x(), resolution.y());
			matrixBuffer->update(&stru);
		}

		if (sky_dirty)
		{
			auto funUpdateIBL = [&]() {
				for (int i = 0; i < envrImage->levels.size() - 1; i++)
				{
					auto cb = begin_once_command_buffer();
					auto fb = get_framebuffer(envr_image_downsample[i], renderpass_color16.get());

					cb->begin_renderpass(renderpass_color16.get(), fb.get());
					cb->bind_pipeline(downsample_pipeline);
					cb->set_viewport_and_scissor(EnvrSizeCx >> (i + 1), EnvrSizeCy >> (i + 1));
					auto size = glm::vec2(EnvrSizeCx >> (i + 1), EnvrSizeCy >> (i + 1));
					cb->push_constant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof glm::vec2, &size);
					updateDescriptorSets(1, &downsample_pipeline->descriptor_set->get_write(0, 0, &get_texture_info(i == 0 ? envrImage.get() : envr_image_downsample[i - 1], plainSampler)));
					cb->bind_descriptor_set();
					cb->draw(3);
					cb->end_renderpass();

					end_once_command_buffer(cb);
				}

				for (int i = 1; i < envrImage->levels.size(); i++)
				{
					auto cb = begin_once_command_buffer();
					auto fb = get_framebuffer(envrImage.get(), renderpass_color16.get(), i);

					cb->begin_renderpass(renderpass_color16.get(), fb.get());
					cb->bind_pipeline(convolve_pipeline);
					auto data = 1.f + 1024.f - 1024.f * (i / 3.f);
					cb->push_constant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &data);
					cb->set_viewport_and_scissor(EnvrSizeCx >> i, EnvrSizeCy >> i);
					updateDescriptorSets(1, &convolve_pipeline->descriptor_set->get_write(0, 0, &get_texture_info(envr_image_downsample[i - 1], plainSampler)));
					cb->bind_descriptor_set();
					cb->draw(3);
					cb->end_renderpass();

					end_once_command_buffer(cb);
				}
			};

			switch (scene->get_sky_type())
			{
				case SkyTypeNull:
				{
					auto cb = begin_once_command_buffer();
					envrImage->transition_layout(cb, envrImage->layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
					auto col = scene->get_bg_color();
					VkClearColorValue clear_value = {col.r, col.g, col.b, 1.f};
					VkImageSubresourceRange range = {
						VK_IMAGE_ASPECT_COLOR_BIT,
						0, envrImage->levels.size(),
						0, envrImage->layer_count
					};
					vkCmdClearColorImage(cb->v, envrImage->v, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_value, 1, &range);
					envrImage->transition_layout(cb, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, envrImage->layout);
					end_once_command_buffer(cb);
				}
					break;
				case SkyTypeDebug:
				{
					auto cb = begin_once_command_buffer();
					auto fb = get_framebuffer(envrImage.get(), renderpass_color.get());

					cb->begin_renderpass(renderpass_color.get(), fb.get());
					cb->bind_pipeline(output_debug_panorama_pipeline);
					cb->draw(3);
					cb->end_renderpass();

					end_once_command_buffer(cb);

					funUpdateIBL();

					break;
				}
				case SkyTypeAtmosphereScattering:
				{
					auto as = (SkyAtmosphereScattering*)scene->get_sky();

					auto cb = begin_once_command_buffer();
					auto fb = get_framebuffer(envrImage.get(), renderpass_color16.get());

					cb->begin_renderpass(renderpass_color16.get(), fb.get());
					cb->bind_pipeline(scattering_pipeline);
					auto dir = -as->sun_light->get_parent()->get_axis()[2];
					cb->push_constant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(dir), &dir);
					cb->draw(3);
					cb->end_renderpass();

					end_once_command_buffer(cb);

					funUpdateIBL();

					break;
				}
				case SkyTypePanorama:
				{
					auto pa = (SkyPanorama*)scene->get_sky();

					if (pa->panoImage)
					{
						auto cb = begin_once_command_buffer();
						auto fb = get_framebuffer(envrImage.get(), renderpass_color16.get());

						cb->begin_renderpass(renderpass_color16.get(), fb.get());
						cb->bind_pipeline(copy_pipeline);
						cb->set_viewport_and_scissor(EnvrSizeCx, EnvrSizeCy);
						updateDescriptorSets(1, &copy_pipeline->descriptor_set->get_write(0, 0, &get_texture_info(pa->panoImage.get(), colorSampler)));
						cb->bind_descriptor_set();
						cb->draw(3);
						cb->end_renderpass();

						end_once_command_buffer(cb);

						funUpdateIBL();
					}
					else
					{
						auto cb = begin_once_command_buffer();
						envrImage->transition_layout(cb, envrImage->layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
						VkClearColorValue clear_value = {};
						VkImageSubresourceRange range = {
							VK_IMAGE_ASPECT_COLOR_BIT,
							0, envrImage->levels.size(),
							0, envrImage->layer_count
						};
						vkCmdClearColorImage(cb->v, envrImage->v, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_value, 1, &range);
						envrImage->transition_layout(cb, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, envrImage->layout);
						end_once_command_buffer(cb);
					}
					break;
				}
			}

			sky_dirty = false;
		}
		if (ambient_dirty)
		{
			AmbientBufferShaderStruct stru;
			stru.color = scene->get_ambient_color();
			stru.envr_max_mipmap = envrImage->levels.size() - 1;
			stru.fogcolor = glm::vec4(scene->get_fog_color(), 1.f); // TODO : FIX FOG COLOR ACCORDING TO SKY
			ambientBuffer->update(&stru);

			ambient_dirty = false;
		}
		auto fUpdateModelInstanceMatrixBuffer = [&](SpareList &list, Buffer *buffer) {
			if (list.get_size() > 0)
			{
				std::vector<VkBufferCopy> ranges;
				defalut_staging_buffer->map(0, sizeof(glm::mat4) * list.get_size());
				auto map = (unsigned char*)defalut_staging_buffer->mapped;

				list.iterate([&](int index, void *p, bool &remove) {
					auto i = (ModelInstanceComponent*)p;
					auto n = i->get_parent();
					auto &updated_frame = i->get_model()->vertexes_skeleton.size() > 0 ? 
						animated_model_instance_auxes[index].matrix_updated_frame
						: static_model_instance_auxes[index].matrix_updated_frame;
					if (updated_frame < n->get_transform_dirty_frame())
					{
						auto srcOffset = sizeof(glm::mat4) * ranges.size();
						memcpy(map + srcOffset, &n->get_world_matrix(), sizeof(glm::mat4));
						VkBufferCopy range = {};
						range.srcOffset = srcOffset;
						range.dstOffset = sizeof(glm::mat4) * index;
						range.size = sizeof(glm::mat4);
						ranges.push_back(range);

						updated_frame = total_frame_count;
					}
					return true;
				});
				defalut_staging_buffer->unmap();
				defalut_staging_buffer->copy_to(buffer, ranges.size(), ranges.data());
			}
		};
		fUpdateModelInstanceMatrixBuffer(static_model_instances, staticModelInstanceMatrixBuffer.get());
		fUpdateModelInstanceMatrixBuffer(animated_model_instances, animatedModelInstanceMatrixBuffer.get());

		std::vector<VkWriteDescriptorSet> writes;

		if (terrains.get_size() > 0)
		{
			std::vector<VkBufferCopy> ranges;
			defalut_staging_buffer->map(0, sizeof(TerrainShaderStruct) * terrains.get_size());
			auto map = (unsigned char*)defalut_staging_buffer->mapped;

			terrains.iterate([&](int index, void *p, bool &remove) {
				auto t = (TerrainComponent*)p;
				auto n = t->get_parent();
				if (terrain_auxes[index].attribute_updated_frame < n->get_transform_dirty_frame() || 
					terrain_auxes[index].attribute_updated_frame < t->get_attribute_dirty_frame())
				{
					auto srcOffset = sizeof(TerrainShaderStruct) * ranges.size();
					TerrainShaderStruct stru;
					stru.coord = n->get_world_coord();
					stru.block_cx = t->get_block_cx();
					stru.block_cy = t->get_block_cy();
					stru.block_size = t->get_block_size();
					stru.terrain_height = t->get_height();
					stru.displacement_height = t->get_displacement_height();
					stru.tessellation_factor = t->get_tessellation_factor();
					stru.tiling_scale = t->get_tiling_scale();
					for (int i = 0; i < 4; i++)
					{
						stru.material_index.v[i] = t->get_material(i) ?
							t->get_material(i)->get_index() : 0;
					}
					stru.material_count = t->get_material_count();
					memcpy(map + srcOffset, &stru, sizeof(TerrainShaderStruct));
					VkBufferCopy range = {};
					range.srcOffset = srcOffset;
					range.dstOffset = sizeof(TerrainShaderStruct) * index;
					range.size = sizeof(TerrainShaderStruct);
					ranges.push_back(range);

					terrain_auxes[index].attribute_updated_frame = total_frame_count;
				}
				if (terrain_auxes[index].blend_map_updated_frame < t->get_blend_image_dirty_frame())
				{
					writes.push_back(ds_terrain->get_write(TerrainBlendImageDescriptorBinding,
						index, &get_texture_info(t->get_blend_image(), colorBorderSampler)));

					terrain_auxes[index].blend_map_updated_frame = total_frame_count;
				}
				return true;
			});

			defalut_staging_buffer->unmap();
			defalut_staging_buffer->copy_to(terrainBuffer.get(), ranges.size(), ranges.data());
		}
		if (waters.get_size() > 0)
		{
			std::vector<VkBufferCopy> ranges;
			defalut_staging_buffer->map(0, sizeof(WaterShaderStruct) * waters.get_size());
			auto map = (unsigned char*)defalut_staging_buffer->mapped;

			waters.iterate([&](int index, void *p, bool &remove) {
				auto w = (WaterComponent*)p;
				auto n = w->get_parent();
				if (water_auxes[index].attribute_updated_frame < n->get_transform_dirty_frame() || 
					water_auxes[index].attribute_updated_frame < w->get_attribute_dirty_frame())
				{
					auto srcOffset = sizeof(WaterShaderStruct) * ranges.size();
					WaterShaderStruct stru;
					stru.coord = glm::vec3(n->get_world_matrix()[3]);
					stru.block_cx = w->get_block_cx();
					stru.block_cy = w->get_block_cy();
					stru.block_size = w->get_block_size();
					stru.height = w->get_height();
					stru.tessellation_factor = w->get_tessellation_factor();
					stru.tiling_scale = w->get_tiling_scale();
					stru.mapDimension = 1024;
					memcpy(map + srcOffset, &stru, sizeof(WaterShaderStruct));
					VkBufferCopy range = {};
					range.srcOffset = srcOffset;
					range.dstOffset = sizeof(WaterShaderStruct) * index;
					range.size = sizeof(WaterShaderStruct);
					ranges.push_back(range);

					water_auxes[index].attribute_updated_frame = total_frame_count;
				}
				return true;
			});
			defalut_staging_buffer->unmap();
			defalut_staging_buffer->copy_to(waterBuffer.get(), ranges.size(), ranges.data());
		}

		auto fUpdateIndirect = [](SpareList &list, Buffer *buffer, int &out_count) {
			if (list.get_size() > 0)
			{
				std::vector<VkDrawIndexedIndirectCommand> commands;
				list.iterate([&](int index, void *p, bool &remove) {
					auto i = (ModelInstanceComponent*)p;
					auto m = i->get_model();
					for (auto &g : m->geometries)
					{
						VkDrawIndexedIndirectCommand command = {};
						command.instanceCount = 1;
						command.indexCount = g->indiceCount;
						command.vertexOffset = m->vertex_base;
						command.firstIndex = m->indice_base + g->indiceBase;
						command.firstInstance = (index << 8) + g->material->get_index();
						commands.push_back(command);
					}
					return true;
				});
				buffer->update(commands.data(), sizeof(VkDrawIndexedIndirectCommand) * commands.size());
				out_count = commands.size();
			}
		};
		if (static_model_instance_count_dirty)
		{
			fUpdateIndirect(static_model_instances, staticObjectIndirectBuffer.get(), static_indirect_count);
			static_model_instance_count_dirty = false;
		}
		if (animated_model_instance_count_dirty)
		{
			fUpdateIndirect(animated_model_instances, animatedObjectIndirectBuffer.get(), animated_indirect_count);
			animated_model_instance_count_dirty = false;
		}

		if (light_count_dirty)
		{
			unsigned int count = lights.get_size();
			lightBuffer->update(&count, sizeof(int));

			light_count_dirty = false;
		}

		if (lights.get_size() > 0)
		{ // light attribute
			std::vector<VkBufferCopy> ranges;
			defalut_staging_buffer->map(0, sizeof(LightShaderStruct) * lights.get_size());
			auto map = (unsigned char*)defalut_staging_buffer->mapped;

			lights.iterate([&](int index, void *p, bool &remove) {
				auto l = (LightComponent*)p;
				auto n = l->get_parent();
				if (light_auxes[index].attribute_updated_frame < n->get_transform_dirty_frame() ||
					light_auxes[index].attribute_updated_frame < l->get_attribute_dirty_frame())
				{
					LightShaderStruct stru;
					if (l->get_type() == LightTypeParallax)
						stru.coord = glm::vec4(n->get_world_axis()[2], 0.f);
					else
						stru.coord = glm::vec4(n->get_world_coord(), l->get_type());
					stru.color = glm::vec4(l->get_color(), l->get_shadow_index() * 6);
					stru.spotData = glm::vec4(-n->get_world_axis()[2], l->get_range());
					auto srcOffset = sizeof(LightShaderStruct) * ranges.size();
					memcpy(map + srcOffset, &stru, sizeof(LightShaderStruct));
					VkBufferCopy range = {};
					range.srcOffset = srcOffset;
					range.dstOffset = 16 + sizeof(LightShaderStruct) * l->get_light_index();
					range.size = sizeof(LightShaderStruct);
					ranges.push_back(range);

					light_auxes[index].attribute_updated_frame = total_frame_count;
				}
				return true;
			});
			defalut_staging_buffer->unmap();
			defalut_staging_buffer->copy_to(lightBuffer.get(), ranges.size(), ranges.data());
		}
		if (enable_shadow)
		{
			if (shadow_lights.get_size() > 0)
			{
				shadow_lights.iterate([&](int index, void *p, bool &remove) {
					std::vector<VkBufferCopy> ranges;
					defalut_staging_buffer->map(0, sizeof(glm::mat4) * shadow_lights.get_size());
					auto map = (unsigned char*)defalut_staging_buffer->mapped;

					auto l = (LightComponent*)p;
					auto n = l->get_parent();
					if (l->get_type() == LightTypeParallax)
					{
						if (light_auxes[index].attribute_updated_frame < n->get_transform_dirty_frame() || 
							light_auxes[index].attribute_updated_frame < camera->get_parent()->get_transform_dirty_frame())
						{
							glm::vec3 p[8];
							auto cameraCoord = camera->get_parent()->get_world_coord();
							for (int i = 0; i < 8; i++)
								p[i] = camera->get_frustum_points()[i] - cameraCoord;
							auto lighAxis = n->get_world_axis();
							auto axisT = glm::transpose(lighAxis);
							auto vMax = axisT * p[0], vMin = vMax;
							for (int i = 1; i < 8; i++)
							{
								auto tp = axisT * p[i];
								vMax = glm::max(tp, vMax);
								vMin = glm::min(tp, vMin);
							}
							auto halfWidth = (vMax.z - vMin.z) * 0.5f;
							auto halfHeight = (vMax.y - vMin.y) * 0.5f;
							auto halfDepth = glm::max(vMax.x - vMin.x, near_plane) * 0.5f;
							auto center = lighAxis * ((vMax + vMin) * 0.5f) + cameraCoord;
							//auto shadowMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, TKE_NEAR, halfDepth + halfDepth) * 
							//glm::lookAt(center + halfDepth * lighAxis[2], center, lighAxis[1]);
							auto camera_coord = camera->get_parent()->get_world_coord();
							auto shadowMatrix = glm::mat4(
								1.f, 0.f, 0.f, 0.f,
								0.f, 1.f, 0.f, 0.f,
								0.f, 0.f, 0.5f, 0.f,
								0.f, 0.f, 0.5f, 1.f
							) * glm::ortho(-1.f, 1.f, -1.f, 1.f, near_plane, far_plane) *
								glm::lookAt(camera_coord + glm::vec3(0, 100, 0), camera_coord, glm::vec3(0, 1, 0));

							auto srcOffset = sizeof(glm::mat4) * ranges.size();
							memcpy(map + srcOffset, &shadowMatrix, sizeof(glm::mat4));
							VkBufferCopy range = {};
							range.srcOffset = srcOffset;
							range.dstOffset = sizeof(glm::mat4) * (index * 6);
							range.size = sizeof(glm::mat4);
							ranges.push_back(range);

							light_auxes[index].attribute_updated_frame = total_frame_count;
						}
					}
					else if (l->get_type() == LightTypePoint)
					{
						if (light_auxes[index].attribute_updated_frame < n->get_transform_dirty_frame())
						{
							glm::mat4 shadowMatrix[6];

							auto coord = n->get_world_coord();
							auto proj = glm::perspective(90.f, 1.f, near_plane, far_plane);
							shadowMatrix[0] = proj * glm::lookAt(coord, coord + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
							shadowMatrix[1] = proj * glm::lookAt(coord, coord + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
							shadowMatrix[2] = proj * glm::lookAt(coord, coord + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
							shadowMatrix[3] = proj * glm::lookAt(coord, coord + glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
							shadowMatrix[4] = proj * glm::lookAt(coord, coord + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
							shadowMatrix[5] = proj * glm::lookAt(coord, coord + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));

							light_auxes[index].attribute_updated_frame = total_frame_count;
						}
					}
					defalut_staging_buffer->unmap();
					defalut_staging_buffer->copy_to(shadowBuffer.get(), ranges.size(), ranges.data());
					return true;
				});
			}
		}

		updateDescriptorSets(writes.size(), writes.data());

		if (enable_shadow)
		{
			cb_shad->begin();

			shadow_lights.iterate([&](int index, void *p, bool &remove) {
				auto l = (LightComponent*)p;

				static VkClearValue clearValues[] = {
					{ 1.f, 0 },
					{ 1.f, 1.f, 1.f, 1.f }
				};
				cb_shad->begin_renderpass(renderpass_color32_and_depth.get(), fb_esm[index].get(), clearValues);

				cb_shad->bind_vertex_buffer2(vertex_static_buffer.get(), vertex_skeleton_Buffer.get());
				cb_shad->bind_index_buffer(index_buffer.get());

				auto fDrawDepth = [&](SpareList &list) {
					list.iterate([&](int index, void *p, bool &remove) {
						auto i = (ModelInstanceComponent*)p;
						auto m = i->get_model();
						for (int gId = 0; gId < m->geometries.size(); gId++)
							cb_shad->draw_model(m, gId, 1, ((l->get_shadow_index() * 6) << 28) + (index << 8) + gId);
						return true;
					});
				};
				if (static_model_instances.get_size() > 0)
				{
					cb_shad->bind_pipeline(esm_pipeline);
					VkDescriptorSet sets[] = {
						ds_esm->v,
						ds_material->v
					};
					cb_shad->bind_descriptor_set(sets, 0, TK_ARRAYSIZE(sets));
					fDrawDepth(static_model_instances);
				}

				if (animated_model_instances.get_size() > 0)
				{
					cb_shad->bind_pipeline(esm_anim_pipeline);
					VkDescriptorSet sets[] = {
						ds_esmAnim->v,
						ds_material->v,
						ds_mrtAnim_bone->v
					};
					cb_shad->bind_descriptor_set(sets, 0, TK_ARRAYSIZE(sets));
					fDrawDepth(animated_model_instances);
				}
				cb_shad->end_renderpass();
				return true;
			});

			cb_shad->end();
		}

		cb_defe->begin();

		cb_defe->begin_renderpass(renderpass_defe.get(), framebuffer.get());
		cb_defe->set_viewport_and_scissor(resolution.x(), resolution.y());

		cb_defe->bind_vertex_buffer2(vertex_static_buffer.get(), vertex_skeleton_Buffer.get());
		cb_defe->bind_index_buffer(index_buffer.get());

		// mrt
		// static
		if (static_model_instances.get_size() > 0)
		{
			cb_defe->bind_pipeline(mrt_pipeline);
			VkDescriptorSet sets[] = {
				ds_mrt->v,
				ds_material->v
			};
			cb_defe->bind_descriptor_set(sets, 0, TK_ARRAYSIZE(sets));
			cb_defe->draw_indirect_index(staticObjectIndirectBuffer.get(), static_indirect_count);
		}
		// animated
		if (animated_model_instances.get_size())
		{
			cb_defe->bind_pipeline(mrt_anim_pipeline);
			VkDescriptorSet sets[] = {
				ds_mrtAnim->v,
				ds_material->v,
				ds_mrtAnim_bone->v
			};
			cb_defe->bind_descriptor_set(sets, 0, TK_ARRAYSIZE(sets));
			cb_defe->draw_indirect_index(animatedObjectIndirectBuffer.get(), animated_indirect_count);
		}
		// terrain
		if (terrains.get_size() > 0)
		{
			cb_defe->bind_pipeline(terrain_pipeline);
			VkDescriptorSet sets[] = {
				ds_terrain->v,
				ds_material->v
			};
			cb_defe->bind_descriptor_set(sets, 0, TK_ARRAYSIZE(sets));
			terrains.iterate([&](int index, void *p, bool &remove) {
				auto t = (TerrainComponent*)p;
				cb_defe->draw(4, 0, (index << 16) + t->get_block_cx() * t->get_block_cx());
				return true;
			});
		}
		// water
		if (waters.get_size() > 0)
		{
			cb_defe->bind_pipeline(water_pipeline);
			cb_defe->bind_descriptor_set(&ds_water->v);
			waters.iterate([&](int index, void *p, bool &remove) {
				auto w = (WaterComponent*)p;
				cb_defe->draw(4, 0, (index << 16) + w->get_block_cx() * w->get_block_cx());
				return true;
			});
		}

		//cb->imageBarrier(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
		//	VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, 
		//	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
		//	esmImage.get(), 0, 1, 0, TKE_MAX_SHADOW_COUNT * 8);

		// deferred
		cb_defe->next_subpass();
		cb_defe->bind_pipeline(deferred_pipeline);
		cb_defe->bind_descriptor_set(&ds_defe->v);
		cb_defe->draw(3);

		// compose
		cb_defe->next_subpass();
		cb_defe->bind_pipeline(compose_pipeline);
		cb_defe->bind_descriptor_set(&ds_comp->v);
		cb_defe->draw(3);

		cb_defe->end_renderpass();

		cb_defe->end();
	}

	void DeferredRenderer::add_to_drawlist()
	{
		if (enable_shadow)
			add_to_draw_list(cb_shad->v);
		add_to_draw_list(cb_defe->v);
	}
}
