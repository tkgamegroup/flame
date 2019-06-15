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
			chk_res(vkCreateSemaphore(d->v, &info, nullptr, &v));
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
			VkFenceCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			chk_res(vkCreateFence(d->v, &info, nullptr, &v));
#elif defined(FLAME_D3D12)
			auto res = d->v->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&v));
			vl = 0;

			ev = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			assert(ev);
#endif
		}

		FencePrivate::~FencePrivate()
		{
#if defined(FLAME_VULKAN)

#elif defined(FLAME_D3D12)

#endif
		}

		void FencePrivate::wait()
		{
#if defined(FLAME_VULKAN)
			chk_res(vkWaitForFences(d->v, 1, &v, true, UINT64_MAX));
			chk_res(vkResetFences(d->v, 1, &v));
#elif defined(FLAME_D3D12)
			if (v->GetCompletedValue() < vl)
			{
				auto res = v->SetEventOnCompletion(vl, ev);
				assert(SUCCEEDED(res));

				WaitForSingleObject(ev, INFINITE);
			}
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

