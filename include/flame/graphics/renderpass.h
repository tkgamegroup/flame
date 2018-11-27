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

#include <flame/graphics/graphics.h>

#include <flame/array.h>

#include <vector>

namespace flame
{
	namespace graphics
	{
		struct Device;

		struct AttachmentInfo : R
		{
			Format format$;
			bool clear$;
			SampleCount sample_count$;

			inline AttachmentInfo()
			{
				format$ = Format_R8G8B8A8_UNORM;
				clear$ = true;
				sample_count$ = SampleCount_1;
			}

			inline AttachmentInfo(Format format, bool clear = true, SampleCount sample_count = SampleCount_1) :
				format$(format),
				clear$(clear),
				sample_count$(sample_count)
			{
			}
		};

		struct SubpassInfo
		{
			std::vector<int> color_attachments;
			std::vector<int> resolve_attachments;
			int depth_attachment;

			inline SubpassInfo() :
				depth_attachment(-1)
			{
			}

			inline SubpassInfo(const std::vector<int> &_color_attachments, int _depth_attachment = -1, const std::vector<int> &_resolve_attachments = std::vector<int>()) :
				color_attachments(_color_attachments),
				depth_attachment(_depth_attachment),
				resolve_attachments(_resolve_attachments)
			{
			}
		};

		struct DependencyInfo
		{
			int src_subpass;
			int dst_subpass;
		};

		struct RenderpassInfo
		{
			std::vector<AttachmentInfo> attachments;
			std::vector<SubpassInfo> subpasses;
			std::vector<DependencyInfo> dependencies;

			inline RenderpassInfo()
			{
				SubpassInfo sp;
				subpasses.push_back(sp);
			}
		};

		struct Renderpass
		{
			FLAME_GRAPHICS_EXPORTS static Renderpass *get(Device *d, const RenderpassInfo &info);
			FLAME_GRAPHICS_EXPORTS static void release(Renderpass *r);
		};

		struct ClearValues
		{
			FLAME_GRAPHICS_EXPORTS void set(int idx, const Bvec4 &col);

			FLAME_GRAPHICS_EXPORTS static ClearValues *create(Renderpass *r);
			FLAME_GRAPHICS_EXPORTS static void destroy(ClearValues *c);
		};
	}
}

