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

		struct CanvasResourcePrivate : CanvasResource
		{
			std::filesystem::path filename;
			ImageViewPrivate* view;
			ImageAtlasPrivate* image_atlas = nullptr;
			FontAtlasPrivate* font_atlas = nullptr;
			Vec2f white_uv = Vec2f(0.5f);

			const wchar_t* get_filename() const override { return filename.c_str(); }
			ImageView* get_view() const override { return view; }
			ImageAtlas* get_image_atlas() const override { return image_atlas; }
			FontAtlas* get_font_atlas() const override { return font_atlas; }
			Vec2f get_white_uv() const override { return white_uv; }
		};

		struct CanvasBridge : Canvas
		{
			void set_target(uint views_count, ImageView* const* views) override;

			uint set_resource(int slot, ImageView* v, Sampler* sp, const wchar_t* filename, ImageAtlas* image_atlas, FontAtlas* font_atlas) override;
			void add_atlas(ImageAtlas* a) override;
			void add_font(FontAtlas* f) override;

			void stroke(const Vec4c& col, float thickness, bool aa) override;
			void fill(const Vec4c& col, bool aa) override;
			void add_text(uint res_id, const wchar_t* text, uint size, const Vec2f& pos, const Vec4c& col) override;
			void add_image(uint res_id, uint tile_id, const Vec2f& pos, const Vec2f& size, const Vec2f& uv0, const Vec2f& uv1, const Vec4c& tint_col) override;

			void record(CommandBuffer* cb, uint image_index) override;
		};

		struct CanvasPrivate : CanvasBridge
		{
			enum CmdType
			{
				CmdDrawElement,
				CmdSetScissor
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
					}draw_data;
					Vec4f scissor;
				}v;
			};

			struct Vertex
			{
				Vec2f pos;
				Vec2f uv;
				Vec4c col;
			};

			DevicePrivate* device;

			Vec4f clear_color = Vec4f(0.f, 0.f, 0.f, 1.f);

			std::unique_ptr<ImagePrivate> img_white;
			std::vector<std::unique_ptr<CanvasResourcePrivate>> resources;
			std::unique_ptr<BufferPrivate> buf_vtx;
			std::unique_ptr<BufferPrivate> buf_idx;
			std::vector<std::unique_ptr<FramebufferPrivate>> fbs;
			std::unique_ptr<DescriptorSetPrivate> ds;

			std::vector<std::vector<Vec2f>> paths;

			Vertex* vtx_end;
			uint* idx_end;

			std::vector<Cmd> cmds;
			uint* p_vtx_cnt;
			uint* p_idx_cnt;

			Vec2u target_size;
			Vec4f curr_scissor;

			CanvasPrivate(DevicePrivate* d);

			void release() override { delete this; }

			Vec4f get_clear_color() const override { return clear_color; }
			void set_clear_color(const Vec4c& color) override { clear_color = Vec4f(color) / 255.f; }

			void set_target(std::span<ImageViewPrivate*> views);

			CanvasResource* get_resource(uint slot) override { return slot < resources_count ? resources[slot].get() : nullptr; }
			uint set_resource(int slot, ImageViewPrivate* v, SamplerPrivate* sp = nullptr, const std::filesystem::path& filename = "", ImageAtlasPrivate* image_atlas = nullptr, FontAtlasPrivate* font_atlas = nullptr);
			void add_atlas(ImageAtlasPrivate* a);
			void add_font(FontAtlasPrivate* f);

			void add_draw_cmd(int id = -1);
			void add_vtx(const Vec2f& pos, const Vec2f& uv, const Vec4c& col);
			void add_idx(uint idx);

			void begin_path() override;
			void move_to(float x, float y) override;
			void line_to(float x, float y) override;
			void close_path() override;

			void stroke(const Vec4c& col, float thickness, bool aa = false);
			void fill(const Vec4c& col, bool aa = false);
			void add_text(uint res_id, const wchar_t* text, uint font_size, const Vec2f& pos, const Vec4c& col);
			void add_image(uint res_id, uint tile_id, const Vec2f& pos, const Vec2f& size, Vec2f uv0, Vec2f uv1, const Vec4c& tint_col);

			Vec4f get_scissor() const override { return curr_scissor; }
			void set_scissor(const Vec4f& scissor) override;

			void prepare() override;

			void record(CommandBufferPrivate* cb, uint image_index);
		};
	}
}
