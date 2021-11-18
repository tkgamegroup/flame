#pragma once

#include "command.h"

namespace flame
{
	namespace graphics
	{
		struct InstanceCB : std::unique_ptr<CommandBufferT>
		{
			DevicePtr device;
			FencePtr fence;

			InstanceCB(DevicePtr device, FencePtr fence = nullptr) :
				device(device),
				fence(fence)
			{
				reset(CommandBuffer::create(CommandPool::get(device)));
				get()->begin(true);
			}

			~InstanceCB()
			{
				get()->end();
				auto q = Queue::get(device);
				q->submit1(get(), nullptr, nullptr, fence);
				if (!fence)
					q->wait_idle();
				else
					fence->wait();
			}
		};
	}
}
