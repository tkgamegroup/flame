#pragma once

#include <flame/engine/graphics/display_layer.h>
#include <flame/engine/graphics/renderer.h>
#include <flame/engine/entity/model.h>
#include <flame/engine/entity/camera.h>
#include <flame/engine/ui/window.h>

struct ModelEditor : flame::ui::Window
{
	std::shared_ptr<flame::Model> model;
	flame::PlainRenderer::DrawData draw_data;

	flame::DisplayLayer layer;

	flame::Node *camera_node;
	flame::CameraComponent *camera;
	float camera_view_length;
	std::unique_ptr<flame::PlainRenderer> renderer;

	ModelEditor(std::shared_ptr<flame::Model> _model);
	virtual void on_show() override;
};
