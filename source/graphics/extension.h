#pragma once

#include "../foundation/typeinfo.h"
#include "command.h"
#include "buffer.h"

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
			using VirtualUdt<id>::set_var;

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
				size = usage == BufferUsageIndirect ? sizeof(graphics::DrawIndexedIndirectCommand) : _size;
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

			void create_with_array_type(UdtInfo* _ui)
			{
				auto& vi = _ui->variables[0];
				create(vi.type->retrive_ui(), vi.array_size);
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
				VirtualUdt<id>::set_var<nh>(pend, v);
			}

			inline void add_draw_indirect(uint vertex_count, uint first_vertex = 0, uint instance_count = 1, uint first_instance = 0)
			{
				DrawIndirectCommand c;
				c.vertex_count = vertex_count;
				c.instance_count = instance_count;
				c.first_vertex = first_vertex;
				c.first_instance = first_instance;
				set_item(c);
				next_item();
			}

			inline void add_draw_indexed_indirect(uint index_count, uint first_index = 0, int vertex_offset = 0, uint instance_count = 1, uint first_instance = 0)
			{
				DrawIndexedIndirectCommand c;
				c.index_count = index_count;
				c.instance_count = instance_count;
				c.first_index = first_index;
				c.vertex_offset = vertex_offset;
				c.first_instance = first_instance;
				set_item(c);
				next_item();
			}

			void upload(CommandBufferPtr cb)
			{
				BufferCopy cpy;
				if (array_capacity > 1)
				{
					cpy.size = pend - pbeg;
					if (rewind)
						pend = pbeg;
					else
						pbeg = pend;
				}
				else
					cpy.size = size;
				if (cpy.size > 0)
				{
					cb->copy_buffer(stagbuf.get(), buf.get(), { &cpy, 1 });
					cb->buffer_barrier(buf.get(), AccessTransferWrite, u2a(usage), PipelineStageTransfer, u2s(usage));
				}
			}
		};

		template<uint id>
		struct PipelineResourceManager
		{
			PipelineLayoutPtr pll = nullptr;
			std::unordered_map<uint, int> dsl_map;
			VirtualUdt<id> vu_pc;

			DescriptorSetPtr temp_dss[8];
			char temp_pc[256];

			void init(PipelineLayoutPtr _pll)
			{
				pll = _pll;
				for (auto i = 0; i < pll->dsls.size(); i++)
				{
					auto dsl = pll->dsls[i];

					std::string name;
					if (dsl->filename != pll->filename)
						name = dsl->filename.stem().string();
					else
						name = "";
					dsl_map.emplace(sh(name.c_str()), i);
				}
				vu_pc.ui = pll->pc_ui;

				memset(temp_dss, 0, sizeof(temp_dss));
				memset(temp_pc, 0, sizeof(temp_pc));
			}

			inline int dsl_idx(uint nh)
			{
				auto it = dsl_map.find(nh);
				if (it == dsl_map.end())
				{
					assert(0);
					return -1;
				}
				return it->second;
			}

			inline DescriptorSetLayoutPtr get_dsl(uint nh)
			{
				auto idx = dsl_idx(nh);
				return idx == -1 ? nullptr : pll->dsls[idx];
			}

			inline void set_ds(uint nh, DescriptorSetPtr ds)
			{
				auto idx = dsl_idx(nh);
				if (idx != -1)
					temp_dss[idx] = ds;
			}

			inline void bind_dss(CommandBufferPtr cb, uint off = 0, uint count = 0xffffffff)
			{
				for (auto i = 0U; i < count; i++)
				{
					auto ii = off + i;
					if (ii >= dsl_map.size() || ii >= _countof(temp_dss) || !temp_dss[ii])
					{
						count = i;
						break;
					}
				}
				cb->bind_descriptor_sets(off, { temp_dss + off, count });
			}

			template<uint nh, typename T>
			inline void set_pc_var(const T& v)
			{
				vu_pc.set_var<nh>(temp_pc, v);
			}

			inline void push_constant(CommandBufferPtr cb, uint off = 0, uint size = 0xffffffff)
			{
				cb->push_constant(0, min(size, pll->pc_sz), temp_pc);
			}
		};
	}
}
