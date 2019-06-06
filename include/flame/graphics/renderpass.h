// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <flame/foundation/foundation.h>
#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Imageview;

		struct AttachmentInfo$
		{
			Format$ format$;
			bool clear$;
			SampleCount$ sample_count$;

			AttachmentInfo$()
			{
				format$ = Format_R8G8B8A8_UNORM;
				clear$ = true;
				sample_count$ = SampleCount_1;
			}

			AttachmentInfo$(Format$ format, bool clear = true, SampleCount$ sample_count = SampleCount_1) :
				format$(format),
				clear$(clear),
				sample_count$(sample_count)
			{
			}
		};

		struct SubpassInfo$
		{
			LNA<int> color_attachments$;
			LNA<int> resolve_attachments$;
			int depth_attachment$;

			SubpassInfo$() :
				depth_attachment$(-1)
			{
				memset(&color_attachments$, 0, sizeof(LNA<int>));
				memset(&resolve_attachments$, 0, sizeof(LNA<int>));
			}
		};

		struct DependencyInfo$
		{
			int src_subpass$;
			int dst_subpass$;
		};

		struct RenderpassInfo$
		{
			LNA<AttachmentInfo$*> attachments$;
			LNA<SubpassInfo$*> subpasses$;
			LNA<DependencyInfo$*> dependencies$;

			RenderpassInfo$()
			{
				memset(&attachments$, 0, sizeof(LNA<AttachmentInfo$*>));
				memset(&subpasses$, 0, sizeof(LNA<SubpassInfo$*>));
				memset(&dependencies$, 0, sizeof(LNA<DependencyInfo$*>));
			}
		};

		struct Renderpass
		{
			FLAME_GRAPHICS_EXPORTS int attachment_count() const;

			FLAME_GRAPHICS_EXPORTS static Renderpass* create(Device *d, const RenderpassInfo$ &info);
			FLAME_GRAPHICS_EXPORTS static void destroy(Renderpass *r);
		};

		struct Renderpass$
		{
			void* in$i;

			void* out$o;

			FLAME_GRAPHICS_EXPORTS void initialize$c();
			FLAME_GRAPHICS_EXPORTS void finish$c();
			FLAME_GRAPHICS_EXPORTS void update$c();
		};

		struct Clearvalues
		{
			FLAME_GRAPHICS_EXPORTS Renderpass* renderpass() const;

			FLAME_GRAPHICS_EXPORTS void set(int idx, const Vec4c &col);

			FLAME_GRAPHICS_EXPORTS static Clearvalues* create(Renderpass* r);
			FLAME_GRAPHICS_EXPORTS static void destroy(Clearvalues* c);
		};

		struct Clearvalues$
		{
			void* in$i;
			void* renderpass$i;
			LNA<Vec4c> colors$i;

			void* out$o;

			FLAME_GRAPHICS_EXPORTS void initialize$c();
			FLAME_GRAPHICS_EXPORTS void finish$c();
			FLAME_GRAPHICS_EXPORTS void update$c();
		};

		struct FramebufferInfo
		{
			Renderpass* rp;
			std::vector<Imageview*> views;
		};

		struct Framebuffer$
		{
			void* in$i;

			void* out$o;

			FLAME_GRAPHICS_EXPORTS void initialize$c();
			FLAME_GRAPHICS_EXPORTS void finish$c();
			FLAME_GRAPHICS_EXPORTS void update$c();
		};

		struct Framebuffer
		{
			FLAME_GRAPHICS_EXPORTS static Framebuffer* create(Device* d, const FramebufferInfo& info);
			FLAME_GRAPHICS_EXPORTS static void destroy(Framebuffer* f);
		};
	}
}

