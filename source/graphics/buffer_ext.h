#pragma once

#include "buffer.h"
#include "command.h"

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

		template<fixed_string id, BufferUsageFlags usage, bool rewind = true>
		struct StorageBuffer
		{
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

			UdtInfo* ui = nullptr;

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

			template<fixed_string n, typename T>
			inline void set_var(const T& v)
			{
				auto get_offset = [&]()->int {
					auto vi = ui->find_variable((char const*)n);
					if (!vi)
					{
						assert(0);
						return -1;
					}
					assert(vi->type == TypeInfo::get<T>());
					return vi->offset;
				};
				static int offset = get_offset();
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
