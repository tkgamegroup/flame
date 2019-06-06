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

		struct AttachmentInfo
		{
			Format$ format;
			bool clear;
			SampleCount$ sample_count;

			AttachmentInfo() :
				format(Format_R8G8B8A8_UNORM),
				clear(true),
				sample_count(SampleCount_1)
			{
			}
		};

		struct AttachmentInfo$
		{
			Format$ format$i;
			bool clear$i;
			SampleCount$ sample_count$i;

			void* out$o;

			AttachmentInfo$() :
				format$i(Format_R8G8B8A8_UNORM),
				clear$i(true),
				sample_count$i(SampleCount_1)
			{
			}

			FLAME_GRAPHICS_EXPORTS void initialize$();
			FLAME_GRAPHICS_EXPORTS void finish$();
			FLAME_GRAPHICS_EXPORTS void update$();
		};

		struct SubpassInfo
		{
			LNA<int> color_attachments;
			LNA<int> resolve_attachments;
			int depth_attachment;

			SubpassInfo() :
				depth_attachment(-1)
			{
				memset(&color_attachments, 0, sizeof(LNA<int>));
				memset(&resolve_attachments, 0, sizeof(LNA<int>));
			}
		};

		struct SubpassInfo$
		{
			LNA<int> color_attachments$i;
			LNA<int> resolve_attachments$i;
			int depth_attachment$i;

			void* out$o;

			SubpassInfo$() :
				depth_attachment$i(-1)
			{
			}

			FLAME_GRAPHICS_EXPORTS void initialize$();
			FLAME_GRAPHICS_EXPORTS void finish$();
			FLAME_GRAPHICS_EXPORTS void update$();
		};

		struct RenderpassInfo
		{
			LNA<AttachmentInfo*> attachments;
			LNA<SubpassInfo*> subpasses;
			LNA<Vec<2, uint>> dependencies;

			RenderpassInfo()
			{
				memset(&attachments, 0, sizeof(LNA<AttachmentInfo*>));
				memset(&subpasses, 0, sizeof(LNA<SubpassInfo*>));
				memset(&dependencies, 0, sizeof(LNA<Vec<2, uint>>));
			}
		};

		struct Renderpass
		{
			FLAME_GRAPHICS_EXPORTS int attachment_count() const;

			FLAME_GRAPHICS_EXPORTS static Renderpass* create(Device *d, const RenderpassInfo& info);
			FLAME_GRAPHICS_EXPORTS static void destroy(Renderpass *r);
		};

		struct Renderpass$
		{
			void* device$i;
			LNA<AttachmentInfo*> attachments$;
			LNA<SubpassInfo*> subpasses$;
			LNA<Vec<2, uint>> dependencies$;

			void* out$o;

			FLAME_GRAPHICS_EXPORTS void initialize$();
			FLAME_GRAPHICS_EXPORTS void finish$();
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
			void* device$i;
			void* renderpass$i;
			LNA<Vec4c> colors$i;

			void* out$o;

			FLAME_GRAPHICS_EXPORTS void initialize$();
			FLAME_GRAPHICS_EXPORTS void finish$();
			FLAME_GRAPHICS_EXPORTS void update$();
		};

		struct FramebufferInfo
		{
			Renderpass* rp;
			std::vector<Imageview*> views;
		};

		struct Framebuffer$
		{
			void* device$i;

			void* out$o;

			FLAME_GRAPHICS_EXPORTS void initialize$();
			FLAME_GRAPHICS_EXPORTS void finish$();
			FLAME_GRAPHICS_EXPORTS void update$();
		};

		struct Framebuffer
		{
			FLAME_GRAPHICS_EXPORTS static Framebuffer* create(Device* d, const FramebufferInfo& info);
			FLAME_GRAPHICS_EXPORTS static void destroy(Framebuffer* f);
		};
	}
}

