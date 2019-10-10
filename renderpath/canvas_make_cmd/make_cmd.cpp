#include <flame/foundation/foundation.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/all.h>

#include "../canvas/type.h"

using namespace flame;
using namespace graphics;

namespace flame
{
	struct Vertex
	{
		Vec2f pos;
		Vec2f uv;
		Vec4c col;
	};

	enum CmdType
	{
		CmdDrawElement,
		CmdDrawTextLcd,
		CmdDrawTextSdf,
		CmdSetScissor,

		CmdDrawRect
	};

	struct CmdBase
	{
		CmdType type;
	};

	struct Cmd : CmdBase
	{
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

	struct Rect : CmdBase
	{
		Vec2f pos;
		Vec2f size;
		Vec4c color;
	};

	struct Rect$
	{
		AttributeV<Vec2f> pos$i;
		AttributeV<Vec2f> size$i;
		AttributeV<Vec4c> color$i;

		AttributeV<Rect> out$o;

		__declspec(dllexport) void update$()
		{
			out$o.v.type = CmdDrawRect;
			if (pos$i.frame > out$o.frame)
				out$o.v.pos = pos$i.v;
			if (size$i.frame > out$o.frame)
				out$o.v.size = size$i.v;
			if (color$i.frame > out$o.frame)
				out$o.v.color = color$i.v;
			out$o.frame = maxN(pos$i.frame, size$i.frame, color$i.frame);
		}
	};

	struct MakeCmd$
	{
		AttributeP<std::vector<void*>> cbs$i;
		AttributeP<void> vtx_buf$i;
		AttributeP<void> idx_buf$i;
		AttributeP<void> rnf$i;
		AttributeV<uint> image_idx$i;
		AttributeP<void> pll$i;
		AttributeP<void> pl_element$i;
		AttributeP<void> ds$i;

		AttributeP<std::vector<void*>> content$i;

		Vertex* vtx_end;
		uint* idx_end;

		std::vector<Cmd> cmds;
		Vec2f surface_size;
		Vec4f curr_scissor;

		int frame;

		__declspec(dllexport) MakeCmd$() :
			frame(-1)
		{
		}

		void begin_draw(CmdType type, uint id)
		{
			if (!cmds.empty())
			{
				auto& last = cmds.back();
				if (last.type == type && last.v.draw_data.id == id)
					return;
			}
			Cmd cmd;
			cmd.type = type;
			cmd.v.draw_data.id = id;
			cmd.v.draw_data.vtx_cnt = 0;
			cmd.v.draw_data.idx_cnt = 0;
			cmds.push_back(cmd);
		}

		void fill(const std::vector<Vec2f>& points, const Vec4c& col)
		{
			if (points.size() < 3)
				return;

			begin_draw(CmdDrawElement, 0);
			auto& vtx_cnt = cmds.back().v.draw_data.vtx_cnt;
			auto& idx_cnt = cmds.back().v.draw_data.idx_cnt;

			for (auto i = 0; i < points.size() - 2; i++)
			{
				vtx_end->pos = points[0];	  vtx_end->uv = Vec2f(0.5f); vtx_end->col = col; vtx_end++;
				vtx_end->pos = points[i + 2]; vtx_end->uv = Vec2f(0.5f); vtx_end->col = col; vtx_end++;
				vtx_end->pos = points[i + 1]; vtx_end->uv = Vec2f(0.5f); vtx_end->col = col; vtx_end++;

				*idx_end = vtx_cnt + 0; idx_end++;
				*idx_end = vtx_cnt + 1; idx_end++;
				*idx_end = vtx_cnt + 2; idx_end++;

				vtx_cnt += 3;
				idx_cnt += 3;
			}
		}

		__declspec(dllexport) void update$()
		{
			auto rnf = (RenderpassAndFramebuffer*)rnf$i.v;
			auto img_idx = image_idx$i.v;
			auto cb = (Commandbuffer*)(*cbs$i.v)[img_idx];
			auto vtx_buf = (Buffer*)vtx_buf$i.v;
			auto idx_buf = (Buffer*)idx_buf$i.v;
			auto pll = (Pipelinelayout*)pll$i.v;
			auto pl_element = (Pipeline*)pl_element$i.v;
			auto ds = (Descriptorset*)ds$i.v;

			if (frame == -1)
			{
				vtx_end = (Vertex*)vtx_buf->mapped;
				idx_end = (uint*)idx_buf->mapped;

				frame = 0;
			}

			if (rnf)
			{
				auto fb = (Framebuffer*)rnf->framebuffers()[img_idx];
				surface_size = Vec2f(fb->image_size);

				auto content = get_attribute_vec(content$i);
				for (auto& _c : content)
				{
					switch (((CmdBase*)_c)->type)
					{
					case CmdDrawRect:
					{
						auto c = (Rect*)_c;
						std::vector<Vec2f> points;
						path_rect(points, c->pos, c->size);
						fill(points, c->color);
					}
						break;
					}
				}

				cb->begin();
				cb->begin_renderpass(fb, rnf->clearvalues());
				if (idx_end != idx_buf->mapped)
				{
					curr_scissor = Vec4f(Vec2f(0.f), surface_size);
					cb->set_viewport(curr_scissor);
					cb->set_scissor(curr_scissor);
					cb->bind_vertexbuffer(vtx_buf, 0);
					cb->bind_indexbuffer(idx_buf, IndiceTypeUint);

					PushconstantT$ pc;
					pc.scale$ = Vec2f(2.f / surface_size.x(), 2.f / surface_size.y());
					pc.sdf_range$ = Vec2f(4.f / 512.f); /* sdf_image->size */
					cb->push_constant(0, sizeof(PushconstantT$), &pc, pll);
					cb->bind_descriptorset(ds, 0, pll);

					auto vtx_off = 0;
					auto idx_off = 0;
					for (auto& cmd : cmds)
					{
						switch (cmd.type)
						{
						case CmdDrawElement:
							if (cmd.v.draw_data.idx_cnt > 0)
							{
								cb->bind_pipeline(pl_element);
								cb->draw_indexed(cmd.v.draw_data.idx_cnt, idx_off, vtx_off, 1, cmd.v.draw_data.id);
								vtx_off += cmd.v.draw_data.vtx_cnt;
								idx_off += cmd.v.draw_data.idx_cnt;
							}
							break;
						}
					}
				}
				cb->end_renderpass();
				cb->end();

				vtx_end = (Vertex*)vtx_buf->mapped;
				idx_end = (uint*)idx_buf->mapped;
				cmds.clear();
			}
			else
			{
				cb->begin();
				cb->end();
			}
		}
	};
}

extern "C" __declspec(dllexport) bool package_check_update(BP::Package * p)
{
	return true;
}
