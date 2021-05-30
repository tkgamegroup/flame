//namespace flame
//{
//	namespace graphics
//	{
//		struct ArmatureDeformer
//		{
//			virtual void set_pose(uint id, const mat4& pose) = 0;
//
//			FLAME_GRAPHICS_EXPORTS static ArmatureDeformer* create(RenderPreferences* preferences, Mesh* mesh);
//		};
// 
//		struct ArmatureDeformerPrivate : ArmatureDeformer
//		{
//			MeshPrivate* mesh;
//			//ShaderBuffer poses_buffer;
//			std::unique_ptr<DescriptorSetPrivate> descriptorset;
//
//			ArmatureDeformerPrivate(RenderPreferencesPrivate* preferences, MeshPrivate* mesh);
//			void release() override { delete this; }
//			void set_pose(uint id, const mat4& pose) override;
//		};
//
//		struct MeshWeight
//		{
//			ivec4 ids;
//			vec4 weights;
//		};
//
//		struct CmdBlur : Cmd
//		{
//			Rect range;
//			uint radius;
//
//			CmdBlur(const Rect& _range, uint _radius) : Cmd(Blur) { range = _range; radius = _radius; }
//		};
//
//		struct CanvasPrivate : Canvas
//		{
//			std::unique_ptr<ImagePrivate> pickup_image;
//			std::unique_ptr<FramebufferPrivate> pickup_framebuffer;
//
//			ShaderGeometryBuffer<Line> line_buffer;
//			ShaderGeometryBuffer<Triangle> triangle_buffer;
//		};
// 
//		RenderPreferencesPrivate::RenderPreferencesPrivate(DevicePtr device, bool hdr)
//		{
//			//mesh_wireframe_pipeline = get_material_pipeline(MaterialForMesh, L"", "WIREFRAME");
//			//mesh_armature_wireframe_pipeline = get_material_pipeline(MaterialForMeshArmature, L"", "WIREFRAME");
//			//terrain_wireframe_pipeline = get_material_pipeline(MaterialForTerrain, L"", "WIREFRAME");
//
//			//mesh_pickup_pipeline = get_material_pipeline(MaterialForMesh, L"", "PICKUP");
//			//mesh_armature_pickup_pipeline = get_material_pipeline(MaterialForMeshArmature, L"", "PICKUP");
//			//terrain_pickup_pipeline = get_material_pipeline(MaterialForTerrain, L"", "PICKUP");
//
//			//mesh_outline_pipeline = get_material_pipeline(MaterialForMesh, L"", "OUTLINE");
//			//mesh_armature_outline_pipeline = get_material_pipeline(MaterialForMeshArmature, L"", "OUTLINE");
//			//terrain_outline_pipeline = get_material_pipeline(MaterialForTerrain, L"", "OUTLINE");
//
//			//{
//			//	ShaderPrivate* shaders[] = {
//			//		ShaderPrivate::get(device, L"plain/plain.vert"),
//			//		ShaderPrivate::get(device, L"plain/plain.frag")
//			//	};
//			//	VertexAttributeInfo vias[2];
//			//	vias[0].location = 0;
//			//	vias[0].format = Format_R32G32B32_SFLOAT;
//			//	vias[1].location = 1;
//			//	vias[1].format = Format_R8G8B8A8_UNORM;
//			//	VertexBufferInfo vib;
//			//	vib.attributes_count = _countof(vias);
//			//	vib.attributes = vias;
//			//	VertexInfo vi;
//			//	vi.primitive_topology = PrimitiveTopologyLineList;
//			//	vi.buffers_count = 1;
//			//	vi.buffers = &vib;
//			//	RasterInfo rst;
//			//	rst.cull_mode = CullModeNone;
//			//	DepthInfo dep;
//			//	dep.test = true;
//			//	dep.write = false;
//			//	BlendOption bo;
//			//	bo.enable = true;
//			//	bo.src_color = BlendFactorSrcAlpha;
//			//	bo.dst_color = BlendFactorOneMinusSrcAlpha;
//			//	bo.src_alpha = BlendFactorOne;
//			//	bo.dst_alpha = BlendFactorZero;
//			//	auto pll = PipelineLayoutPrivate::get(device, L"plain/plain.pll");
//			//	auto rp = mesh_renderpass.get();
//			//	line_pipeline.reset(PipelinePrivate::create(device, shaders, pll, rp, 0, &vi, &rst, &dep, { &bo, 1 }));
//			//	vi.primitive_topology = PrimitiveTopologyTriangleList;
//			//	triangle_pipeline.reset(PipelinePrivate::create(device, shaders, pll, rp, 0, &vi, &rst, &dep, { &bo, 1 }));
//			//}
//
//			//for (auto i = 0; i < 10; i++)
//			//{
//			//	{
//			//		ShaderPrivate* shaders[] = {
//			//			fullscreen_vert,
//			//			ShaderPrivate::get(device, L"post/blur.frag", "R" + std::to_string(i + 1) + " H")
//			//		};
//			//		blurh_pipeline[i].reset(PipelinePrivate::create(device, shaders, post_pll, hdr ? rgba16_renderpass.get() : rgba8_renderpass.get(), 0));
//			//	}
//
//			//	{
//			//		ShaderPrivate* shaders[] = {
//			//			fullscreen_vert,
//			//			ShaderPrivate::get(device, L"post/blur.frag", "R" + std::to_string(i + 1) + " V")
//			//		};
//			//		blurv_pipeline[i].reset(PipelinePrivate::create(device, shaders, post_pll, hdr ? rgba16_renderpass.get() : rgba8_renderpass.get(), 0));
//			//	}
//			//}
//
//			//{
//			//	ShaderPrivate* shaders[] = {
//			//		fullscreen_vert,
//			//		ShaderPrivate::get(device, L"post/blur_depth.frag", "H")
//			//	};
//			//	blurh_depth_pipeline.reset(PipelinePrivate::create(device, shaders, post_pll, r16_renderpass.get(), 0));
//			//}
//
//			//{
//			//	ShaderPrivate* shaders[] = {
//			//		fullscreen_vert,
//			//		ShaderPrivate::get(device, L"post/blur_depth.frag", "V")
//			//	};
//			//	blurv_depth_pipeline.reset(PipelinePrivate::create(device, shaders, post_pll, r16_renderpass.get(), 0));
//			//}
//
//			//{
//			//	ShaderPrivate* shaders[] = {
//			//		fullscreen_vert,
//			//		ShaderPrivate::get(device, L"post/blit.frag")
//			//	};
//			//	blit_8_pipeline.reset(PipelinePrivate::create(device, shaders, post_pll, rgba8_renderpass.get(), 0));
//			//	blit_16_pipeline.reset(PipelinePrivate::create(device, shaders, post_pll, rgba16_renderpass.get(), 0));
//			//	BlendOption bo;
//			//	bo.enable = true;
//			//	bo.src_color = BlendFactorSrcAlpha;
//			//	bo.dst_color = BlendFactorOneMinusSrcAlpha;
//			//	bo.src_alpha = BlendFactorOne;
//			//	bo.dst_alpha = BlendFactorZero;
//			//	blend_8_pipeline.reset(PipelinePrivate::create(device, shaders, post_pll, rgba8_renderpass.get(), 0, nullptr, nullptr, nullptr, { &bo, 1 }));
//			//	blend_16_pipeline.reset(PipelinePrivate::create(device, shaders, post_pll, rgba16_renderpass.get(), 0, nullptr, nullptr, nullptr, { &bo, 1 }));
//			//}
//
//			//{
//			//	ShaderPrivate* shaders[] = {
//			//		fullscreen_vert,
//			//		ShaderPrivate::get(device, L"post/filter_bright.frag")
//			//	};
//			//	filter_bright_pipeline.reset(PipelinePrivate::create(device, shaders, post_pll, rgba16_renderpass.get(), 0));
//			//}
//
//			//auto box_frag = ShaderPrivate::get(device, L"post/box.frag");
//
//			//{
//			//	ShaderPrivate* shaders[] = {
//			//		fullscreen_vert,
//			//		box_frag
//			//	};
//			//	downsample_pipeline.reset(PipelinePrivate::create(device, shaders, post_pll, rgba16_renderpass.get(), 0));
//			//}
//
//			//{
//			//	ShaderPrivate* shaders[] = {
//			//		fullscreen_vert,
//			//		box_frag
//			//	};
//			//	BlendOption bo;
//			//	bo.enable = true;
//			//	bo.src_color = BlendFactorOne;
//			//	bo.dst_color = BlendFactorOne;
//			//	bo.src_alpha = BlendFactorOne;
//			//	bo.dst_alpha = BlendFactorOne;
//			//	upsample_pipeline.reset(PipelinePrivate::create(device, shaders, post_pll, rgba16_renderpass.get(), 0, nullptr, nullptr, nullptr, { &bo, 1 }));
//			//}
//
//			//{
//			//	ShaderPrivate* shaders[] = {
//			//		fullscreen_vert,
//			//		ShaderPrivate::get(device, L"post/gamma.frag")
//			//	};
//			//	gamma_pipeline.reset(PipelinePrivate::create(device, shaders, post_pll, rgba8_renderpass.get(), 0));
//			//}
//		}
//
//		ArmatureDeformerPrivate::ArmatureDeformerPrivate(RenderPreferencesPrivate* preferences, MeshPrivate* mesh) :
//			mesh(mesh)
//		{
//			auto dsl = DescriptorSetLayoutPrivate::get(preferences->device, L"armature.dsl");
//			//poses_buffer.create(preferences->device, BufferUsageStorage, find_type(dsl->types, "mat4"), mesh->bones.size());
//			descriptorset.reset(new DescriptorSetPrivate(preferences->device->dsp.get(), dsl));
//			//descriptorset->set_buffer(0, 0, poses_buffer.buf.get());
//		}
//
//		void ArmatureDeformerPrivate::set_pose(uint id, const mat4& pose)
//		{
//			//auto dst = (mat4*)poses_buffer.mark_item(id);
//			//*dst = pose * mesh->bones[id]->offset_matrix;
//		}
//
//		CanvasPrivate::CanvasPrivate(RenderPreferencesPrivate* preferences) :
//			preferences(preferences)
//		{
//			line_buffer.create(device, BufferUsageVertex, 200000);
//			triangle_buffer.create(device, BufferUsageVertex, 1000);
//		}
// 
//		void CanvasPrivate::set_output(std::span<ImageViewPrivate*> views)
//		{
//			{
//				InstanceCB cb(device);
//
//				back_image.reset(new ImagePrivate(device, hdr ? Format_R16G16B16A16_SFLOAT : Format_B8G8R8A8_UNORM, output_size, 0, 1, SampleCount_1, ImageUsageSampled | ImageUsageAttachment));
//				cb->image_barrier(back_image.get(), { 0U, back_image->levels, 0U, 1U }, ImageLayoutUndefined, ImageLayoutShaderReadOnly);
//				back_framebuffers.resize(back_image->levels);
//				back_nearest_descriptorsets.resize(back_image->levels);
//				back_linear_descriptorsets.resize(back_image->levels);
//				for (auto lv = 0; lv < back_image->levels; lv++)
//				{
//					auto iv = back_image->views[lv].get();
//					back_framebuffers[lv].reset(new FramebufferPrivate(device, hdr ? preferences->rgba16_renderpass.get() : preferences->rgba8_renderpass.get(), { &iv, 1 }));
//					back_nearest_descriptorsets[lv].reset(new DescriptorSetPrivate(device->dsp.get(), post_dsl));
//					back_linear_descriptorsets[lv].reset(new DescriptorSetPrivate(device->dsp.get(), post_dsl));
//					back_nearest_descriptorsets[lv]->set_image(0, 0, iv, SamplerPrivate::get(device, FilterNearest, FilterNearest, false, AddressClampToEdge));
//					back_linear_descriptorsets[lv]->set_image(0, 0, iv, SamplerPrivate::get(device, FilterLinear, FilterLinear, false, AddressClampToEdge));
//				}
//
//				{
//					pickup_image.reset(new ImagePrivate(device, Format_R8G8B8A8_UNORM, output_size, 1, 1, SampleCount_1, ImageUsageTransferSrc | ImageUsageAttachment));
//					ImageViewPrivate* vs[] = {
//						pickup_image->views[0].get(),
//						depth_image->views[0].get()
//					};
//					pickup_framebuffer.reset(new FramebufferPrivate(device, preferences->pickup_renderpass.get(), vs));
//				}
//			}
//		}
//
//		uint CanvasPrivate::set_model_resource(int slot, ModelPrivate* mod, const std::string& name)
//		{
			//if (!ms->bones.empty())
			//{
			//	mrm->weight_buffer.create(device, BufferUsageVertex, ms->positions.size());
			//	std::vector<std::vector<Bone::Weight>> weights;
			//	weights.resize(ms->positions.size());
			//	for (auto j = 0; j < ms->bones.size(); j++)
			//	{
			//		auto& b = ms->bones[j];
			//		for (auto& w : b->weights)
			//			weights[w.vid].emplace_back(j, w.w);
			//	}
			//	std::vector<MeshWeight> mesh_weights;
			//	mesh_weights.resize(weights.size());
			//	for (auto j = 0; j < weights.size(); j++)
			//	{
			//		auto& src = weights[j];
			//		auto& dst = mesh_weights[j];
			//		for (auto k = 0; k < 4; k++)
			//		{
			//			if (k < src.size())
			//			{
			//				dst.ids[k] = src[k].vid;
			//				dst.weights[k] = src[k].w;
			//			}
			//			else
			//				dst.ids[k] = -1;
			//		}
			//	}
			//	mrm->weight_buffer.push(mesh_weights.size(), mesh_weights.data());
			//	mrm->weight_buffer.upload((CommandBufferPrivate*)cb.get());
			//}
