#pragma once

#include "buffer.h"
#include "command.h"
#include "../foundation/typeinfo.h"

namespace flame
{
	namespace graphics
	{
		struct StagingBuffer : std::unique_ptr<BufferT>
		{
			StagingBuffer(DevicePtr device, uint size, void* data = nullptr, BufferUsageFlags extra_usage = BufferUsageNone)
			{
				reset(Buffer::create(device, size, BufferUsageTransferSrc | BufferUsageTransferDst | extra_usage, MemoryPropertyHost | MemoryPropertyCoherent));
				get()->map();
				if (data)
					memcpy(get()->mapped, data, size);
			}
		};

		template<uint id, BufferUsageFlags usage, bool rewind = true>
		struct StorageBuffer : VirtualUdt<id>
		{
			using VirtualUdt<id>::ui;
			using VirtualUdt<id>::var_off;

			constexpr inline AccessFlags u2a(BufferUsageFlags u)
			{
				switch (u)
				{
				case BufferUsageVertex:
					return AccessVertexAttributeRead;
				case BufferUsageIndex:
					return AccessIndexRead;
				case BufferUsageIndirect:
					return AccessIndirectCommandRead;
				}
				return AccessNone;
			}

			constexpr inline PipelineStageFlags u2s(BufferUsageFlags u)
			{
				switch (u)
				{
				case BufferUsageVertex:
				case BufferUsageIndex:
					return PipelineStageVertexInput;
				case BufferUsageIndirect:
					return PipelineStageDrawIndirect;
				}
				return PipelineStageAllCommand;
			}

			uint size = 0;
			uint array_capacity = 0;

			std::unique_ptr<BufferT> buf;
			std::unique_ptr<BufferT> stagbuf;
			char* pbeg;
			char* pend;

			void create(uint _size, uint _array_capacity = 1)
			{
				size = _size;
				array_capacity = _array_capacity;
				buf.reset(Buffer::create(nullptr, array_capacity * size, BufferUsageTransferDst | usage, MemoryPropertyDevice));
				stagbuf.reset(Buffer::create(nullptr, buf->size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
				stagbuf->map();
				pbeg = pend = (char*)stagbuf->mapped;
			}

			void create(UdtInfo* _ui, uint _array_capacity = 1)
			{
				ui = _ui;
				create(ui->size, _array_capacity);
			}

			inline uint n_offset()
			{
				return (pend - (char*)stagbuf->mapped) / size;
			}

			inline void next_item()
			{
				pend += size;
			}

			inline void push(uint n, void* d)
			{
				auto s = n * size;
				memcpy(pend, d, s);
				pend += s;
			}

			template<typename T>
			inline void set_item(const T& v)
			{
				*(T*)pend = v;
			}

			template<uint nh, typename T>
			inline void set_var(const T& v)
			{
				auto offset = var_off<nh>();
				if (offset == -1)
					return;
				*(T*)(pend + offset) = v;
			}

			void upload(CommandBufferPtr cb)
			{
				if (array_capacity > 1)
				{
					BufferCopy cpy;
					cpy.size = pend - pbeg;
					cb->copy_buffer(stagbuf.get(), buf.get(), { &cpy, 1 });
					cb->buffer_barrier(buf.get(), AccessTransferWrite, u2a(usage), PipelineStageTransfer, u2s(usage));
					if (rewind)
						pend = pbeg;
					else
						pbeg = pend;
					return;
				}
			}
		};
	}
}
