#include <flame/graphics/canvas.h>

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;
		struct BufferPrivate;
		struct ImagePrivate;
		struct ImageViewPrivate;
		struct ImageAtlasPrivate;
		struct FontAtlasPrivate;
		struct MaterialPrivate;
		struct ModelPrivate;
		struct RenderpassPrivate;
		struct FramebufferPrivate;
		struct DescriptorSetPrivate;
		struct CommandBufferPrivate;

		struct RenderPreferencesPrivate : RenderPreferences
		{
			DevicePrivate* device;

			bool hdr;
			bool msaa_3d;

			std::unique_ptr<SamplerPrivate> shadow_sampler;
			std::unique_ptr<RenderpassPrivate> image1_8_renderpass;
			std::unique_ptr<RenderpassPrivate> image1_16_renderpass;
			std::unique_ptr<RenderpassPrivate> image1_r16_renderpass;
			std::unique_ptr<RenderpassPrivate> mesh_renderpass;
			std::unique_ptr<RenderpassPrivate> depth_renderpass;
			std::unique_ptr<DescriptorSetLayoutPrivate> element_descriptorsetlayout;
			std::unique_ptr<DescriptorSetLayoutPrivate> mesh_descriptorsetlayout;
			std::unique_ptr<DescriptorSetLayoutPrivate> armature_descriptorsetlayout;
			std::unique_ptr<DescriptorSetLayoutPrivate> material_descriptorsetlayout;
			std::unique_ptr<DescriptorSetLayoutPrivate> light_descriptorsetlayout;
			std::unique_ptr<DescriptorSetLayoutPrivate> render_data_descriptorsetlayout;
			std::unique_ptr<DescriptorSetLayoutPrivate> terrain_descriptorsetlayout;
			std::unique_ptr<DescriptorSetLayoutPrivate> sampler1_descriptorsetlayout;
			std::unique_ptr<PipelineLayoutPrivate> element_pipelinelayout;
			std::unique_ptr<PipelineLayoutPrivate> mesh_pipelinelayout;
			std::unique_ptr<PipelineLayoutPrivate> terrain_pipelinelayout;
			std::unique_ptr<PipelineLayoutPrivate> depth_pipelinelayout;
			std::unique_ptr<PipelineLayoutPrivate> sampler1_pc0_pipelinelayout;
			std::unique_ptr<PipelineLayoutPrivate> sampler1_pc4_pipelinelayout;
			std::unique_ptr<PipelineLayoutPrivate> sampler1_pc8_pipelinelayout;
			std::unique_ptr<PipelineLayoutPrivate> image1_pc0_pipelinelayout;
			std::unique_ptr<PipelineLayoutPrivate> image2_pc0_pipelinelayout;
			std::unique_ptr<PipelineLayoutPrivate> line_pipelinelayout;
			std::unique_ptr<ShaderPrivate> element_vert;
			std::unique_ptr<ShaderPrivate> element_frag;
			std::unique_ptr<ShaderPrivate> mesh_vert;
			std::unique_ptr<ShaderPrivate> mesh_armature_vert;
			std::unique_ptr<ShaderPrivate> mesh_frag;
			std::unique_ptr<ShaderPrivate> terrain_vert;
			std::unique_ptr<ShaderPrivate> terrain_tesc;
			std::unique_ptr<ShaderPrivate> terrain_tese;
			std::unique_ptr<ShaderPrivate> terrain_frag;
			std::unique_ptr<ShaderPrivate> depth_vert;
			std::unique_ptr<ShaderPrivate> depth_armature_vert;
			std::unique_ptr<ShaderPrivate> depth_frag;
			std::unique_ptr<ShaderPrivate> line3_vert;
			std::unique_ptr<ShaderPrivate> line3_frag;
			std::unique_ptr<ShaderPrivate> fullscreen_vert;
			std::unique_ptr<ShaderPrivate> blurh_frag[10];
			std::unique_ptr<ShaderPrivate> blurv_frag[10];
			std::unique_ptr<ShaderPrivate> blurh_depth_frag;
			std::unique_ptr<ShaderPrivate> blurv_depth_frag;
			std::unique_ptr<ShaderPrivate> blit_frag;
			std::unique_ptr<ShaderPrivate> filter_bright_frag;
			std::unique_ptr<ShaderPrivate> box_frag;
			std::unique_ptr<ShaderPrivate> gamma_frag;
			std::unique_ptr<PipelinePrivate> element_pipeline;
			std::unique_ptr<PipelinePrivate> mesh_pipeline;
			std::unique_ptr<PipelinePrivate> mesh_armature_pipeline;
			std::unique_ptr<PipelinePrivate> terrain_pipeline;
			std::unique_ptr<PipelinePrivate> depth_pipeline;
			std::unique_ptr<PipelinePrivate> depth_armature_pipeline;
			std::unique_ptr<PipelinePrivate> line3_pipeline;
			std::unique_ptr<PipelinePrivate> blurh_pipeline[10];
			std::unique_ptr<PipelinePrivate> blurv_pipeline[10];
			std::unique_ptr<PipelinePrivate> blurh_depth_pipeline;
			std::unique_ptr<PipelinePrivate> blurv_depth_pipeline;
			std::unique_ptr<PipelinePrivate> blit_8_pipeline;
			std::unique_ptr<PipelinePrivate> blit_16_pipeline;
			std::unique_ptr<PipelinePrivate> filter_bright_pipeline;
			std::unique_ptr<PipelinePrivate> downsample_pipeline;
			std::unique_ptr<PipelinePrivate> upsample_pipeline;
			std::unique_ptr<PipelinePrivate> gamma_pipeline;

			RenderType terrain_render = RenderNormal;

			RenderPreferencesPrivate(DevicePrivate* device, bool hdr, bool msaa_3d);

			void make_terrain_pipeline();

			void set_terrain_render(RenderType type) override;
		};

		template <class T, BufferUsageFlags U>
		struct TBuffer
		{
			DevicePrivate* d = nullptr;
			uint count = 0;
			std::unique_ptr<BufferPrivate> buf;
			std::unique_ptr<BufferPrivate> stg;
			T* beg = nullptr;
			T* end = nullptr;

			void _build()
			{
				T* temp = nullptr;
				auto n = 0;
				if (end)
				{
					n = stg_num();
					temp = new T[n];
					memcpy(temp, stg->mapped, sizeof(T) * n);
				}
				buf.reset(new BufferPrivate(d, count * sizeof(T), BufferUsageTransferDst | U, MemoryPropertyDevice));
				stg.reset(new BufferPrivate(d, buf->size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
				stg->map();
				beg = (T*)stg->mapped;
				end = beg;
				if (temp)
				{
					push(n, temp);
					delete[]temp;
				}
			}

			void create(DevicePrivate* _d, uint _count)
			{
				d = _d;
				count = _count;
				_build();
			}

			void stg_rewind()
			{
				end = (T*)stg->mapped;
			}

			uint stg_num()
			{
				return end - stg->mapped;
			}

			void push(const T& t)
			{
				auto n = stg_num();
				if (n > count)
				{
					count *= 2;
					_build();
				}
				*end = t;
				end++;
			}

			void push(uint cnt, const T* p)
			{
				auto n = stg_num();
				if (n + cnt > count)
				{
					count = (n + cnt) * 2;
					_build();
				}
				memcpy(end, p, sizeof(T) * cnt);
				end += cnt;
			}

			void push(uint off, const T& t)
			{
				beg[off] = t;
			}

			void upload(CommandBufferPrivate* cb)
			{
				BufferCopy cpy;
				cpy.size = (char*)end - stg->mapped;
				cb->copy_buffer(stg.get(), buf.get(), { &cpy, 1 });
			}

			void upload(CommandBufferPrivate* cb, uint off, uint n)
			{
				BufferCopy cpy;
				cpy.src_off = cpy.dst_off = off * sizeof(T);
				cpy.size = n * sizeof(T);
				cb->copy_buffer(stg.get(), buf.get(), { &cpy, 1 });
			}

			void barrier(CommandBufferPrivate* cb)
			{
				cb->buffer_barrier(buf.get(), AccessTransferWrite, AccessVertexAttributeRead);
			}
		};

		struct ArmatureDeformerPrivate : ArmatureDeformer
		{
			MeshPrivate* mesh;
			TBuffer<Mat4f, BufferUsageStorage> poses_buffer;
			std::unique_ptr<DescriptorSetPrivate> descriptorset;
			Vec2i dirty_range = Vec2i(0);

			ArmatureDeformerPrivate(RenderPreferencesPrivate* preferences, MeshPrivate* mesh);
			void release() override { delete this; }
			void set_pose(uint id, const Mat4f& pose) override;
		};

		const auto msaa_sample_count = SampleCount_8;
		const auto shadow_map_size = Vec2u(2048);

		struct ElementVertex
		{
			Vec2f position;
			Vec2f uv;
			Vec4c color;
		};

		struct MeshVertex
		{
			Vec3f position;
			Vec2f uv;
			Vec3f normal;
		};

		struct MeshWeight
		{
			Vec4i ids;
			Vec4f weights;
		};

		struct ElementResourceSlot
		{
			std::string name;
			ImageViewPrivate* iv;
			ImageAtlasPrivate* ia;
			FontAtlasPrivate* fa;
		};

		struct TextureResourceSlot
		{
			std::string name;
			ImageViewPrivate* iv;
		};

		struct MaterialResourceSlot
		{
			std::string name;
			MaterialPrivate* material;
			std::vector<std::pair<uint, std::unique_ptr<ImagePrivate>>> textures;
		};

		struct ModelResourceSlot
		{
			struct Mesh
			{
				TBuffer<MeshVertex, BufferUsageVertex> vertex_buffer;
				TBuffer<MeshWeight, BufferUsageVertex> weight_buffer;
				TBuffer<uint, BufferUsageIndex> index_buffer;
				uint material_index;
			};

			std::string name;
			ModelPrivate* model;
			std::vector<uint> materials;
			std::vector<std::unique_ptr<Mesh>> meshes;
		};

		struct RenderDataS
		{
			float fovy;
			float aspect;
			float zNear;
			float zFar;
			Vec3f camera_coord;
			float dummy1;
			Vec4f frustum_planes[6];

			Vec2f fb_size;
			float shadow_distance;
			uint csm_levels;
			Vec4f dummy3[3];

			Mat4f view_inv;
			Mat4f view;
			Mat4f proj;
			Mat4f proj_view;
		};

		struct MeshMatrixS
		{
			Mat4f transform;
			Mat4f normal_matrix;
		};

		struct MaterialInfoS
		{
			Vec4f color;
			float metallic;
			float roughness;
			float alpha_test;
			float dummy1;
			int color_map_index = -1;
			int metallic_roughness_ao_map_index = -1;
			int normal_hegiht_map_index = -1;
			int dummy2;
		};

		struct LightIndicesS
		{
			uint directional_lights_count;
			uint point_lights_count;
			uint point_light_indices[1022];
		};

		struct DirectionalLightInfoS
		{
			Vec3f dir;
			float distance;
			Vec3f color;
			int dummy1;

			int shadow_map_index;
			float dummy2;
			Vec2f dummy3;
			Vec4f dummy4;
			Mat4f shadow_matrices[4];
		};

		struct PointLightInfoS
		{
			Vec3f coord;
			float distance;
			Vec3f color;
			int shadow_map_index;
		};

		struct TerrainInfoS
		{
			Vec3f coord;
			float dummy1;

			Vec2u blocks;
			Vec2f dummy2;

			Vec3f scale;
			float tess_levels;

			uint height_tex_id;
			uint normal_tex_id;
			uint color_tex_id;
			float dummy3;
		};

		struct Cmd
		{
			enum Type
			{
				DrawElement,
				DrawMesh,
				DrawTerrain,
				DrawLine3,
				SetScissor,
				Blur,
				Bloom
			};

			Type type;

			Cmd(Type t) : type(t) {}
			virtual ~Cmd() {}
		};

		struct CmdDrawElement : Cmd
		{
			uint id;
			uint vertices_count = 0;
			uint indices_count = 0;

			CmdDrawElement(uint _id) : Cmd(DrawElement) { id = _id; }
		};

		struct CmdDrawMesh : Cmd
		{
			std::vector<std::tuple<uint, ModelResourceSlot::Mesh*, bool, ArmatureDeformerPrivate*>> meshes;

			CmdDrawMesh() : Cmd(DrawMesh) {}
		};

		struct CmdDrawTerrain : Cmd
		{
			uint idx;

			CmdDrawTerrain() : Cmd(DrawTerrain) {}
		};

		struct CmdDrawLine3 : Cmd
		{
			uint count;

			CmdDrawLine3() : Cmd(DrawLine3) {}
		};

		struct CmdSetScissor : Cmd
		{
			Vec4f scissor;

			CmdSetScissor(const Vec4f& _scissor) : Cmd(SetScissor) { scissor = _scissor; }
		};

		struct CmdBlur : Cmd
		{
			Vec4f range;
			uint radius;

			CmdBlur(const Vec4f& _range, uint _radius) : Cmd(Blur) { range = _range; radius = _radius; }
		};

		struct CmdBloom : Cmd
		{
			CmdBloom() : Cmd(Bloom) {}
		};

		struct CanvasBridge : Canvas
		{
			void set_output(uint views_count, ImageView* const* views) override;

			int find_element_resource(const char* name) override;
			uint set_element_resource(int slot, ElementResource r, const char* name) override;
			int find_texture_resource(const char* name) override;
			uint set_texture_resource(int slot, ImageView* iv, Sampler* sp, const char* name) override;
			int find_material_resource(const char* name) override;
			uint set_material_resource(int slot, Material* mat, const char* name) override;
			int find_model_resource(const char* name) override;
			uint set_model_resource(int slot, Model* mod, const char* name) override;

			void record(CommandBuffer* cb, uint image_index) override;
		};

		struct CanvasPrivate : CanvasBridge
		{
			RenderPreferencesPrivate* preferences;

			Vec4c clear_color = Vec4c(0, 0, 0, 255);

			std::unique_ptr<ImagePrivate> white_image;
			std::vector < ElementResourceSlot > element_resources;
			std::vector<TextureResourceSlot> texture_resources;
			std::vector<std::unique_ptr<MaterialResourceSlot>> material_resources;
			std::vector<std::unique_ptr<ModelResourceSlot>> model_resources;

			TBuffer<ElementVertex, BufferUsageVertex> element_vertex_buffer;
			TBuffer<uint, BufferUsageIndex> element_index_buffer;
			std::unique_ptr<DescriptorSetPrivate> element_descriptorset;

			TBuffer<RenderDataS, BufferUsageUniform> render_data_buffer;
			std::unique_ptr<DescriptorSetPrivate> render_data_descriptorset;

			TBuffer<MeshMatrixS, BufferUsageStorage> mesh_matrix_buffer;
			std::unique_ptr<DescriptorSetPrivate> mesh_descriptorset;

			TBuffer<MaterialInfoS, BufferUsageStorage> material_info_buffer;
			std::unique_ptr<DescriptorSetPrivate> material_descriptorset;

			std::unique_ptr<ImagePrivate> shadow_depth_image;
			std::unique_ptr<ImagePrivate> shadow_blur_pingpong_image;
			std::unique_ptr<FramebufferPrivate> shadow_blur_pingpong_image_framebuffer;
			std::unique_ptr<DescriptorSetPrivate> shadow_blur_pingpong_image_descriptorset;

			TBuffer<LightIndicesS, BufferUsageStorage> light_indices_buffer;

			TBuffer<DirectionalLightInfoS, BufferUsageStorage> directional_light_info_buffer;
			std::vector<std::unique_ptr<ImagePrivate>> directional_light_shadow_maps;
			std::vector<std::unique_ptr<FramebufferPrivate>> directional_light_shadow_map_depth_framebuffers;
			std::vector<std::unique_ptr<FramebufferPrivate>> directional_light_shadow_map_framebuffers;
			std::vector<std::unique_ptr<DescriptorSetPrivate>> directional_light_shadow_map_descriptorsets;
			uint used_directional_light_shadow_maps_count = 0;

			TBuffer<PointLightInfoS, BufferUsageStorage> point_light_info_buffer;
			std::vector<std::unique_ptr<ImagePrivate>> point_light_shadow_maps;
			std::vector<std::unique_ptr<FramebufferPrivate>> point_light_shadow_map_depth_framebuffers;
			std::vector<std::unique_ptr<FramebufferPrivate>> point_light_shadow_map_framebuffers;
			std::vector<std::unique_ptr<DescriptorSetPrivate>> point_light_shadow_map_descriptorsets;
			uint used_point_light_shadow_maps_count = 0;

			std::unique_ptr<DescriptorSetPrivate> light_descriptorset;

			TBuffer<TerrainInfoS, BufferUsageStorage> terrain_info_buffer;
			std::unique_ptr<DescriptorSetPrivate> terrain_descriptorset;

			std::vector<ImageViewPrivate*> output_imageviews;
			std::vector<std::unique_ptr<FramebufferPrivate>> output_framebuffers;
			std::vector<std::unique_ptr<DescriptorSetPrivate>> output_descriptorsets;

			std::unique_ptr<ImagePrivate> hdr_image;
			std::unique_ptr<FramebufferPrivate> hdr_framebuffer;
			std::unique_ptr<DescriptorSetPrivate> hdr_descriptorset;

			std::unique_ptr<ImagePrivate> depth_image;

			std::unique_ptr<ImagePrivate> msaa_image;
			std::unique_ptr<ImagePrivate> msaa_resolve_image;
			std::unique_ptr<DescriptorSetPrivate> msaa_descriptorset;

			std::vector<std::unique_ptr<FramebufferPrivate>> mesh_framebuffers;

			std::unique_ptr<ImagePrivate> back_image;
			std::vector<std::unique_ptr<FramebufferPrivate>> back_framebuffers;
			std::vector<std::unique_ptr<DescriptorSetPrivate>> back_nearest_descriptorsets;
			std::vector<std::unique_ptr<DescriptorSetPrivate>> back_linear_descriptorsets;

			std::vector<std::vector<Vec2f>> paths;

			TBuffer<Line3, BufferUsageVertex> line3_buffer;

			std::vector<std::unique_ptr<Cmd>> cmds;
			CmdDrawElement* last_element_cmd = nullptr;
			CmdDrawMesh* last_mesh_cmd = nullptr;
			CmdDrawLine3* last_line3_cmd = nullptr;

			Vec2u output_size;
			Vec4f curr_scissor;

			CanvasPrivate(RenderPreferencesPrivate* preferences);

			void release() override { delete this; }

			RenderPreferences* get_preferences() const override { return preferences; };

			Vec4c get_clear_color() const override { return clear_color; }
			void set_clear_color(const Vec4c& color) override { clear_color = color; }

			ImageView* get_output(uint idx) const override { return output_imageviews.empty() ? nullptr : (ImageView*)output_imageviews[idx]; }
			void set_output(std::span<ImageViewPrivate*> views);

			ElementResource get_element_resource(uint slot) override;
			int find_element_resource(const std::string& name);
			uint set_element_resource(int slot, ElementResource r, const std::string& name);

			ImageView* get_texture_resource(uint slot) override;
			int find_texture_resource(const std::string& name);
			uint set_texture_resource(int slot, ImageViewPrivate* iv, SamplerPrivate* sp, const std::string& name);

			Material* get_material_resource(uint slot) override;
			int find_material_resource(const std::string& name);
			uint set_material_resource(int slot, MaterialPrivate* mat, const std::string& name);

			Model* get_model_resource(uint slot) override;
			int find_model_resource(const std::string& name);
			uint set_model_resource(int slot, ModelPrivate* mat, const std::string& name);

			void add_draw_element_cmd(uint id);
			void add_vtx(const Vec2f& position, const Vec2f& uv, const Vec4c& color);
			void add_idx(uint idx);

			void begin_path() override;
			void move_to(const Vec2f& pos) override;
			void line_to(const Vec2f& pos) override;
			void close_path() override;

			void stroke(const Vec4c& col, float thickness, bool aa = false) override;
			void fill(const Vec4c& col, bool aa = false) override;
			void draw_text(uint res_id, const wchar_t* text_beg, const wchar_t* text_end, uint font_size, const Vec4c& col, const Vec2f& pos, const Mat2f& axes) override;
			void draw_image(uint res_id, uint tile_id, const Vec2f& LT, const Vec2f& RT, const Vec2f& RB, const Vec2f& LB, const Vec2f& uv0, const Vec2f& uv1, const Vec4c& tint_col) override;

			void set_camera(float fovy, float aspect, float zNear, float zFar, const Mat3f& axes, const Vec3f& coord) override;

			void draw_mesh(uint mod_id, uint mesh_idx, const Mat4f& transform, const Mat4f& normal_matrix, bool cast_shadow, ArmatureDeformer* deformer) override;
			void draw_terrain(const Vec2u& blocks, const Vec3f& scale, const Vec3f& coord, float tess_levels, uint height_tex_id, uint normal_tex_id, uint color_tex_id) override;
			void add_light(LightType type, const Mat4f& transform, const Vec3f& color, bool cast_shadow) override;

			void draw_lines(uint lines_count, const Line3* lines) override;

			Vec4f get_scissor() const override { return curr_scissor; }
			void set_scissor(const Vec4f& scissor) override;

			void add_blur(const Vec4f& range, uint radius) override;
			void add_bloom() override;

			void prepare() override;

			void record(CommandBufferPrivate* cb, uint image_index);
		};

		inline void CanvasBridge::set_output(uint views_count, ImageView* const* views)
		{
			((CanvasPrivate*)this)->set_output({ (ImageViewPrivate**)views, views_count });
		}

		inline int CanvasBridge::find_element_resource(const char* name)
		{
			return ((CanvasPrivate*)this)->find_element_resource(name ? name : "");
		}

		inline uint CanvasBridge::set_element_resource(int slot, ElementResource r, const char* name)
		{
			return ((CanvasPrivate*)this)->set_element_resource(slot, r, name ? name : "");
		}

		inline int CanvasBridge::find_texture_resource(const char* name)
		{
			return ((CanvasPrivate*)this)->find_texture_resource(name ? name : "");
		}

		inline uint CanvasBridge::set_texture_resource(int slot, ImageView* iv, Sampler* sp, const char* name)
		{
			return ((CanvasPrivate*)this)->set_texture_resource(slot, (ImageViewPrivate*)iv, (SamplerPrivate*)sp, name ? name : "");
		}

		inline int CanvasBridge::find_material_resource(const char* name)
		{
			return ((CanvasPrivate*)this)->find_material_resource(name ? name : "");
		}

		inline uint CanvasBridge::set_material_resource(int slot, Material* mat, const char* name) 
		{
			return ((CanvasPrivate*)this)->set_material_resource(slot, (MaterialPrivate*)mat, name ? name : "");
		}

		inline int CanvasBridge::find_model_resource(const char* name)
		{
			return ((CanvasPrivate*)this)->find_model_resource(name ? name : "");
		}

		inline uint CanvasBridge::set_model_resource(int slot, Model* mod, const char* name)
		{
			return ((CanvasPrivate*)this)->set_model_resource(slot, (ModelPrivate*)mod, name ? name : "");
		}

		inline void CanvasBridge::record(CommandBuffer* cb, uint image_index)
		{
			((CanvasPrivate*)this)->record((CommandBufferPrivate*)cb, image_index);
		}
	}
}
