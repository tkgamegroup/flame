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
		struct FramebufferPrivate;
		struct DescriptorSetPrivate;
		struct CommandBufferPrivate;

		const auto resources_count = 64U;
		const auto downsample_level = 6U;

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
				CmdBlur
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
						Mat4f matrix;
					}d3;
				}v;
			};

			struct ElementVertex
			{
				Vec2f pos;
				Vec2f uv;
				Vec4c col;
			};

			struct ModelVertex
			{
				Vec3f pos;
				Vec2f uv;
				Vec3f normal;
				Vec3f tangent;
			};

			struct AddedMesh
			{
				uint idx_off;
				uint vtx_off;
				uint vtx_cnt;
				uint idx_cnt;
			};

			struct AddedModel
			{
				std::string name;
				std::vector<AddedMesh> meshes;
			};

			DevicePrivate* device;

			Vec4c clear_color = Vec4c(0, 0, 0, 255);

			std::vector<std::unique_ptr<CanvasResourcePrivate>> resources;
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
			std::unique_ptr<BufferPrivate> object_matrix_buffer;
			std::unique_ptr<BufferPrivate> object_matrix_staging_buffer;
			std::unique_ptr<BufferPrivate> object_indirect_buffer;
			std::unique_ptr<BufferPrivate> object_indirect_staging_buffer;
			std::unique_ptr<DescriptorSetPrivate> element_descriptorset;

			ElementVertex* element_vertex_buffer_end;
			uint* element_index_buffer_end;
			Mat4f* object_matrix_buffer_end;
			DrawIndexedIndirectCommand* object_indirect_buffer_end;

			std::vector<ImageViewPrivate*> target_imageviews;
			std::vector<std::unique_ptr<FramebufferPrivate>> target_framebuffers;
			std::vector<std::unique_ptr<DescriptorSetPrivate>> target_descriptors;

			std::unique_ptr<ImagePrivate> depth_image;
			std::vector<std::unique_ptr<FramebufferPrivate>> forward_framebuffers;

			std::unique_ptr<ImagePrivate> back_images[2];
			std::unique_ptr<ImageViewPrivate> back_imageviews[2][downsample_level];
			std::unique_ptr<FramebufferPrivate> back_framebuffers[2][downsample_level];
			std::unique_ptr<DescriptorSetPrivate> back_descriptorsets[2][downsample_level];

			std::vector<std::vector<Vec2f>> paths;

			std::vector<Cmd> cmds;

			Vec2u target_size;
			Vec4f curr_scissor;

			CanvasPrivate(DevicePrivate* d);

			void release() override { delete this; }

			Vec4c get_clear_color() const override { return clear_color; }
			void set_clear_color(const Vec4c& color) override { clear_color = color; }

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

			void add_object(const Mat4f& mat, uint mod_id) override;

			void add_blur(const Vec4f& range, uint radius) override;

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
			return ((CanvasPrivate*)this)->set_resource(slot, ((ImageAtlasPrivate*)image_atlas)->image->default_view.get(), 
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