//		}
//
//		/*
//		inline void path_arc(std::vector<vec2>& points, const vec2& center, float radius, float a1, float a2, uint lod = 0)
//		{
//			int a = pieces_num * a1;
//			int b = pieces_num * a2;
//			lod += 1;
//			for (; a <= b; a += lod)
//				points.push_back(center + pieces[a % pieces_num] * radius);
//		}
//
//		// roundness: a RT b LB
//		inline void path_rect(std::vector<vec2>& points, const vec2& pos, const vec2& size, const vec4& roundness = vec4(0.f), uint lod = 0)
//		{
//			if (roundness[0] > 0.f)
//				path_arc(points, pos + vec2(roundness[0]), roundness[0], 0.5f, 0.75f, lod);
//			else
//				points.push_back(pos);
//			if (roundness[1] > 0.f)
//				path_arc(points, pos + vec2(size.x - roundness[1], roundness[1]), roundness[1], 0.75f, 1.f, lod);
//			else
//				points.push_back(pos + vec2(size.x, 0.f));
//			if (roundness[2] > 0.f)
//				path_arc(points, pos + size - vec2(roundness[2]), roundness[2], 0.f, 0.25f, lod);
//			else
//				points.push_back(pos + size);
//			if (roundness[3] > 0.f)
//				path_arc(points, pos + vec2(roundness[3], size.y - roundness[3]), roundness[3], 0.25f, 0.5f, lod);
//			else
//				points.push_back(pos + vec2(0.f, size.y));
//		}
//
//		inline void path_circle(std::vector<vec2>& points, const vec2& center, float radius, uint lod = 0)
//		{
//			path_arc(points, center, radius, 0.f, 1.f, lod);
//		}
//
//		inline void path_bezier(std::vector<vec2>& points, const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4, uint level = 0)
//		{
//			auto dx = p4.x - p1.x;
//			auto dy = p4.y - p1.y;
//			auto d2 = ((p2.x - p4.x) * dy - (p2.y - p4.y) * dx);
//			auto d3 = ((p3.x - p4.x) * dy - (p3.y - p4.y) * dx);
//			d2 = (d2 >= 0) ? d2 : -d2;
//			d3 = (d3 >= 0) ? d3 : -d3;
//			if ((d2 + d3) * (d2 + d3) < 1.25f * (dx * dx + dy * dy))
//			{
//				if (points.empty())
//					points.push_back(p1);
//				points.push_back(p4);
//			}
//			else if (level < 10)
//			{
//				auto p12 = (p1 + p2) * 0.5f;
//				auto p23 = (p2 + p3) * 0.5f;
//				auto p34 = (p3 + p4) * 0.5f;
//				auto p123 = (p12 + p23) * 0.5f;
//				auto p234 = (p23 + p34) * 0.5f;
//				auto p1234 = (p123 + p234) * 0.5f;
//
//				path_bezier(points, p1, p12, p123, p1234, level + 1);
//				path_bezier(points, p1234, p234, p34, p4, level + 1);
//			}
//		}
//		*/
//
//		void CanvasPrivate::draw_image(uint res_id, uint tile_id, const vec2& pos, const vec2& size, const mat2& axes, const vec2& uv0, const vec2& uv1, const cvec4& tint_col)
//		{
//			if (res_id >= element_resources.size())
//				res_id = 0;
//			auto& res = element_resources[res_id];
//			auto _uv0 = uv0;
//			auto _uv1 = uv1;
//			if (res.ia)
//			{
//				auto tile = res.ia->tiles[tile_id].get();
//				auto tuv = tile->uv;
//				auto tuv0 = vec2(tuv.x, tuv.y);
//				auto tuv1 = vec2(tuv.z, tuv.w);
//				_uv0 = mix(tuv0, tuv1, uv0);
//				_uv1 = mix(tuv0, tuv1, uv1);
//			}
//
//			auto cmd = add_draw_element_cmd(res_id);
//
//			auto vtx_cnt = cmd->vertices_count;
//
//			add_vtx(pos, _uv0, tint_col);
//			add_vtx(pos + size.x * axes[0], vec2(_uv1.x, _uv0.y), tint_col);
//			add_vtx(pos + size.x * axes[0] + size.y * axes[1], _uv1, tint_col);
//			add_vtx(pos + size.y * axes[1], vec2(_uv0.x, _uv1.y), tint_col);
//			add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 2); add_idx(vtx_cnt + 1); add_idx(vtx_cnt + 0); add_idx(vtx_cnt + 3); add_idx(vtx_cnt + 2);
//		}
//
//		static void draw_frustum(CanvasPrivate* thiz, vec3* ps, const cvec4& col0, const cvec4& col1)
//		{
//			std::vector<Triangle> triangles;
//			std::vector<Line> lines;
//
//			triangles.push_back({ { ps[0], col0 }, { ps[3], col0 }, { ps[1], col0 } });
//			triangles.push_back({ { ps[1], col0 }, { ps[3], col0 }, { ps[2], col0 } });
//			triangles.push_back({ { ps[4], col0 }, { ps[5], col0 }, { ps[6], col0 } });
//			triangles.push_back({ { ps[4], col0 }, { ps[6], col0 }, { ps[7], col0 } });
//			triangles.push_back({ { ps[5], col0 }, { ps[4], col0 }, { ps[0], col0 } });
//			triangles.push_back({ { ps[5], col0 }, { ps[0], col0 }, { ps[1], col0 } });
//			triangles.push_back({ { ps[7], col0 }, { ps[6], col0 }, { ps[2], col0 } });
//			triangles.push_back({ { ps[7], col0 }, { ps[2], col0 }, { ps[3], col0 } });
//			triangles.push_back({ { ps[0], col0 }, { ps[4], col0 }, { ps[7], col0 } });
//			triangles.push_back({ { ps[0], col0 }, { ps[7], col0 }, { ps[3], col0 } });
//			triangles.push_back({ { ps[5], col0 }, { ps[1], col0 }, { ps[2], col0 } });
//			triangles.push_back({ { ps[5], col0 }, { ps[2], col0 }, { ps[6], col0 } });
//
//			lines.push_back({ { ps[0], col1 }, { ps[1], col1 } });
//			lines.push_back({ { ps[1], col1 }, { ps[2], col1 } });
//			lines.push_back({ { ps[2], col1 }, { ps[3], col1 } });
//			lines.push_back({ { ps[3], col1 }, { ps[0], col1 } });
//			lines.push_back({ { ps[4], col1 }, { ps[5], col1 } });
//			lines.push_back({ { ps[5], col1 }, { ps[6], col1 } });
//			lines.push_back({ { ps[6], col1 }, { ps[7], col1 } });
//			lines.push_back({ { ps[7], col1 }, { ps[4], col1 } });
//			lines.push_back({ { ps[0], col1 }, { ps[4], col1 } });
//			lines.push_back({ { ps[1], col1 }, { ps[5], col1 } });
//			lines.push_back({ { ps[2], col1 }, { ps[6], col1 } });
//			lines.push_back({ { ps[3], col1 }, { ps[7], col1 } });
//
//			thiz->draw_triangles(triangles.size(), triangles.data());
//			thiz->draw_lines(lines.size(), lines.data());
//		}
//
//		void* CanvasPrivate::pickup(const vec2& p)
//		{
//			std::vector<void*> userdatas;
//
//			{
//				InstanceCB cb(preferences->device);
//
//				auto mesh_off = 0;
//				auto terr_off = 0;
//
//				vec4 cvs[] = {
//					vec4(0.f, 0.f, 0.f, 0.f),
//					vec4(1.f, 0.f, 0.f, 0.f)
//				};
//				cb->begin_renderpass(nullptr, pickup_framebuffer.get(), cvs);
//				cb->set_viewport(curr_viewport);
//				cb->set_scissor(Rect(p.x - 1, p.y - 1, p.x + 1, p.y + 1));
//				if (!meshes.empty())
//				{
//					cb->bind_pipeline_layout(preferences->mesh_pipeline_layout);
//					//cb->bind_descriptor_set(S<"render_data"_h>, render_data_descriptorset.get());
//					//cb->bind_descriptor_set(S<"material"_h>, material_descriptorset.get());
//					for (auto& m : meshes)
//					{
//						userdatas.push_back(m.userdata);
//						//cb->push_constant_ht(S<"i"_h>, ivec4(userdatas.size(), 0, 0, 0));
//
//						auto mrm = m.res;
//						cb->bind_vertex_buffer(mrm->vertex_buffer.buf.get(), 0);
//						cb->bind_index_buffer(mrm->index_buffer.buf.get(), IndiceTypeUint);
//						if (m.deformer)
//						{
//							cb->bind_pipeline(preferences->mesh_armature_pickup_pipeline);
//							cb->bind_vertex_buffer(mrm->weight_buffer.buf.get(), 1);
//							//cb->bind_descriptor_set(S<"armature"_h>, m.deformer->descriptorset.get());
//						}
//						else
//						{
//							cb->bind_pipeline(preferences->mesh_pickup_pipeline);
//							//cb->bind_descriptor_set(S<"mesh"_h>, mesh_descriptorset.get());
//						}
//						cb->draw_indexed(mrm->index_buffer.capacity, 0, 0, 1, (mesh_off << 16) + mrm->material_id);
//						mesh_off++;
//					}
//				}
//				if (!terrains.empty())
//				{
//					cb->bind_pipeline_layout(preferences->terrain_pipeline_layout);
//					//cb->bind_descriptor_set(S<"render_data"_h>, render_data_descriptorset.get());
//					//cb->bind_descriptor_set(S<"material"_h>, material_descriptorset.get());
//					for (auto& t : terrains)
//					{
//						userdatas.push_back(t.userdata);
//						//cb->push_constant_ht(S<"i"_h>, ivec4(userdatas.size(), 0, 0, 0));
//
//						cb->bind_pipeline(preferences->terrain_pickup_pipeline);
//						//cb->bind_descriptor_set(S<"render_data"_h>, render_data_descriptorset.get());
//						//cb->bind_descriptor_set(S<"terrain"_h>, terrain_descriptorset.get());
//						cb->draw(4, t.blocks.x * t.blocks.y, 0, terr_off << 16);
//						terr_off++;
//					}
//				}
//				cb->end_renderpass();
//			}
//
//			StagingBuffer stag(preferences->device, sizeof(cvec4), nullptr);
//			{
//				InstanceCB cb(preferences->device);
//
//				BufferImageCopy cpy;
//				cpy.image_offset = uvec2(p.x, p.y);
//				cpy.image_extent = uvec2(1);
//				cb->copy_image_to_buffer(pickup_image.get(), stag.get(), 1, &cpy);
//				cb->image_barrier(pickup_image.get(), {}, ImageLayoutTransferSrc, ImageLayoutAttachment);
//			}
//			auto pixel = *(cvec4*)stag.mapped;
//			auto index = (uint)pixel[0];
//			index += pixel[1] << 8;
//			index += pixel[2] << 16;
//			index += pixel[3] << 24;
//			if (index == 0)
//				return nullptr;
//			return userdatas[index - 1];
//		}
//
//		void CanvasPrivate::add_blur(const Rect& _range, uint radius)
//		{
//			auto range = Rect(
//				max(_range.a.x, 0.f),
//				max(_range.a.y, 0.f),
//				min(_range.b.x, (float)output_size.x),
//				min(_range.b.y, (float)output_size.y));
//			cmds.emplace_back(new CmdBlur(range, radius));
//		}
//
//		void CanvasPrivate::record(CommandBufferPrivate* cb, uint image_index)
//		{
//			for (auto& p : passes)
//			{
//				switch (p.type)
//				{
//				case Pass3D:
//				{
//					std::vector<std::pair<MeshInfo*, uint>> outline_meshes;
//					std::vector<std::pair<TerrainInfo*, uint>> outline_terrains;
//
//					if (!outline_meshes.empty() || !outline_terrains.empty())
//					{
//						auto cv = vec4(0.f);
//						cb->begin_renderpass(hdr_image ? preferences->rgba16c_renderpass.get() : preferences->rgba8c_renderpass.get(), back_framebuffers[0].get(), &cv);
//						if (!outline_meshes.empty())
//						{
//							cb->bind_pipeline_layout(preferences->mesh_pipeline_layout);
//							//cb->bind_descriptor_set(S<"render_data"_h>, render_data_descriptorset.get());
//							//cb->push_constant_ht(S<"f"_h>, vec4(1.f, 1.f, 0.f, 1.f));
//							for (auto& mi : outline_meshes)
//							{
//								auto& m = *mi.first;
//								auto mrm = m.res;
//								auto mat = material_resources[mrm->material_id].get();
//								cb->bind_vertex_buffer(mrm->vertex_buffer.buf.get(), 0);
//								cb->bind_index_buffer(mrm->index_buffer.buf.get(), IndiceTypeUint);
//								if (m.deformer)
//								{
//									cb->bind_vertex_buffer(mrm->weight_buffer.buf.get(), 1);
//									//cb->bind_descriptor_set(S<"armature"_h>, m.deformer->descriptorset.get());
//								}
//								else
//									//cb->bind_descriptor_set(S<"mesh"_h>, mesh_descriptorset.get());
//								if (m.deformer)
//									cb->bind_pipeline(preferences->mesh_armature_outline_pipeline);
//								else
//									cb->bind_pipeline(preferences->mesh_outline_pipeline);
//								cb->draw_indexed(mrm->index_buffer.capacity, 0, 0, 1, (mi.second << 16) + mrm->material_id);
//							}
//						}
//						if (!outline_terrains.empty())
//						{
//							cb->bind_pipeline_layout(preferences->terrain_pipeline_layout);
//							//cb->bind_descriptor_set(S<"render_data"_h>, render_data_descriptorset.get());
//							//cb->bind_descriptor_set(S<"terrain"_h>, terrain_descriptorset.get());
//							//cb->push_constant_ht(S<"f"_h>, vec4(1.f, 1.f, 0.f, 1.f));
//							for (auto& ti : outline_terrains)
//							{
//								auto& t = *ti.first;
//								cb->bind_pipeline(preferences->terrain_outline_pipeline);
//								cb->draw(4, t.blocks.x * t.blocks.y, 0, ti.second << 16);
//							}
//						}
//						cb->end_renderpass();
//
//						auto lvs = min(back_image->levels, 3U);
//						for (auto i = 0; i < lvs - 1; i++)
//						{
//							cb->image_barrier(back_image.get(), { (uint)i }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
//							cb->set_viewport(Rect(vec2(0.f), back_image->sizes[i + 1]));
//							cb->begin_renderpass(nullptr, back_framebuffers[i + 1].get());
//							cb->bind_pipeline(preferences->downsample_pipeline.get());
//							//cb->bind_descriptor_set(back_linear_descriptorsets[i].get(), 0);
//							//cb->push_constant_ht(S<"pxsz"_h>, 1.f / vec2(back_image->sizes[i]));
//							cb->draw(3, 1, 0, 0);
//							cb->end_renderpass();
//						}
//						for (auto i = lvs - 1; i > 0; i--)
//						{
//							cb->image_barrier(back_image.get(), { (uint)i }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
//							cb->set_viewport(Rect(vec2(0.f), back_image->sizes[i - 1]));
//							cb->begin_renderpass(nullptr, back_framebuffers[i - 1].get());
//							cb->bind_pipeline(preferences->upsample_pipeline.get());
//							//cb->bind_descriptor_set(back_linear_descriptorsets[i].get(), 0);
//							//cb->push_constant_ht(S<"pxsz"_h>, 1.f / vec2(back_image->sizes[(uint)i - 1]));
//							cb->draw(3, 1, 0, 0);
//							cb->end_renderpass();
//						}
//
//						cb->image_barrier(back_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutAttachment, AccessColorAttachmentWrite);
//						cb->set_viewport(curr_viewport);
//						cb->begin_renderpass(nullptr, back_framebuffers[0].get());
//						if (!outline_meshes.empty())
//						{
//							cb->bind_pipeline_layout(preferences->mesh_pipeline_layout);
//							//cb->bind_descriptor_set(S<"render_data"_h>, render_data_descriptorset.get());
//							//cb->push_constant_ht(S<"f"_h>, vec4(0.f, 0.f, 0.f, 0.f));
//							for (auto& mi : outline_meshes)
//							{
//								auto& m = *mi.first;
//								auto mrm = m.res;
//								auto mat = material_resources[mrm->material_id].get();
//								cb->bind_vertex_buffer(mrm->vertex_buffer.buf.get(), 0);
//								cb->bind_index_buffer(mrm->index_buffer.buf.get(), IndiceTypeUint);
//								if (m.deformer)
//								{
//									cb->bind_vertex_buffer(mrm->weight_buffer.buf.get(), 1);
//									//cb->bind_descriptor_set(S<"armature"_h>, m.deformer->descriptorset.get());
//								}
//								else
//									//cb->bind_descriptor_set(S<"mesh"_h>, mesh_descriptorset.get());
//								if (m.deformer)
//									cb->bind_pipeline(preferences->mesh_armature_outline_pipeline);
//								else
//									cb->bind_pipeline(preferences->mesh_outline_pipeline);
//								cb->draw_indexed(mrm->index_buffer.capacity, 0, 0, 1, (mi.second << 16) + mrm->material_id);
//							}
//						}
//						if (!outline_terrains.empty())
//						{
//							cb->bind_pipeline_layout(preferences->terrain_pipeline_layout);
//							//cb->bind_descriptor_set(S<"render_data"_h>, render_data_descriptorset.get());
//							//cb->bind_descriptor_set(S<"terrain"_h>, terrain_descriptorset.get());
//							//cb->push_constant_ht(S<"f"_h>, vec4(0.f, 0.f, 0.f, 0.f));
//							for (auto& ti : outline_terrains)
//							{
//								auto& t = *ti.first;
//								cb->bind_pipeline(preferences->terrain_outline_pipeline);
//								cb->draw(4, t.blocks.x * t.blocks.y, 0, ti.second << 16);
//							}
//						}
//						cb->end_renderpass();
//					}
//
//					cb->set_viewport(Rect(0.f, 0.f, output_size.x, output_size.y));
//					cb->set_scissor(Rect(0.f, 0.f, output_size.x, output_size.y));
//				}
//					break;
//				case PassLines:
//				{
//					if (line_off == 0)
//						line_buffer.upload(cb);
//					cb->image_barrier(dst, {}, ImageLayoutShaderReadOnly, ImageLayoutAttachment, AccessColorAttachmentWrite);
//					cb->set_viewport(curr_viewport);
//					cb->set_scissor(curr_viewport);
//					cb->begin_renderpass(nullptr, mesh_framebuffers[hdr_image ? 0 : image_index].get());
//					cb->bind_vertex_buffer(line_buffer.buf.get(), 0);
//					cb->bind_pipeline(preferences->line_pipeline.get());
//					//cb->push_constant_ht(S<"proj_view"_h>, proj_view_matrix);
//					for (auto& i : p.cmd_ids)
//					{
//						auto& cmd = cmds[i];
//						switch (cmd->type)
//						{
//						case Cmd::DrawLines:
//						{
//							auto c = (CmdDrawLines*)cmd.get();
//							cb->draw(c->count * 2, 1, line_off, 0);
//							line_off += c->count * 2;
//						}
//							break;
//						}
//					}
//					cb->end_renderpass();
//					cb->set_viewport(Rect(0.f, 0.f, output_size.x, output_size.y));
//					cb->set_scissor(Rect(0.f, 0.f, output_size.x, output_size.y));
//				}
//					break;
//				case PassTriangles:
//				{
//					if (tri_off == 0)
//						triangle_buffer.upload(cb);
//					cb->image_barrier(dst, {}, ImageLayoutShaderReadOnly, ImageLayoutAttachment, AccessColorAttachmentWrite);
//					cb->set_viewport(curr_viewport);
//					cb->set_scissor(curr_viewport);
//					cb->begin_renderpass(nullptr, mesh_framebuffers[hdr_image ? 0 : image_index].get());
//					cb->bind_vertex_buffer(triangle_buffer.buf.get(), 0);
//					cb->bind_pipeline(preferences->triangle_pipeline.get());
//					//cb->push_constant_ht(S<"proj_view"_h>, proj_view_matrix);
//					for (auto& i : p.cmd_ids)
//					{
//						auto& cmd = cmds[i];
//						switch (cmd->type)
//						{
//						case Cmd::DrawTriangles:
//						{
//							auto c = (CmdDrawTriangles*)cmd.get();
//							cb->draw(c->count * 3, 1, tri_off, 0);
//							tri_off += c->count * 3;
//						}
//							break;
//						}
//					}
//					cb->end_renderpass();
//					cb->set_viewport(Rect(0.f, 0.f, output_size.x, output_size.y));
//					cb->set_scissor(Rect(0.f, 0.f, output_size.x, output_size.y));
//				}
//					break;
//				case PassBlur:
//				{
//					auto c = (CmdBlur*)cmds[p.cmd_ids[0]].get();
//					auto blur_radius = clamp(c->radius, 0U, 10U);
//					auto blur_range = c->range;
//					auto blur_size = vec2(blur_range.b.x - blur_range.a.x, blur_range.b.y - blur_range.a.y);
//					if (blur_size.x < 1.f || blur_size.y < 1.f)
//						continue;
//
//					cb->image_barrier(dst, {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
//					cb->set_scissor(Rect(blur_range.a.x - blur_radius, blur_range.a.y - blur_radius,
//						blur_range.b.x + blur_radius, blur_range.b.y + blur_radius));
//					cb->begin_renderpass(nullptr, back_framebuffers[0].get());
//					cb->bind_pipeline(preferences->blurh_pipeline[blur_radius - 1].get());
//					//cb->bind_descriptor_set(dst_ds, 0);
//					//cb->push_constant_ht(S<"pxsz"_h>, 1.f / output_size.x);
//					cb->draw(3, 1, 0, 0);
//					cb->end_renderpass();
//
//					cb->image_barrier(back_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
//					cb->set_scissor(blur_range);
//					cb->begin_renderpass(nullptr, dst_fb);
//					cb->bind_pipeline(preferences->blurv_pipeline[blur_radius - 1].get());
//					//cb->bind_descriptor_set(back_nearest_descriptorsets[0].get(), 0);
//					//cb->push_constant_ht(S<"pxsz"_h>, 1.f / output_size.y);
//					cb->draw(3, 1, 0, 0);
//					cb->end_renderpass();
//				}
//					break;
//				case PassBloom:
//				{
//					cb->set_scissor(Rect(0.f, 0.f, output_size.x, output_size.y));
//
//					cb->set_viewport(Rect(0.f, 0.f, output_size.x, output_size.y));
//					cb->image_barrier(hdr_image.get(), {}, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
//					cb->begin_renderpass(nullptr, back_framebuffers[0].get());
//					cb->bind_pipeline(preferences->filter_bright_pipeline.get());
//					//cb->bind_descriptor_set(hdr_descriptorset.get(), 0);
//					cb->draw(3, 1, 0, 0);
//					cb->end_renderpass();
//
//					for (auto i = 0; i < back_image->levels - 1; i++)
//					{
//						cb->image_barrier(back_image.get(), { (uint)i }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
//						cb->set_viewport(Rect(vec2(0.f), back_image->sizes[i + 1]));
//						cb->begin_renderpass(nullptr, back_framebuffers[i + 1].get());
//						cb->bind_pipeline(preferences->downsample_pipeline.get());
//						//cb->bind_descriptor_set(back_linear_descriptorsets[i].get(), 0);
//						//cb->push_constant_ht(S<"pxsz"_h>, 1.f / vec2(back_image->sizes[i]));
//						cb->draw(3, 1, 0, 0);
//						cb->end_renderpass();
//					}
//					for (auto i = back_image->levels - 1; i > 1; i--)
//					{
//						cb->image_barrier(back_image.get(), { (uint)i }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
//						cb->set_viewport(Rect(vec2(0.f), back_image->sizes[i - 1]));
//						cb->begin_renderpass(nullptr, back_framebuffers[i - 1].get());
//						cb->bind_pipeline(preferences->upsample_pipeline.get());
//						//cb->bind_descriptor_set(back_linear_descriptorsets[i].get(), 0);
//						//cb->push_constant_ht(S<"pxsz"_h>, 1.f / vec2(back_image->sizes[(uint)i - 1]));
//						cb->draw(3, 1, 0, 0);
//						cb->end_renderpass();
//					}
//					cb->image_barrier(back_image.get(), { 1U }, ImageLayoutShaderReadOnly, ImageLayoutShaderReadOnly, AccessColorAttachmentWrite);
//					cb->set_viewport(Rect(0.f, 0.f, output_size.x, output_size.y));
//					cb->begin_renderpass(nullptr, hdr_framebuffer.get());
//					cb->bind_pipeline(preferences->upsample_pipeline.get());
//					//cb->bind_descriptor_set(back_linear_descriptorsets[1].get(), 0);
//					//cb->push_constant_ht(S<"pxsz"_h>, 1.f / vec2(output_size));
//					cb->draw(3, 1, 0, 0);
//					cb->end_renderpass();
//				}
//					break;
//				}
//			}
// 
//			cb->image_barrier(output_imageviews[image_index]->image, {}, ImageLayoutShaderReadOnly, ImageLayoutPresent);
//		}
//	}
//}
