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

int main(int argc, char** args)
{
	auto app = Application::create();
	auto w = Window::create(app, "DX Test", Ivec2(1280, 720), WindowFrame);

	for (auto i = 0; i < 2; i++)
	{
		command_list->ResourceBarrier(1, &dx_barrier_transition(backbuffers[i], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		auto descriptor_handle = descriptor_heap->GetCPUDescriptorHandleForHeapStart();
		descriptor_handle.ptr += i * descriptor_size;

		command_list->OMSetRenderTargets(1, &descriptor_handle, false, nullptr);

		const float clear_color[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		command_list->ClearRenderTargetView(descriptor_handle, clear_color, 0, nullptr);

		command_list->ResourceBarrier(1, &dx_barrier_transition(backbuffers[i], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	}

	app->run(Function<void(void* c)>(
		[](void* c) {
			auto frame_index = swapchain->GetCurrentBackBufferIndex();
		}));

	return 0;
}

