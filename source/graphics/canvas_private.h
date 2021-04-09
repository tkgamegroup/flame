//#include <flame/graphics/canvas.h>
//
//namespace flame
//{
//	namespace graphics
//	{
//		struct DevicePrivate;
//		struct BufferPrivate;
//		struct ImagePrivate;
//		struct ImageViewPrivate;
//		struct ImageAtlasPrivate;
//		struct FontAtlasPrivate;
//		struct MaterialPrivate;
//		struct ModelPrivate;
//		struct RenderpassPrivate;
//		struct FramebufferPrivate;
//		struct DescriptorSetPrivate;
//		struct ShaderPrivate;
//		struct PipelineLayoutPrivate;
//		struct PipelinePrivate;
//		struct CommandBufferPrivate;
//		struct CanvasPrivate;
//
//		enum MaterialUsage
//		{
//			MaterialForMesh,
//			MaterialForMeshArmature,
//			MaterialForMeshShadow,
//			MaterialForMeshShadowArmature,
//			MaterialForTerrain,
//
//			MaterialUsageCount
//		};
//
//		struct MaterialPipeline
//		{
//			std::filesystem::path mat;
//			std::vector<std::string> defines;
//			uint ref_count = 1;
//			std::unique_ptr<PipelinePrivate> pipeline;
//		};
//
//		struct RenderPreferencesPrivate : RenderPreferences
//		{
//			bool hdr;
//
//			DevicePtr device;
//
//			std::unique_ptr<RenderpassPrivate> rgba8_renderpass;
//			std::unique_ptr<RenderpassPrivate> rgba16_renderpass;
//			std::unique_ptr<RenderpassPrivate> rgba8c_renderpass;
//			std::unique_ptr<RenderpassPrivate> rgba16c_renderpass;
//			std::unique_ptr<RenderpassPrivate> r16_renderpass;
//			std::unique_ptr<RenderpassPrivate> mesh_renderpass;
//			std::unique_ptr<RenderpassPrivate> depth_renderpass;
//			std::unique_ptr<RenderpassPrivate> pickup_renderpass;
//			std::vector<MaterialPipeline> material_pipelines[MaterialUsageCount];
//			std::unique_ptr<PipelinePrivate> element_pipeline;
//			std::unique_ptr<PipelinePrivate> sky_pipeline;
//			PipelineLayoutPrivate* mesh_pipeline_layout;
//			PipelineLayoutPrivate* terrain_pipeline_layout;
//			PipelinePrivate* mesh_wireframe_pipeline;
//			PipelinePrivate* mesh_armature_wireframe_pipeline;
//			PipelinePrivate* terrain_wireframe_pipeline;
//			PipelinePrivate* mesh_pickup_pipeline;
//			PipelinePrivate* mesh_armature_pickup_pipeline;
//			PipelinePrivate* terrain_pickup_pipeline;
//			PipelinePrivate* mesh_outline_pipeline;
//			PipelinePrivate* mesh_armature_outline_pipeline;
//			PipelinePrivate* terrain_outline_pipeline;
//			std::unique_ptr<PipelinePrivate> line_pipeline;
//			std::unique_ptr<PipelinePrivate> triangle_pipeline;
//			std::unique_ptr<PipelinePrivate> blurh_pipeline[10];
//			std::unique_ptr<PipelinePrivate> blurv_pipeline[10];
//			std::unique_ptr<PipelinePrivate> blurh_depth_pipeline;
//			std::unique_ptr<PipelinePrivate> blurv_depth_pipeline;
//			std::unique_ptr<PipelinePrivate> blit_8_pipeline;
//			std::unique_ptr<PipelinePrivate> blit_16_pipeline;
//			std::unique_ptr<PipelinePrivate> blend_8_pipeline;
//			std::unique_ptr<PipelinePrivate> blend_16_pipeline;
//			std::unique_ptr<PipelinePrivate> filter_bright_pipeline;
//			std::unique_ptr<PipelinePrivate> downsample_pipeline;
//			std::unique_ptr<PipelinePrivate> upsample_pipeline;
//			std::unique_ptr<PipelinePrivate> gamma_pipeline;
//
//			RenderPreferencesPrivate(DevicePtr device, bool hdr);
//
//			void release() override { delete this; }
//
//			PipelinePrivate* get_material_pipeline(MaterialUsage usage, const std::filesystem::path& mat, const std::string& defines);
//			void release_material_pipeline(MaterialUsage usage, PipelinePrivate* p);
//		};
//
//		template <class T>
//		struct ShaderGeometryBuffer
//		{
//			BufferUsageFlags usage;
//			uint capacity = 0;
//			AccessFlags access;
//			T* stag = nullptr;
//			uint stag_num = 0;
//
//			DevicePrivate* d = nullptr;
//			std::unique_ptr<BufferPrivate> buf;
//			std::unique_ptr<BufferPrivate> stgbuf;
//
//			void rebuild()
//			{
//				T* temp = nullptr;
//				auto n = 0;
//				if (stag_num > 0)
//				{
//					n = stag_num;
//					temp = new T[n];
//					memcpy(temp, stgbuf->mapped, sizeof(T) * n);
//				}
//				buf.reset(new BufferPrivate(d, capacity * sizeof(T), BufferUsageTransferDst | usage, MemoryPropertyDevice));
//				stgbuf.reset(new BufferPrivate(d, buf->size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
//				stgbuf->map();
//				stag = (T*)stgbuf->mapped;
//				if (temp)
//				{
//					push(n, temp);
//					delete[]temp;
//				}
//			}
//
//			void create(DevicePrivate* _d, BufferUsageFlags _usage, uint _capacity, AccessFlags _access = AccessVertexAttributeRead)
//			{
//				d = _d;
//				usage = _usage;
//				capacity = _capacity;
//				access = _access;
//				rebuild();
//			}
//
//			void push(uint cnt, const T* p)
//			{
//				if (stag_num + cnt > capacity)
//				{
//					capacity = (stag_num + cnt) * 2;
//					rebuild();
//				}
//
//				memcpy(stag + stag_num, p, sizeof(T) * cnt);
//				stag_num += cnt;
//			}
//
//			void upload(CommandBufferPrivate* cb)
//			{
//				BufferCopy cpy;
//				cpy.size = stag_num * sizeof(T);
//				cb->copy_buffer(stgbuf.get(), buf.get(), { &cpy, 1 });
//				cb->buffer_barrier(buf.get(), AccessTransferWrite, access);
//			}
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
//		const auto msaa_sample_count = SampleCount_8;
//		const auto shadow_map_size = uvec2(2048);
//
//		struct ElementVertex
//		{
//			vec2 position;
//			vec2 uv;
//			cvec4 color;
//		};
//
//		struct MeshVertex
//		{
//			vec3 position;
//			vec2 uv;
//			vec3 normal;
//		};
//
//		struct MeshWeight
//		{
//			ivec4 ids;
//			vec4 weights;
//		};
//
//		struct ElementResourcePrivate
//		{
//			std::string name;
//			ImageViewPrivate* iv;
//			ImageAtlasPrivate* ia;
//			FontAtlasPrivate* fa;
//		};
//
//		struct TextureResourcePrivate
//		{
//			std::string name;
//			ImageViewPrivate* iv;
//		};
//
//		struct MaterialResourcePrivate
//		{
//			CanvasPrivate* canvas;
//
//			std::string name;
//			MaterialPrivate* material;
//			std::pair<uint, std::unique_ptr<ImagePrivate>> textures[4];
//
//			PipelinePrivate* pipelines[MaterialUsageCount] = {};
//
//			MaterialResourcePrivate(CanvasPrivate* canvas) :
//				canvas(canvas)
//			{
//			}
//
//			~MaterialResourcePrivate();
//
//			PipelinePrivate* get_pipeline(MaterialUsage u);
//		};
//
//		struct MeshResourcePrivate
//		{
//			ShaderGeometryBuffer<MeshVertex> vertex_buffer;
//			ShaderGeometryBuffer<MeshWeight> weight_buffer;
//			ShaderGeometryBuffer<uint> index_buffer;
//			uint material_id;
//		};
//
//		struct ModelResourcePrivate
//		{
//			CanvasPrivate* canvas;
//
//			ModelResourcePrivate(CanvasPrivate* canvas) :
//				canvas(canvas)
//			{
//			}
//
//			~ModelResourcePrivate();
//
//			std::string name;
//			ModelPrivate* model;
//			std::vector<uint> materials;
//			std::vector<std::unique_ptr<MeshResourcePrivate>> meshes;
//		};
//
//		struct MeshInfo
//		{
//			MeshResourcePrivate* res;
//			mat4 transform;
//			bool cast_shadow;
//			ArmatureDeformerPrivate* deformer;
//			ShadeFlags flags;
//			void* userdata;
//		};
//
//		struct TerrainInfo
//		{
//			uvec2 blocks;
//			vec3 scale;
//			vec3 coord;
//			float tess_levels;
//			uint height_tex_id;
//			uint normal_tex_id;
//			uint material_id;
//			ShadeFlags flags;
//			void* userdata;
//		};
//
//		struct DirectionalLight
//		{
//			vec3 dir;
//			vec3 side;
//			vec3 up;
//			vec3 color;
//			bool cast_shadow;
//			float shadow_distance;
//		};
//
//		struct PointLight
//		{
//			vec3 coord;
//			vec3 color;
//			bool cast_shadow;
//			float shadow_distance;
//		};
//
//		struct Cmd
//		{
//			enum Type
//			{
//				DrawElement,
//				DrawMesh,
//				DrawTerrain,
//				DrawLines,
//				DrawTriangles,
//				SetScissor,
//				SetViewport,
//				Blur,
//				Bloom
//			};
//
//			Type type;
//
//			Cmd(Type t) : type(t) {}
//			virtual ~Cmd() {}
//		};
//
//		struct CmdDrawElement : Cmd
//		{
//			uint id;
//			uint vertices_count = 0;
//			uint indices_count = 0;
//
//			CmdDrawElement(uint _id) : Cmd(DrawElement) { id = _id; }
//		};
//
//		struct CmdDrawMesh : Cmd
//		{
//			std::vector<uint> entries;
//
//			CmdDrawMesh() : Cmd(DrawMesh) {}
//		};
//
//		struct CmdDrawTerrain : Cmd
//		{
//			std::vector<uint> entries;
//
//			CmdDrawTerrain() : Cmd(DrawTerrain) {}
//		};
//
//		struct CmdDrawLines : Cmd
//		{
//			uint count = 0;
//
//			CmdDrawLines() : Cmd(DrawLines) {}
//		};
//
//		struct CmdDrawTriangles : Cmd
//		{
//			uint count = 0;
//
//			CmdDrawTriangles() : Cmd(DrawTriangles) {}
//		};
//
//		struct CmdSetScissor : Cmd
//		{
//			Rect scissor;
//
//			CmdSetScissor(const Rect& _scissor) : Cmd(SetScissor) { scissor = _scissor; }
//		};
//
//		struct CmdSetViewport : Cmd
//		{
//			Rect viewport;
//
//			CmdSetViewport(const Rect& _viewport) : Cmd(SetViewport) { viewport = _viewport; }
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
//		struct CmdBloom : Cmd
//		{
//			CmdBloom() : Cmd(Bloom) {}
//		};
//
//		struct CanvasBridge : Canvas
//		{
//			void set_output(uint views_count, ImageViewPtr const* views) override;
//
//			int find_element_resource(const char* name) override;
//			uint set_element_resource(int slot, ElementResource r, const char* name) override;
//			int find_texture_resource(const char* name) override;
//			uint set_texture_resource(int slot, ImageViewPtr iv, Sampler* sp, const char* name) override;
//			int find_material_resource(const char* name) override;
//			uint set_material_resource(int slot, Material* mat, const char* name) override;
//			int find_model_resource(const char* name) override;
//			uint set_model_resource(int slot, Model* mod, const char* name) override;
//
//			void set_sky(ImageViewPtr box, ImageViewPtr irr, ImageViewPtr rad, ImageViewPtr lut) override;
//
//			void record(CommandBuffer* cb, uint image_index) override;
//		};
//
//		struct CanvasPrivate : CanvasBridge
//		{
//			RenderPreferencesPrivate* preferences;
//
//			float shadow_distance = 100.f;
//			uint csm_levels = 3;
//			float csm_factor = 0.3f;
//
//			cvec4 clear_color = cvec4(0, 0, 0, 255);
//
//			float fovy;
//			float aspect;
//			float zNear;
//			float zFar;
//			vec3 camera_coord;
//			mat3 camera_dirs;
//			mat4 view_matrix;
//			mat4 view_inv_matrix;
//			mat4 proj_matrix;
//			mat4 proj_view_matrix;
//
//			std::unique_ptr<ImagePrivate> white_image;
//			std::vector < ElementResourcePrivate > element_resources;
//			std::vector<TextureResourcePrivate> texture_resources;
//			std::vector<std::unique_ptr<MaterialResourcePrivate>> material_resources;
//			std::vector<std::unique_ptr<ModelResourcePrivate>> model_resources;
//
//			ShaderGeometryBuffer<ElementVertex> element_vertex_buffer;
//			ShaderGeometryBuffer<uint> element_index_buffer;
//			std::unique_ptr<DescriptorSetPrivate> element_descriptorset;
//
//			//ShaderBuffer render_data_buffer;
//			std::unique_ptr<DescriptorSetPrivate> render_data_descriptorset;
//
//			std::unique_ptr<ImagePrivate> default_sky_box_image;
//			std::unique_ptr<ImagePrivate> default_sky_irr_image;
//			std::unique_ptr<ImagePrivate> default_sky_rad_image;
//			std::unique_ptr<DescriptorSetPrivate> sky_descriptorset;
//
//			//ShaderBuffer mesh_matrix_buffer;
//			std::unique_ptr<DescriptorSetPrivate> mesh_descriptorset;
//
//			//ShaderBuffer material_info_buffer;
//			std::unique_ptr<DescriptorSetPrivate> material_descriptorset;
//
//			std::unique_ptr<ImagePrivate> shadow_depth_image;
//			std::unique_ptr<ImagePrivate> shadow_depth_back_image;
//			std::unique_ptr<FramebufferPrivate> shadow_depth_back_framebuffer;
//			std::unique_ptr<DescriptorSetPrivate> shadow_depth_back_descriptorset;
//
//			//ShaderBuffer light_sets_buffer;
//			//ShaderBuffer light_infos_buffer;
//			//ShaderBuffer shadow_matrices_buffer;
//			std::vector<std::unique_ptr<ImagePrivate>> directional_shadow_maps;
//			std::vector<std::unique_ptr<FramebufferPrivate>> directional_light_depth_framebuffers;
//			std::vector<std::unique_ptr<FramebufferPrivate>> directional_shadow_map_framebuffers;
//			std::vector<std::unique_ptr<DescriptorSetPrivate>> directional_shadow_map_descriptorsets;
//			std::vector<std::unique_ptr<ImagePrivate>> point_shadow_maps;
//			std::vector<std::unique_ptr<FramebufferPrivate>> point_light_depth_framebuffers;
//			std::vector<std::unique_ptr<FramebufferPrivate>> point_shadow_map_framebuffers;
//			std::vector<std::unique_ptr<DescriptorSetPrivate>> point_shadow_map_descriptorsets;
//
//			std::unique_ptr<DescriptorSetPrivate> light_descriptorset;
//
//			//ShaderBuffer terrain_info_buffer;
//			std::unique_ptr<DescriptorSetPrivate> terrain_descriptorset;
//
//			std::vector<ImageViewPrivate*> output_imageviews;
//			std::vector<std::unique_ptr<FramebufferPrivate>> output_framebuffers;
//			std::vector<std::unique_ptr<DescriptorSetPrivate>> output_descriptorsets;
//
//			std::unique_ptr<ImagePrivate> hdr_image;
//			std::unique_ptr<FramebufferPrivate> hdr_framebuffer;
//			std::unique_ptr<DescriptorSetPrivate> hdr_descriptorset;
//
//			std::unique_ptr<ImagePrivate> depth_image;
//
//			std::vector<std::unique_ptr<FramebufferPrivate>> mesh_framebuffers;
//
//			std::unique_ptr<ImagePrivate> back_image;
//			std::vector<std::unique_ptr<FramebufferPrivate>> back_framebuffers;
//			std::vector<std::unique_ptr<DescriptorSetPrivate>> back_nearest_descriptorsets;
//			std::vector<std::unique_ptr<DescriptorSetPrivate>> back_linear_descriptorsets;
//
//			std::unique_ptr<ImagePrivate> pickup_image;
//			std::unique_ptr<FramebufferPrivate> pickup_framebuffer;
//
//			std::vector<std::vector<vec2>> paths;
//
//			ShaderGeometryBuffer<Line> line_buffer;
//			ShaderGeometryBuffer<Triangle> triangle_buffer;
//
//			std::vector<MeshInfo> meshes;
//			std::vector<TerrainInfo> terrains;
//			std::vector<DirectionalLight> directional_lights;
//			std::vector<PointLight> point_lights;
//
//			std::vector<std::unique_ptr<Cmd>> cmds;
//
//			uvec2 output_size;
//			Rect curr_scissor;
//			Rect curr_viewport;
//
//			CanvasPrivate(RenderPreferencesPrivate* preferences);
//
//			void release() override { delete this; }
//
//			RenderPreferences* get_preferences() const override { return preferences; };
//
//			void set_shadow(float distance, uint csm_levels, float csm_factor) override;
//
//			cvec4 get_clear_color() const override { return clear_color; }
//			void set_clear_color(const cvec4& color) override;
//
//			ImageViewPtr get_output(uint idx) const override { return output_imageviews.empty() ? nullptr : (ImageViewPtr)output_imageviews[idx]; }
//			void set_output(std::span<ImageViewPrivate*> views);
//
//			ElementResource get_element_resource(uint slot) override;
//			int find_element_resource(const std::string& name);
//			uint set_element_resource(int slot, ElementResource r, const std::string& name);
//
//			ImageViewPtr get_texture_resource(uint slot) override;
//			int find_texture_resource(const std::string& name);
//			uint set_texture_resource(int slot, ImageViewPrivate* iv, SamplerPrivate* sp, const std::string& name);
//
//			Material* get_material_resource(uint slot) override;
//			int find_material_resource(const std::string& name);
//			uint set_material_resource(int slot, MaterialPrivate* mat, const std::string& name);
//
//			Model* get_model_resource(uint slot) override;
//			int find_model_resource(const std::string& name);
//			uint set_model_resource(int slot, ModelPrivate* mat, const std::string& name);
//
//			CmdDrawElement* add_draw_element_cmd(uint id);
//			void add_vtx(const vec2& position, const vec2& uv, const cvec4& color);
//			void add_idx(uint idx);
//
//			void begin_path() override;
//			void move_to(const vec2& pos) override;
//			void line_to(const vec2& pos) override;
//			void close_path() override;
//
//			void stroke(const cvec4& col, float thickness, bool aa = false) override;
//			void fill(const cvec4& col, bool aa = false) override;
//			void draw_image(uint res_id, uint tile_id, const vec2& pos, const vec2& size, const mat2& axes, const vec2& uv0, const vec2& uv1, const cvec4& tint_col) override;
//			void draw_text(uint res_id, const wchar_t* text_beg, const wchar_t* text_end, uint font_size, const cvec4& col, const vec2& pos, const mat2& axes) override;
//
//			void set_camera(float fovy, float aspect, float zNear, float zFar, const mat3& dirs, const vec3& coord) override;
//			void set_sky(ImageViewPrivate* box, ImageViewPrivate* irr, ImageViewPrivate* rad, ImageViewPrivate* lut);
//
//			void draw_mesh(uint mod_id, uint mesh_idx, const mat4& transform, bool cast_shadow, ArmatureDeformer* deformer, ShadeFlags flags = ShadeMaterial, void* userdata = nullptr) override;
//			void draw_terrain(const uvec2& blocks, const vec3& scale, const vec3& coord, float tess_levels, uint height_tex_id, uint normal_tex_id, uint material_id, ShadeFlags flags = ShadeMaterial, void* userdata = nullptr) override;
//			void add_light(LightType type, const mat3& dirs, const vec3& color, bool cast_shadow) override;
//
//			void draw_lines(uint lines_count, const Line* lines) override;
//			void draw_triangles(uint triangles_count, const Triangle* triangles) override;
//
//			mat4 get_view_matrix() const override { return view_matrix; }
//			mat4 get_proj_matrix() const override { return proj_matrix; }
//
//			void* pickup(const vec2& p) override;
//
//			Rect get_scissor() const override { return curr_scissor; }
//			void set_scissor(const Rect& scissor) override;
//
//			Rect get_viewport() const override { return curr_viewport; }
//			void set_viewport(const Rect& viewport) override;
//
//			void add_blur(const Rect& range, uint radius) override;
//			void add_bloom() override;
//
//			void prepare() override;
//
//			void record(CommandBufferPrivate* cb, uint image_index);
//		};
//
//		inline void CanvasBridge::set_output(uint views_count, ImageViewPtr const* views)
//		{
//			((CanvasPrivate*)this)->set_output({ (ImageViewPrivate**)views, views_count });
//		}
//
//		inline int CanvasBridge::find_element_resource(const char* name)
//		{
//			return ((CanvasPrivate*)this)->find_element_resource(name ? name : "");
//		}
//
//		inline uint CanvasBridge::set_element_resource(int slot, ElementResource r, const char* name)
//		{
//			return ((CanvasPrivate*)this)->set_element_resource(slot, r, name ? name : "");
//		}
//
//		inline int CanvasBridge::find_texture_resource(const char* name)
//		{
//			return ((CanvasPrivate*)this)->find_texture_resource(name ? name : "");
//		}
//
//		inline uint CanvasBridge::set_texture_resource(int slot, ImageViewPtr iv, Sampler* sp, const char* name)
//		{
//			return ((CanvasPrivate*)this)->set_texture_resource(slot, (ImageViewPrivate*)iv, (SamplerPrivate*)sp, name ? name : "");
//		}
//
//		inline int CanvasBridge::find_material_resource(const char* name)
//		{
//			return ((CanvasPrivate*)this)->find_material_resource(name ? name : "");
//		}
//
//		inline uint CanvasBridge::set_material_resource(int slot, Material* mat, const char* name) 
//		{
//			return ((CanvasPrivate*)this)->set_material_resource(slot, (MaterialPrivate*)mat, name ? name : "");
//		}
//
//		inline int CanvasBridge::find_model_resource(const char* name)
//		{
//			return ((CanvasPrivate*)this)->find_model_resource(name ? name : "");
//		}
//
//		inline uint CanvasBridge::set_model_resource(int slot, Model* mod, const char* name)
//		{
//			return ((CanvasPrivate*)this)->set_model_resource(slot, (ModelPrivate*)mod, name ? name : "");
//		}
//
//		inline void CanvasBridge::set_sky(ImageViewPtr box, ImageViewPtr irr, ImageViewPtr rad, ImageViewPtr lut)
//		{
//			return ((CanvasPrivate*)this)->set_sky((ImageViewPrivate*)box, (ImageViewPrivate*)irr, (ImageViewPrivate*)rad, (ImageViewPrivate*)lut);
//		}
//
//		inline void CanvasBridge::record(CommandBuffer* cb, uint image_index)
//		{
//			((CanvasPrivate*)this)->record((CommandBufferPrivate*)cb, image_index);
//		}
//	}
//}
