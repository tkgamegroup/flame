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

			void record(CommandBuffer* cb, uint image_index) override;
		};

		struct CanvasPrivate : CanvasBridge
		{
			enum CmdType
			{
				CmdDrawElement,
				CmdSetScissor,
				CmdDrawObject,
				CmdBlur,
				CmdBloom
			};

			struct Cmd
			{
				CmdType type;
				union
				{
					struct
					{
						uint id;
						uint vtx_cnt;
						uint idx_cnt;
					}d1;
					struct
					{
						Vec4f scissor;
						uint radius;
					}d2;
					struct
					{
						uint count;
					}d3;
				}v;
			};

			struct ElementVertex
			{
				Vec2f pos;
				Vec2f uv;
				Vec4c col;
			};

			struct BoundMaterial
			{
				Vec4f albedo;
				Vec4f spec_roughness;
				uint albedo_map_index;
				uint spec_map_index;
				uint roughness_map_index;
				uint normal_map_index;
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
				std::vector<BoundMesh> meshes;
			};

			struct ObjectMatrix
			{
				Mat4f mvp;
				Mat4f nor;
			};

			struct LightInfo
			{
				Vec4f col;
				Vec4f pos;
			};

			DevicePrivate* device;

			bool hdr = false;

			Vec4c clear_color = Vec4c(0, 0, 0, 255);

			std::unique_ptr<CanvasResourcePrivate> resources[resources_count];
			std::vector<BoundModel> models;
			std::vector<std::unique_ptr<Image>> model_textures;
			uint uploaded_models_count = 0;
			uint uploaded_vertices_count = 0;
			uint uploaded_indices_count = 0;
			uint uploaded_model_textures_count = 0;
			uint uploaded_materials_count = 0;

			std::unique_ptr<ImagePrivate> white_image;
			uint white_slot = 0;

			std::unique_ptr<BufferPrivate> element_vertex_buffer;
			std::unique_ptr<BufferPrivate> element_vertex_staging_buffer;
			std::unique_ptr<BufferPrivate> element_index_buffer;
			std::unique_ptr<BufferPrivate> element_index_staging_buffer;
			std::unique_ptr<BufferPrivate> model_vertex_buffer;
			std::unique_ptr<BufferPrivate> model_vertex_staging_buffer;
			std::unique_ptr<BufferPrivate> model_index_buffer;
			std::unique_ptr<BufferPrivate> model_index_staging_buffer;
			std::unique_ptr<BufferPrivate> material_info_buffer;
			std::unique_ptr<BufferPrivate> material_info_staging_buffer;
			std::unique_ptr<BufferPrivate> object_matrix_buffer;
			std::unique_ptr<BufferPrivate> object_matrix_staging_buffer;
			std::unique_ptr<BufferPrivate> object_indirect_buffer;
			std::unique_ptr<BufferPrivate> object_indirect_staging_buffer;
			std::unique_ptr<BufferPrivate> light_info_buffer;
			std::unique_ptr<BufferPrivate> light_info_staging_buffer;
			std::unique_ptr<DescriptorSetPrivate> element_descriptorset;
			std::unique_ptr<DescriptorSetPrivate> forward_descriptorset;

			ElementVertex*				element_vertex_buffer_end;
			uint*						element_index_buffer_end;
			ObjectMatrix*				object_matrix_buffer_end;
			DrawIndexedIndirectCommand* object_indirect_buffer_end;
			LightInfo*					light_info_buffer_end;

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

			std::vector<std::vector<Vec2f>> paths;

			std::vector<Cmd> cmds;

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

			void add_draw_cmd(uint id);
			void add_vtx(const Vec2f& pos, const Vec2f& uv, const Vec4c& col);
			void add_idx(uint idx);

			void begin_path() override;
			void move_to(const Vec2f& pos) override;
			void line_to(const Vec2f& pos) override;
			void close_path() override;

			void stroke(const Vec4c& col, float thickness, bool aa = false) override;
			void fill(const Vec4c& col, bool aa = false) override;
			void add_text(uint res_id, const wchar_t* text_beg, const wchar_t* text_end, uint font_size, const Vec4c& col, const Vec2f& pos, const Mat2f& axes) override;
			void add_image(uint res_id, uint tile_id, const Vec2f& LT, const Vec2f& RT, const Vec2f& RB, const Vec2f& LB, const Vec2f& uv0, const Vec2f& uv1, const Vec4c& tint_col) override;

			void add_object(uint mod_id, const Mat4f& mvp, const Mat4f& nor) override;
			void add_light(LightType type, const Vec3f& color, const Vec3f& pos) override;

			void add_blur(const Vec4f& range, uint radius) override;
			void add_bloom() override;

			Vec4f get_scissor() const override { return curr_scissor; }
			void set_scissor(const Vec4f& scissor) override;

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

		inline void CanvasBridge::record(CommandBuffer* cb, uint image_index)
		{
			((CanvasPrivate*)this)->record((CommandBufferPrivate*)cb, image_index);
		}
	}
}
