// MIT License
// 
// Copyright (c) 2019 wjs
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
#include "synchronize_private.h"

namespace flame
{
	namespace graphics
	{
		SemaphorePrivate::SemaphorePrivate(Device *_d)
		{
			d = (DevicePrivate*)_d;
#if defined(FLAME_VULKAN)
			VkSemaphoreCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			vk_chk_res(vkCreateSemaphore(d->v, &info, nullptr, &v));
#endif
		}

		SemaphorePrivate::~SemaphorePrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroySemaphore(d->v, v, nullptr);
#endif
		}

		Semaphore *Semaphore::create(Device *d)
		{
			return new SemaphorePrivate(d);
		}

		void Semaphore::destroy(Semaphore *s)
		{
			delete (SemaphorePrivate*)s;
		}

		FencePrivate::FencePrivate(Device* _d)
		{
			d = (DevicePrivate*)_d;
#if defined(FLAME_VULKAN)

#elif defined(FLAME_D3D12)
			auto res = d->v->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&v));
			vl = 0;

			ev = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			assert(ev);
#endif
		}

		void FencePrivate::wait()
		{
			if (v->GetCompletedValue() < vl)
			{
				auto res = v->SetEventOnCompletion(vl, ev);
				assert(SUCCEEDED(res));

				WaitForSingleObject(ev, INFINITE);
		}
	}

		FencePrivate::~FencePrivate()
		{
#if defined(FLAME_VULKAN)

#elif defined(FLAME_D3D12)

#endif
		}

		Fence* Fence::create(Device* d)
		{
			return new FencePrivate(d);
		}

		void Fence::destroy(Fence* s)
		{
			delete (FencePrivate*)s;
		}

		void Fence::wait()
		{
			((FencePrivate*)this)->wait();
		}
	}
}

