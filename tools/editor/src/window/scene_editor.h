#pragma once

#include <flame/engine/entity/scene.h>
#include <flame/engine/entity/camera.h>
#include <flame/engine/entity/controller.h>
#include <flame/engine/graphics/display_layer.h>
#include <flame/engine/graphics/renderer.h>
#include <flame/engine/ui/window.h>
#include "../tool/transformer_tool.h"

struct SceneEditor : flame::ui::Window
{
	flame::Node *camera_node;
	flame::CameraComponent *camera;
	flame::ControllerComponent *camera_controller;

	flame::Scene *scene;

	flame::DisplayLayer layer;

	std::unique_ptr<flame::PlainRenderer> plain_renderer;

	bool enableRender = true;
	std::unique_ptr<flame::DeferredRenderer> defe_renderer;

	bool viewPhysx = false;
	std::unique_ptr<flame::Buffer> physx_vertex_buffer;
	std::unique_ptr<flame::LinesRenderer> lines_renderer;

	bool showSelectedWireframe = true;

	Tool *curr_tool;
	std::unique_ptr<TransformerTool> transformerTool;

	std::shared_ptr<flame::Texture> select_image;
	std::shared_ptr<flame::Texture> move_image;
	std::shared_ptr<flame::Texture> rotate_image;
	std::shared_ptr<flame::Texture> scale_image;

	SceneEditor(flame::Scene *_scene);
	~SceneEditor();
	void on_file_menu();
	void on_menu_bar();
	void on_toolbar();
	virtual void on_show() override;
	void save(flame::XMLNode *);

	void on_delete();
};

extern SceneEditor *scene_editor;