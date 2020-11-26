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
		struct CanvasPrivate;

		enum MaterialUsage
		{
			MaterialForMesh,
			MaterialForMeshArmature,
			MaterialForTerrain,
			MaterialForDepth,
			MaterialForDepthArmature,

			MaterialUsageCount
		};

		struct RenderPreferencesPrivate : RenderPreferences
		{
			DevicePrivate* device;

			bool hdr;
			bool msaa_3d;

			std::unique_ptr<RenderpassPrivate> image1_8_renderpass;
			std::unique_ptr<RenderpassPrivate> image1_16_renderpass;
			std::unique_ptr<RenderpassPrivate> image1_r16_renderpass;
			std::unique_ptr<RenderpassPrivate> mesh_renderpass;
			std::unique_ptr<RenderpassPrivate> depth_renderpass;
			std::vector<std::tuple<std::filesystem::path, std::string, uint, std::unique_ptr<PipelinePrivate>>> material_pipelines[MaterialUsageCount];
			std::unique_ptr<PipelinePrivate> mesh_wireframe_pipeline;
			std::unique_ptr<PipelinePrivate> mesh_armature_wireframe_pipeline;
			std::unique_ptr<PipelinePrivate> terrain_wireframe_pipeline;
			std::unique_ptr<PipelinePrivate> element_pipeline;
			std::unique_ptr<PipelinePrivate> sky_pipeline;
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

			RenderPreferencesPrivate(DevicePrivate* device, bool hdr, bool msaa_3d);

			PipelinePrivate* get_material_pipeline(MaterialUsage usage, const std::filesystem::path& mat, const std::string& defines);
			PipelinePrivate* create_material_pipeline(MaterialUsage usage, const std::filesystem::path& mat, const std::string& defines);
			void release_material_pipeline(MaterialUsage usage, PipelinePrivate* p);
		};

		struct ShaderBufferBase
		{
			std::unique_ptr<BufferPrivate> buf;
			std::unique_ptr<BufferPrivate> stgbuf;
		};

		struct ShaderBuffer : ShaderBufferBase
		{
			struct Piece
			{
				uint beg;
				uint end;
				bool peeding_update = false;
				std::list<std::pair<uint, uint>>::iterator cpy_it;
			};

			uint size;
			uint arrsize = 0;
			char* stag = nullptr;

			ShaderType* t = nullptr;
			std::vector<Piece> pieces;
			std::list<std::pair<uint, uint>> cpys;

			void create(DevicePrivate* d, BufferUsageFlags usage, ShaderType* _t, uint _arrsize = 0)
			{
				t = _t;
				size = t->size;
				auto bufsize = size * (_arrsize == 0 ? 1 : _arrsize);
				buf.reset(new BufferPrivate(d, bufsize, BufferUsageTransferDst | usage, MemoryPropertyDevice));
				stgbuf.reset(new BufferPrivate(d, bufsize, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
				stgbuf->map();
				stag = (char*)stgbuf->mapped;

				if (_arrsize == 0)
				{
					pieces.resize(t->variables.size());
					for (auto i = 0; i < pieces.size(); i++)
					{
						auto& src = t->variables[i];
						Piece dst;
						dst.beg = src.offset;
						dst.end = src.offset + src.size;
						pieces[i] = dst;
					}
				}
				else
				{
					arrsize = _arrsize;
					pieces.resize(arrsize);
					for (auto i = 0; i < pieces.size(); i++)
					{
						Piece dst;
						dst.beg = i * size;
						dst.end = dst.beg + size;
						pieces[i] = dst;
					}
				}
			}

			void mark_piece_dirty(uint idx)
			{
				auto& p = pieces[idx];
				if (!p.peeding_update)
				{
					if (idx > 0)
					{
						auto& pp = pieces[idx - 1];
						if (pp.peeding_update)
						{
							p.peeding_update = true;
							p.cpy_it = pp.cpy_it;
							pp.cpy_it->second++;
						}
					}
					if (!p.peeding_update)
					{
						p.peeding_update = true;
						p.cpy_it = cpys.emplace(cpys.end(), std::make_pair(idx, 1));
					}
					if (idx < pieces.size() - 1)
					{
						auto& np = pieces[idx + 1];
						if (np.peeding_update)
						{
							auto it = np.cpy_it;
							p.cpy_it->second += it->second;
							for (auto i = 0; i < it->second; i++)
								pieces[it->first + i].cpy_it = p.cpy_it;
							cpys.erase(it);
						}
					}
				}
			}

			char* dst(uint64 h, char* p = nullptr)
			{
				if (!p)
					p = (char*)stag;
				return p + t->variables[t->variables_map[h]].offset;
			}

			char* mark(uint64 h)
			{
				fassert(arrsize == 0);
				auto idx = t->variables_map[h];
				mark_piece_dirty(idx);
				return stag + t->variables[idx].offset;
			}

			template <class T>
			void set(uint64 h, const T& v)
			{
				fassert(arrsize == 0);
				*(T*)mark(h) = v;
			}

			char* mark_item(uint idx)
			{
				fassert(arrsize != 0);
				mark_piece_dirty(idx);
				return stag + pieces[idx].beg;
			}

			template <class T>
			void set(char* p, uint64 h, const T& v)
			{
				fassert(arrsize != 0);
				*(T*)dst(h, p) = v;
			}

			void upload(CommandBufferPrivate* cb)
			{
				if (cpys.empty())
					return;
				std::vector<BufferCopy> _cpys;
				for (auto& cpy : cpys)
				{
					for (auto i = 0; i < cpy.second; i++)
						pieces[cpy.first + i].peeding_update = false;
					BufferCopy _cpy;
					_cpy.src_off = _cpy.dst_off = pieces[cpy.first].beg;
					_cpy.size = pieces[cpy.first + cpy.second - 1].end - _cpy.src_off;
					_cpys.push_back(_cpy);
				}
				cb->copy_buffer(stgbuf.get(), buf.get(), _cpys);
				cb->buffer_barrier(buf.get(), AccessTransferWrite, AccessVertexAttributeRead);
				cpys.clear();
			}
		};

		template <class T>
		struct ShaderGeometryBuffer : ShaderBufferBase
		{
			DevicePrivate* d = nullptr;
			BufferUsageFlags usage;
			uint capacity = 0;
			T* stag = nullptr;
			uint stagnum = 0;

			void rebuild()
			{
				T* temp = nullptr;
				auto n = 0;
				if (stagnum > 0)
				{
					n = stagnum;
					temp = new T[n];
					memcpy(temp, stgbuf->mapped, sizeof(T) * n);
				}
				buf.reset(new BufferPrivate(d, capacity * sizeof(T), BufferUsageTransferDst | usage, MemoryPropertyDevice));
				stgbuf.reset(new BufferPrivate(d, buf->size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
				stgbuf->map();
				stag = (T*)stgbuf->mapped;
				if (temp)
				{
					push(n, temp);
					delete[]temp;
				}
			}

			void create(DevicePrivate* _d, BufferUsageFlags _usage, uint _capacity)
			{
				d = _d;
				usage = _usage;
				capacity = _capacity;
				rebuild();
			}

			void push(uint cnt, const T* p)
			{
				if (stagnum + cnt > capacity)
				{
					capacity = (stagnum + cnt) * 2;
					rebuild();
				}

				memcpy(stag + stagnum, p, sizeof(T) * cnt);
				stagnum += cnt;
			}

			void upload(CommandBufferPrivate* cb)
			{
				BufferCopy cpy;
				cpy.size = stagnum * sizeof(T);
				cb->copy_buffer(stgbuf.get(), buf.get(), { &cpy, 1 });
				cb->buffer_barrier(buf.get(), AccessTransferWrite, AccessVertexAttributeRead);
			}
		};

		struct ArmatureDeformerPrivate : ArmatureDeformer
		{
			MeshPrivate* mesh;
			ShaderBuffer poses_buffer;
			std::unique_ptr<DescriptorSetPrivate> descriptorset;

			ArmatureDeformerPrivate(RenderPreferencesPrivate* preferences, MeshPrivate* mesh);
			void release() override { delete this; }
			void set_pose(uint id, const mat4& pose) override;
		};

		const auto msaa_sample_count = SampleCount_8;
		const auto shadow_map_size = uvec2(2048);

		struct ElementVertex
		{
			vec2 position;
			vec2 uv;
			cvec4 color;
		};

		struct MeshVertex
		{
			vec3 position;
			vec2 uv;
			vec3 normal;
		};

		struct MeshWeight
		{
			ivec4 ids;
			vec4 weights;
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
			CanvasPrivate* canvas;

			std::string name;
			MaterialPrivate* material;
			std::pair<uint, std::unique_ptr<ImagePrivate>> textures[4];

			PipelinePrivate* pipelines[MaterialUsageCount] = {};

			MaterialResourceSlot(CanvasPrivate* canvas) :
				canvas(canvas)
			{
			}

			~MaterialResourceSlot();

			PipelinePrivate* get_pipeline(MaterialUsage u);
		};

		struct ModelResourceSlot
		{
			struct Mesh
			{
				ShaderGeometryBuffer<MeshVertex> vertex_buffer;
				ShaderGeometryBuffer<MeshWeight> weight_buffer;
				ShaderGeometryBuffer<uint> index_buffer;
				uint material_id;
			};

			CanvasPrivate* canvas;

			ModelResourceSlot(CanvasPrivate* canvas) :
				canvas(canvas)
			{
			}

			~ModelResourceSlot();

			std::string name;
			ModelPrivate* model;
			std::vector<uint> materials;
			std::vector<std::unique_ptr<Mesh>> meshes;
		};

		struct DirectionalShadow
		{
			mat4 matrices[4];
		};

		struct PointShadow
		{
			vec3 coord;
			float distance;
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
			uint drawcall_count;
			uint material_id;

			CmdDrawTerrain() : Cmd(DrawTerrain) {}
		};

		struct CmdDrawLine3 : Cmd
		{
			uint count;

			CmdDrawLine3() : Cmd(DrawLine3) {}
		};

		struct CmdSetScissor : Cmd
		{
			vec4 scissor;

			CmdSetScissor(const vec4& _scissor) : Cmd(SetScissor) { scissor = _scissor; }
		};

		struct CmdBlur : Cmd
		{
			vec4 range;
			uint radius;

			CmdBlur(const vec4& _range, uint _radius) : Cmd(Blur) { range = _range; radius = _radius; }
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

			ShadingType shading_type = ShadingSolid;

			float shadow_distance = 100.f;
			uint csm_levels = 3;
			float csm_factor = 0.3f;

			cvec4 clear_color = cvec4(0, 0, 0, 255);

			float fovy;
			float aspect;
			float zNear;
			float zFar;
			vec3 camera_coord;
			mat3 camera_dirs;
			mat4 view_matrix;
			mat4 view_inv_matrix;
			mat4 proj_matrix;
			mat4 proj_view_matrix;

			int sky_box_tex_id = -1;
			int sky_irr_tex_id = -1;
			int sky_rad_tex_id = -1;

			std::unique_ptr<ImagePrivate> white_image;
			std::vector < ElementResourceSlot > element_resources;
			std::vector<TextureResourceSlot> texture_resources;
			std::vector<std::unique_ptr<MaterialResourceSlot>> material_resources;
			std::vector<std::unique_ptr<ModelResourceSlot>> model_resources;

			ShaderGeometryBuffer<ElementVertex> element_vertex_buffer;
			ShaderGeometryBuffer<uint> element_index_buffer;
			std::unique_ptr<DescriptorSetPrivate> element_descriptorset;

			ShaderBuffer render_data_buffer;
			std::unique_ptr<DescriptorSetPrivate> render_data_descriptorset;

			ShaderBuffer mesh_matrix_buffer;
			std::unique_ptr<DescriptorSetPrivate> mesh_descriptorset;

			ShaderBuffer material_info_buffer;
			std::unique_ptr<DescriptorSetPrivate> material_descriptorset;

			std::unique_ptr<ImagePrivate> shadow_depth_image;
			std::unique_ptr<ImagePrivate> shadow_blur_pingpong_image;
			std::unique_ptr<FramebufferPrivate> shadow_blur_pingpong_image_framebuffer;
			std::unique_ptr<DescriptorSetPrivate> shadow_blur_pingpong_image_descriptorset;

			ShaderBuffer light_indices_buffer;

			ShaderBuffer directional_light_info_buffer;
			std::vector<std::unique_ptr<ImagePrivate>> directional_light_shadow_maps;
			std::vector<std::unique_ptr<FramebufferPrivate>> directional_light_shadow_map_depth_framebuffers;
			std::vector<std::unique_ptr<FramebufferPrivate>> directional_light_shadow_map_framebuffers;
			std::vector<std::unique_ptr<DescriptorSetPrivate>> directional_light_shadow_map_descriptorsets;

			ShaderBuffer point_light_info_buffer;
			std::vector<std::unique_ptr<ImagePrivate>> point_light_shadow_maps;
			std::vector<std::unique_ptr<FramebufferPrivate>> point_light_shadow_map_depth_framebuffers;
			std::vector<std::unique_ptr<FramebufferPrivate>> point_light_shadow_map_framebuffers;
			std::vector<std::unique_ptr<DescriptorSetPrivate>> point_light_shadow_map_descriptorsets;

			std::unique_ptr<DescriptorSetPrivate> light_descriptorset;

			ShaderBuffer terrain_info_buffer;
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
			std::unique_ptr<FramebufferPrivate> mesh_resolve_resframebuffer;

			std::unique_ptr<ImagePrivate> back_image;
			std::vector<std::unique_ptr<FramebufferPrivate>> back_framebuffers;
			std::vector<std::unique_ptr<DescriptorSetPrivate>> back_nearest_descriptorsets;
			std::vector<std::unique_ptr<DescriptorSetPrivate>> back_linear_descriptorsets;

			std::vector<std::vector<vec2>> paths;

			ShaderGeometryBuffer<Line3> line3_buffer;

			std::vector<std::unique_ptr<Cmd>> cmds;
			CmdDrawElement* last_element_cmd = nullptr;
			uint  meshes_count = 0;
			uint terrains_count = 0;
			uint directional_lights_count;
			uint point_lights_count;
			std::vector<DirectionalShadow> directional_shadows;
			std::vector<PointShadow> point_shadows;
			CmdDrawMesh* last_mesh_cmd = nullptr;
			CmdDrawLine3* last_line3_cmd = nullptr;

			uvec2 output_size;
			vec4 curr_scissor;

			CanvasPrivate(RenderPreferencesPrivate* preferences);

			void release() override { delete this; }

			RenderPreferences* get_preferences() const override { return preferences; };

			void set_shading(ShadingType type) override;
			void set_shadow(float distance, uint csm_levels, float csm_factor) override;

			cvec4 get_clear_color() const override { return clear_color; }
			void set_clear_color(const cvec4& color) override { clear_color = color; }

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
			void add_vtx(const vec2& position, const vec2& uv, const cvec4& color);
			void add_idx(uint idx);

			void begin_path() override;
			void move_to(const vec2& pos) override;
			void line_to(const vec2& pos) override;
			void close_path() override;

			void stroke(const cvec4& col, float thickness, bool aa = false) override;
			void fill(const cvec4& col, bool aa = false) override;
			void draw_image(uint res_id, uint tile_id, const vec2& pos, const vec2& size, const mat3& transform, const vec2& uv0, const vec2& uv1, const cvec4& tint_col) override;
			void draw_text(uint res_id, const wchar_t* text_beg, const wchar_t* text_end, uint font_size, const cvec4& col, const vec2& pos, const mat3& transform) override;

			void set_camera(float fovy, float aspect, float zNear, float zFar, const mat3& dirs, const vec3& coord) override;
			void set_sky(int box_tex_id, int irr_tex_id, int rad_tex_id) override;

			void draw_mesh(uint mod_id, uint mesh_idx, const mat4& transform, const mat3& dirs, bool cast_shadow, ArmatureDeformer* deformer) override;
			void draw_terrain(const uvec2& blocks, const vec3& scale, const vec3& coord, float tess_levels, uint height_tex_id, uint normal_tex_id, uint material_id) override;
			void add_light(LightType type, const mat3& dirs, const vec3& color, bool cast_shadow) override;

			void draw_lines(uint lines_count, const Line3* lines) override;

			vec4 get_scissor() const override { return curr_scissor; }
			void set_scissor(const vec4& scissor) override;

			void add_blur(const vec4& range, uint radius) override;
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
