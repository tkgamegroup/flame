#pragma once

#include "app.h"

struct DebuggerView : View
{
	int view_swizzle = ImGui::ImageViewRGBA;
	int view_sampler = ImGui::ImageViewLinear;
	int view_level = 0;
	int view_layer = 0;
	float view_zoom = 1.f;
	float view_range_min = 0.f;
	float view_range_max = 1.f;

	graphics::BufferPtr selected_buffer = nullptr;
	graphics::ImagePtr selected_image = nullptr;
	graphics::RenderpassPtr selected_renderpass = nullptr;
	graphics::FramebufferPtr selected_framebuffer = nullptr;
	graphics::DescriptorSetLayoutPtr selected_descriptor_layout = nullptr;
	graphics::DescriptorSetPtr selected_descriptor_set = nullptr;
	graphics::PipelineLayoutPtr selected_pipeline_layout = nullptr;
	graphics::GraphicsPipelinePtr selected_graphics_pipeline = nullptr;
	graphics::ComputePipelinePtr selected_compute_pipeline = nullptr;
	graphics::ShaderPtr selected_shader = nullptr;

	DebuggerView();
	DebuggerView(const std::string& name);
	void on_draw() override;
};

struct DebuggerWindow : Window
{
	DebuggerWindow();
	View* open_view(bool new_instance) override;
	View* open_view(const std::string& name) override;
};

extern DebuggerWindow debugger_window;
