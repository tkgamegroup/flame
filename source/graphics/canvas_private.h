#pragma once

#include "canvas.h"
#include "buffer_private.h"
#include "shader_private.h"
#include "command_private.h"
#include "extension.h"

namespace flame
{
	namespace graphics
	{
		struct CanvasPrivate : Canvas
		{
			struct DrawVert
			{
				vec2  pos;
				vec2  uv;
				cvec4 col;
			};

			struct DrawCmd
			{
				uint idx_cnt;
				DescriptorSetPtr ds;
			};

			GraphicsPipelinePtr pl = nullptr;
			RenderpassPtr rp = nullptr;
			RenderpassPtr rp_load = nullptr;
			VertexBuffer buf_vtx;
			IndexBuffer<> buf_idx;
			FontAtlasPtr main_font = nullptr;
			ImagePtr main_img = nullptr;
			std::unique_ptr<DescriptorSetT> main_ds;

			std::vector<DrawCmd> draw_cmds;
			std::vector<vec2> path;
			std::stack<FontAtlasPtr> fonts;
			uint font_size = 14;

			CanvasPrivate(WindowPtr window);
			~CanvasPrivate();
			void reset();
			DrawCmd& get_cmd(DescriptorSetPtr ds);

			void path_rect(const vec2& a, const vec2& b);
			void stroke(float thickness, const cvec4& col, bool closed);
			void fill(const cvec4& col);

			void add_rect(const vec2& a, const vec2& b, float thickness, const cvec4& col) override;
			void add_rect_filled(const vec2& a, const vec2& b, const cvec4& col) override;
			void push_font(FontAtlasPtr font) override;
			void pop_font() override;
			void add_text(const vec2& pos, std::wstring_view str, const cvec4& col) override;
		};
	}
}
