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

#include <flame/foundation/window.h>

using namespace flame;

ID3D12DescriptorHeap* descriptor_heap;
ID3D12CommandAllocator* command_allocator;
ID3D12GraphicsCommandList* command_lists[2];
ID3D12Fence* fence;
HANDLE fence_event;
uint fence_value;

D3D12_RESOURCE_BARRIER dx_barrier_transition(ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
{
	D3D12_RESOURCE_BARRIER ret = {};
	ret.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	ret.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	ret.Transition.pResource = pResource;
	ret.Transition.StateBefore = stateBefore;
	ret.Transition.StateAfter = stateAfter;
	ret.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	return ret;
}

int main(int argc, char** args)
{
	auto app = Application::create();
	auto w = Window::create(app, "DX Test", Ivec2(1280, 720), WindowFrame);

	D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {};
	descriptor_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptor_heap_desc.NumDescriptors = 2;
	descriptor_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	res = device->CreateDescriptorHeap(&descriptor_heap_desc, IID_PPV_ARGS(&descriptor_heap));
	assert(SUCCEEDED(res));

	auto descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	auto descriptor_handle = descriptor_heap->GetCPUDescriptorHandleForHeapStart();

	for (auto i = 0; i < 2; i++)
	{
		res = swapchain->GetBuffer(i, IID_PPV_ARGS(&backbuffers[i]));
		assert(SUCCEEDED(res));

		device->CreateRenderTargetView(backbuffers[i], nullptr, descriptor_handle);

		descriptor_handle.ptr += descriptor_size;
	}

	res = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator));
	assert(SUCCEEDED(res));

	for (auto i = 0; i < 2; i++)
	{
		res = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator, nullptr, IID_PPV_ARGS(&command_lists[i]));
		assert(SUCCEEDED(res));

		auto command_list = command_lists[i];

		command_list->ResourceBarrier(1, &dx_barrier_transition(backbuffers[i], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		auto descriptor_handle = descriptor_heap->GetCPUDescriptorHandleForHeapStart();
		descriptor_handle.ptr += i * descriptor_size;

		command_list->OMSetRenderTargets(1, &descriptor_handle, false, nullptr);

		const float clear_color[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		command_list->ClearRenderTargetView(descriptor_handle, clear_color, 0, nullptr);

		command_list->ResourceBarrier(1, &dx_barrier_transition(backbuffers[i], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		res = command_list->Close();
		assert(SUCCEEDED(res));

		res = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
		fence_value = 0;

		fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		assert(fence_event);
	}

	app->run(Function<void(void* c)>(
		[](void* c) {
			HRESULT res;

			auto frame_index = swapchain->GetCurrentBackBufferIndex(); 

			if (fence->GetCompletedValue() < fence_value)
			{
				res = fence->SetEventOnCompletion(fence_value, fence_event);
				assert(SUCCEEDED(res));

				WaitForSingleObject(fence_event, INFINITE);
			}

			fence_value++;

			ID3D12CommandList* _command_lists[] = { command_lists[frame_index] };
			command_queue->ExecuteCommandLists(1, _command_lists);

			res = command_queue->Signal(fence, fence_value);
			assert(SUCCEEDED(res)); 

			res = swapchain->Present(0, 0);
			assert(SUCCEEDED(res));
		}));

	return 0;
}

