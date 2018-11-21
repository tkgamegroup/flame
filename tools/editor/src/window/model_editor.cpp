#include <flame/global.h>
#include <flame/engine/core/input.h>
#include <flame/engine/entity/node.h>
#include <flame/engine/entity/camera_third_person.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/graphics/command_buffer.h>

#include "model_editor.h"

ModelEditor::ModelEditor(std::shared_ptr<flame::Model> _model) :
	Window(model->filename, flame::ui::WindowHasMenu | flame::ui::WindowNoSavedSettings),
	model(_model), 
	layer(true)
{
	first_cx = 800;
	first_cy = 600;

	draw_data.mode = flame::PlainRenderer::mode_just_texture;
	draw_data.obj_data.resize(1);
	draw_data.obj_data[0].mat = glm::mat4(1);
	draw_data.obj_data[0].color = glm::vec4(0.f, 1.f, 0.f, 1.f);
	draw_data.obj_data[0].fill_with_model_texture_mode(model.get());

	camera_node = new flame::Node;
	camera = new flame::CameraComponent;
	camera_node->add_component(camera);
	camera_view_length = 1.f;
	flame::set_camera_third_person_position(camera, glm::vec3(0), camera_view_length);

	renderer = std::make_unique<flame::PlainRenderer>();
}

void ModelEditor::on_show()
{
	ImGui::BeginMenuBar();
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("Save"))
			;

		ImGui::EndMenu();
	}
	ImGui::EndMenuBar();

	auto image_pos = ImGui::GetCursorScreenPos();
	auto image_size = ImVec2(layer.image->get_cx(), layer.image->get_cy());
	ImGui::InvisibleButton("canvas", image_size);
	auto draw_list = ImGui::GetWindowDrawList();
	draw_list->AddImage(ImTextureID(layer.image->ui_index), image_pos, image_pos + image_size);
	if (ImGui::IsItemHovered())
	{
		if (flame::mouse.disp_x != 0 || flame::mouse.disp_y != 0)
		{
			auto distX = (float)flame::mouse.disp_x / (float)flame::resolution.x();
			auto distY = (float)flame::mouse.disp_y / (float)flame::resolution.y();
			if (flame::mouse.button[2].pressing)
			{
				camera_node->add_euler(-distX * 180.f, -distY * 180.f, 0.f);
				flame::set_camera_third_person_position(camera, glm::vec3(0), camera_view_length);
			}
		}
		if (flame::mouse.scroll != 0)
		{
			if (flame::mouse.scroll < 0.f)
				camera_view_length = (camera_view_length + 0.1) * 1.1f;
			else
				camera_view_length = glm::max((camera_view_length - 0.1f) / 1.1f, 0.f);
			flame::set_camera_third_person_position(camera, glm::vec3(0), camera_view_length);
		}
	}

	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::BeginChild("right", ImVec2(500, 0));

	ImGui::Text("indice count:%d", model->indices.size());
	ImGui::Text("indice base:%d", model->indice_base);
	ImGui::Text("vertex base:%d", model->vertex_base);

	//auto funShowAndSelectAnim = [&](flame::ModelStateAnimationKind kind, const char *name) {
	//	int index = 0;
	//	for (int i = 0; i < flame::animations.size(); i++)
	//	{
	//		if (flame::animations[i].get() == m->stateAnimations[kind]->animation)
	//		{
	//			index = i + 1;
	//			break;
	//		}
	//	}
	//	if (ImGui::Combo("Stand Animation", &index, [](void *, int idx, const char **out_text) {
	//		if (idx == 0)
	//			*out_text = "[NULL]";
	//		else
	//			*out_text = flame::animations[idx - 1]->filename.c_str();
	//		return true;
	//	}, nullptr, flame::animations.size() + 1))
	//	{
	//		auto b = m->bindAnimation(flame::animations[index - 1].get());
	//		m->setStateAnimation(kind, b);
	//	}
	//};

	//for (int i = 0; i < flame::ModelStateAnimationCount; i++)
	//{
	//	const char *names[] = {
	//		"Stand Animation",
	//		"Forward Animation",
	//		"Backward Animation",
	//		"Leftward Animation",
	//		"Rightward Animation",
	//		"Jump Animation",
	//	};
	//	funShowAndSelectAnim((flame::ModelStateAnimationKind)i, names[i]);
	//}

	ImGui::DragFloat("Controller Height", &model->controller_height, 0.1f, 0.f, 10000.f);
	ImGui::DragFloat("Controller Radius", &model->controller_radius, 0.1f, 0.f, 10000.f);
	ImGui::DragFloat3("Controller Position", &model->controller_position[0], 0.1f, 0.f, 10000.f);
	ImGui::DragFloat3("Eye Position", &model->eye_position[0]);

	ImGui::EndChild();
	ImGui::EndGroup();

	renderer->render(layer.framebuffer.get(), true, camera, &draw_data);
	renderer->add_to_drawlist();
}
