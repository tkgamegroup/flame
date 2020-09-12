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
		struct ModelPrivate;
		struct FramebufferPrivate;
		struct DescriptorSetPrivate;
		struct CommandBufferPrivate;

		const auto resources_count = 64U;

		struct CanvasResourcePrivate : CanvasResource
		{
			std::string name;
			ImageViewPrivate* view;
			ImageAtlasPrivate* image_atlas = nullptr;
			FontAtlasPrivate* font_atlas = nullptr;

			const char* get_name() const override { return name.c_str(); }
			ImageView* get_view() const override { return view; }
			ImageAtlas* get_image_atlas() const override { return image_atlas; }
			FontAtlas* get_font_atlas() const override { return font_atlas; }
		};

		struct CanvasBridge : Canvas
		{
			void set_target(uint views_count, ImageView* const* views) override;

			uint set_resource(int slot, ImageView* v, Sampler* sp, const char* name) override;
			uint set_resource(int slot, ImageAtlas* image_atlas, const char* name) override;
			uint set_resource(int slot, FontAtlas* font_atlas, const char* name) override;

			uint bind_model(Model* model, const char* name) override;

			void record(CommandBuffer* cb, uint image_index) override;
		};

		template <class T, BufferUsageFlags U>
		struct TBuffer
		{
			DevicePrivate* d = nullptr;
			uint count = 0;
			std::unique_ptr<BufferPrivate> buf;
			std::unique_ptr<BufferPrivate> stg;
			T* end = nullptr;

			void init(DevicePrivate* _d, uint _count)
			{
				d = _d;
				count = _count;
				create();
			}

			void create()
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
				end = (T*)stg->mapped;
				if (temp)
				{
					push(n, temp);
					delete[]temp;
				}
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
				if (n >= count)
				{
					count *= 2;
					create();
				}
				*end = t;
				end++;
			}

			void push(uint cnt, const T* p)
			{
				auto n = stg_num();
				if (n + cnt >= count)
				{
					count = (n + cnt) * 2;
					create();
				}
				memcpy(end, p, sizeof(T) * cnt);
				end += cnt;
			}

			void upload(CommandBufferPrivate* cb, bool all = false)
			{
				BufferCopy cpy;
				cpy.size = all ? buf->size : (char*)end - stg->mapped;
				cb->copy_buffer(stg.get(), buf.get(), { &cpy, 1 });
			}

			void barrier(CommandBufferPrivate* cb)
			{
				cb->buffer_barrier(buf.get(), AccessTransferWrite, AccessVertexAttributeRead);
			}
		};

		struct CanvasPrivate : CanvasBridge
		{
			struct Cmd
			{
				enum Type
				{
					DrawElement,
					DrawMesh,
					SetScissor,
					Blur,
					Bloom
				};

				Type type;

				Cmd(Type t) : type(t) {}
				virtual ~Cmd() {}

				union
				{
					struct
					{
						uint count;
					}d3;
				}v;
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
				uint indiret_count = 0;

				CmdDrawMesh() : Cmd(DrawMesh) {}
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

			struct ElementVertex
			{
				Vec2f pos;
				Vec2f uv;
				Vec4c col;
			};

			struct BoundMaterial
			{
				Vec4f color;
				float metallic;
				float roughness;
				float alpha_test;
				float dummy0;
				int color_map_index = -1;
				int metallic_roughness_ao_map_index = -1;
				int normal_hegiht_map_index = -1;
				int dummy1;
			};

			struct BoundMesh
			{
				uint vtx_off;
				uint idx_off;
				uint idx_cnt;

				uint mat_idx;
			};

			struct BoundModel
			{
				std::string name;
				ModelPrivate* model;
				std::vector<uint> mat_idxs;
				std::vector<BoundMesh> meshes;
			};

			struct CameraData
			{
				Mat4f view;
				Mat4f proj;
				Mat4f proj_view;
				Vec4f coord;
				Vec4f dummy0;
				Vec4f dummy1;
				Vec4f dummy2;
			}; 

			struct MeshMatrix
			{
				Mat4f model;
				Mat4f mvp;
				Mat4f nor;
			};

			struct LightInfo
			{
				Vec4f col;
				Vec4f pos;
			};

			struct LightIndices
			{
				uint count;
				uint indices[1023];
			};

			DevicePrivate* device;

			bool hdr = false;

			Vec4c clear_color = Vec4c(0, 0, 0, 255);

			std::unique_ptr<CanvasResourcePrivate> resources[resources_count];
			std::vector<std::unique_ptr<ImagePrivate>> model_textures;
			std::vector<BoundModel> models;
			uint uploaded_models_count = 0;
			uint uploaded_vertices_count = 0;
			uint uploaded_indices_count = 0;
			uint uploaded_materials_count = 0;
			uint uploaded_model_textures_count = 0;

			std::unique_ptr<ImagePrivate> white_image;
			uint white_slot = 0;

			Mat4f project_view_matrix = Mat4f(1.f);

			TBuffer<ElementVertex, BufferUsageVertex> element_vertex_buffer;
			TBuffer<uint, BufferUsageIndex> element_index_buffer;
			TBuffer<ModelVertex1, BufferUsageVertex> model_vertex_buffer_1;
			TBuffer<uint, BufferUsageIndex> model_index_buffer;
			TBuffer<BoundMaterial, BufferUsageUniform> material_info_buffer;
			TBuffer<CameraData, BufferUsageUniform> camera_data_buffer;
			TBuffer<MeshMatrix, BufferUsageStorage> mesh_matrix_buffer;
			TBuffer<DrawIndexedIndirectCommand, BufferUsageIndirect> mesh_indirect_buffer;
			TBuffer<LightInfo, BufferUsageStorage> light_info_buffer;
			TBuffer<LightIndices, BufferUsageStorage> light_indices_buffer;

			std::unique_ptr<DescriptorSetPrivate> element_descriptorset;
			std::unique_ptr<DescriptorSetPrivate> forward_descriptorset;

			std::vector<ImageViewPrivate*> target_imageviews;
			std::vector<std::unique_ptr<FramebufferPrivate>> target_framebuffers;
			std::vector<std::unique_ptr<DescriptorSetPrivate>> target_nearest_descriptorsets;
			std::vector<std::unique_ptr<DescriptorSetPrivate>> target_linear_descriptorsets;

			std::unique_ptr<ImagePrivate> hdr_image;
			std::unique_ptr<FramebufferPrivate> hdr_framebuffer;
			std::unique_ptr<DescriptorSetPrivate> hdr_nearest_descriptorset;
			std::unique_ptr<DescriptorSetPrivate> hdr_linear_descriptorset;

			std::unique_ptr<ImagePrivate> depth_image;
			std::vector<std::unique_ptr<FramebufferPrivate>> forward8_framebuffers;
			std::unique_ptr<FramebufferPrivate> forward16_framebuffer;

			std::unique_ptr<ImagePrivate> back8_image;
			std::vector<std::unique_ptr<FramebufferPrivate>> back8_framebuffers;
			std::vector<std::unique_ptr<DescriptorSetPrivate>> back8_nearest_descriptorsets;
			std::vector<std::unique_ptr<DescriptorSetPrivate>> back8_linear_descriptorsets;

			std::unique_ptr<ImagePrivate> back16_image;
			std::vector<std::unique_ptr<FramebufferPrivate>> back16_framebuffers;
			std::vector<std::unique_ptr<DescriptorSetPrivate>> back16_nearest_descriptorsets;
			std::vector<std::unique_ptr<DescriptorSetPrivate>> back16_linear_descriptorsets;

			std::unique_ptr<DescriptorSetPrivate> image1_descriptorset;
			std::unique_ptr<DescriptorSetPrivate> image2_descriptorset;

			std::vector<std::vector<Vec2f>> paths;

			std::vector<std::unique_ptr<Cmd>> cmds;
			CmdDrawElement* last_element_cmd = nullptr;
			CmdDrawMesh* last_mesh_cmd = nullptr;

			Vec2u target_size;
			Vec4f curr_scissor;

			CanvasPrivate(DevicePrivate* d);

			void release() override { delete this; }

			void set_hdr(bool v) override { hdr = v; }

			Vec4c get_clear_color() const override { return clear_color; }
			void set_clear_color(const Vec4c& color) override { clear_color = color; }

			ImageView* get_target(uint idx) const override { return (ImageView*)target_imageviews[idx]; }
			void set_target(std::span<ImageViewPrivate*> views);

			CanvasResource* get_resource(uint slot) override { return slot < resources_count ? resources[slot].get() : nullptr; }
			uint set_resource(int slot, ImageViewPrivate* v, SamplerPrivate* sp, const std::string& name, ImageAtlasPrivate* image_atlas, FontAtlasPrivate* font_atlas);

			uint bind_model(ModelPrivate* model, const std::string& name);
			Model* get_model(uint idx) const override { return models[idx].model; }
			int find_model(const char* name) override;

			void add_draw_element_cmd(uint id);
			void add_vtx(const Vec2f& pos, const Vec2f& uv, const Vec4c& col);
			void add_idx(uint idx);

			void begin_path() override;
			void move_to(const Vec2f& pos) override;
			void line_to(const Vec2f& pos) override;
			void close_path() override;

			void stroke(const Vec4c& col, float thickness, bool aa = false) override;
			void fill(const Vec4c& col, bool aa = false) override;
			void draw_text(uint res_id, const wchar_t* text_beg, const wchar_t* text_end, uint font_size, const Vec4c& col, const Vec2f& pos, const Mat2f& axes) override;
			void draw_image(uint res_id, uint tile_id, const Vec2f& LT, const Vec2f& RT, const Vec2f& RB, const Vec2f& LB, const Vec2f& uv0, const Vec2f& uv1, const Vec4c& tint_col) override;

			void set_camera(const Mat4f& proj, const Mat4f& view, const Vec3f& coord) override;

			void draw_mesh(uint mod_id, uint mesh_idx, const Mat4f& model, const Mat4f& normal) override;
			void add_light(LightType type, const Vec3f& color, const Vec3f& pos) override;

			Vec4f get_scissor() const override { return curr_scissor; }
			void set_scissor(const Vec4f& scissor) override;

			void add_blur(const Vec4f& range, uint radius) override;
			void add_bloom() override;

			void prepare() override;

			void record(CommandBufferPrivate* cb, uint image_index);
		};

		inline void CanvasBridge::set_target(uint views_count, ImageView* const* views)
		{
			((CanvasPrivate*)this)->set_target({ (ImageViewPrivate**)views, views_count });
		}

		inline uint CanvasBridge::set_resource(int slot, ImageView* v, Sampler* sp, const char* name)
		{
			return ((CanvasPrivate*)this)->set_resource(slot, (ImageViewPrivate*)v, (SamplerPrivate*)sp, name ? name : "", nullptr, nullptr);
		}

		inline uint CanvasBridge::set_resource(int slot, ImageAtlas* image_atlas, const char* name)
		{
			return ((CanvasPrivate*)this)->set_resource(slot, ((ImageAtlasPrivate*)image_atlas)->image->default_views[0].get(), 
				((ImageAtlasPrivate*)image_atlas)->border ? ((CanvasPrivate*)this)->device->sampler_linear.get() : ((CanvasPrivate*)this)->device->sampler_nearest.get(), 
				name ? name : "", (ImageAtlasPrivate*)image_atlas, nullptr);
		}

		inline uint CanvasBridge::set_resource(int slot, FontAtlas* font_atlas, const char* name)
		{
			return ((CanvasPrivate*)this)->set_resource(slot, ((FontAtlasPrivate*)font_atlas)->view.get(), ((CanvasPrivate*)this)->device->sampler_nearest.get(), 
				name ? name : "", nullptr, (FontAtlasPrivate*)font_atlas);
		}

		inline uint CanvasBridge::bind_model(Model* model, const char* name)
		{
			return ((CanvasPrivate*)this)->bind_model((ModelPrivate*)model, name);
		}

		inline void CanvasBridge::record(CommandBuffer* cb, uint image_index)
		{
			((CanvasPrivate*)this)->record((CommandBufferPrivate*)cb, image_index);
		}
	}
}
