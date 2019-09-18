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
};

extern SceneEditor *scene_editor;