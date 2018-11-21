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

#include "device_private.h"
#include "framebuffer_private.h"
#include "renderpass_private.h"
#include "image_private.h"

#include <algorithm>

namespace flame
{
	namespace graphics
	{
		inline FramebufferPrivate::FramebufferPrivate(Device *_d, const FramebufferInfo &_info)
		{
			d = (DevicePrivate*)_d;
			info = _info;

			Ivec2 size(0);

			std::vector<VkImageView> vk_views(info.views.size());
			for (auto i = 0; i < info.views.size(); i++)
			{
				if (size.x == 0 && size.y == 0)
					size = info.views[i]->image()->size;
				else
				{
					if (size != 1)
						assert(size == info.views[i]->image()->size);
				}

				vk_views[i] = ((ImageviewPrivate*)info.views[i])->v;
			}

			VkFramebufferCreateInfo create_info;
			create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			create_info.flags = 0;
			create_info.pNext = nullptr;
			create_info.width = size.x;
			create_info.height = size.y;
			create_info.layers = 1;
			create_info.renderPass = ((RenderpassPrivate*)info.rp)->v;
			create_info.attachmentCount = info.views.size();
			create_info.pAttachments = vk_views.data();

			vk_chk_res(vkCreateFramebuffer(d->v, &create_info, nullptr, &v));
		}

		inline FramebufferPrivate::~FramebufferPrivate()
		{
			vkDestroyFramebuffer(d->v, v, nullptr);
		}

		std::vector<FramebufferPrivate*> created_framebuffers;

		Framebuffer *Framebuffer::get(Device *d, const FramebufferInfo &info)
		{
			for (auto &f : created_framebuffers)
			{
				if (!(f->info.rp == info.rp && f->info.views == info.views))
					continue;
				f->ref_count++;
				return f;
			}

			auto f = new FramebufferPrivate(d, info);
			f->ref_count++;
			created_framebuffers.push_back(f);
			return f;
		}

		void Framebuffer::release(Framebuffer *f)
		{
			if (((FramebufferPrivate*)f)->ref_count == 1)
			{
				for (auto it = created_framebuffers.begin(); it != created_framebuffers.end(); it++)
				{
					if ((*it) == f)
					{
						created_framebuffers.erase(it);
						break;
					}
				}
				delete (FramebufferPrivate*)f;
			}
			else
				((FramebufferPrivate*)f)->ref_count--;
		}
	}
}

