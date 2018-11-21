#include <flame/global.h>
#include <flame/filesystem.h>
#include <flame/engine/core/core.h>
#include <flame/engine/core/input.h>
#include <flame/engine/graphics/buffer.h>
#include <flame/engine/graphics/descriptor.h>
#include <flame/engine/graphics/command_buffer.h>
#include <flame/engine/graphics/pick_up.h>
#include <flame/engine/entity/model.h>
#include <flame/engine/entity/animation.h>
#include <flame/engine/entity/light.h>
#include <flame/engine/entity/model_instance.h>
#include <flame/engine/entity/terrain.h>
#include <flame/engine/entity/water.h>
#include <flame/engine/physics/physics.h>
#include "../select.h"
#include "resource_explorer.h"
#include "hierarchy.h"
#include "scene_editor.h"

void SceneEditor::on_delete()
{
	auto n = selected.get_node();
	if (!n || n == scene)
		return;

	n->get_parent()->remove_child(n);
}

SceneEditor *scene_editor = nullptr;

SceneEditor::SceneEditor(flame::Scene *_scene) :
	Window("Scene"),
	layer(true),
	curr_tool(nullptr)
{
	camera_node = new flame::Node;
	camera = new flame::CameraComponent;
	camera_controller = new flame::ControllerComponent;
	camera_node->add_component(camera);
	camera_node->add_component(camera_controller);
	flame::root_node->add_child(camera_node);

	plain_renderer = std::make_unique<flame::PlainRenderer>();
	defe_renderer = std::make_unique<flame::DeferredRenderer>(false, &layer);

	scene = _scene;
	flame::root_node->add_child(scene);

	physx_vertex_buffer = std::make_unique<flame::Buffer>(flame::BufferTypeImmediateVertex, 16);
	lines_renderer = std::make_unique<flame::LinesRenderer>();

	transformerTool = std::make_unique<TransformerTool>();

	select_image = flame::get_texture("Select.png");
	move_image = flame::get_texture("Move.png");
	rotate_image = flame::get_texture("Rotate.png");
	scale_image = flame::get_texture("Scale.png");

	assert(select_image);
	assert(move_image);
	assert(rotate_image);
	assert(scale_image);

	flame::ui::increase_texture_ref(select_image.get());
	flame::ui::increase_texture_ref(move_image.get());
	flame::ui::increase_texture_ref(rotate_image.get());
	flame::ui::increase_texture_ref(scale_image.get());
}

SceneEditor::~SceneEditor()
{
	flame::root_node->remove_child(camera_node);
	flame::root_node->remove_child(scene);
	scene_editor = nullptr;

	flame::ui::decrease_texture_ref(select_image.get());
	flame::ui::decrease_texture_ref(move_image.get());
	flame::ui::decrease_texture_ref(rotate_image.get());
	flame::ui::decrease_texture_ref(scale_image.get());
}

void SceneEditor::on_file_menu()
{
	if (ImGui::MenuItem("Save", "Ctrl+S"))
		flame::save_scene(scene);
}

void SceneEditor::on_menu_bar()
{
	if (ImGui::BeginMenu("Create"))
	{
		if (ImGui::MenuItem("Empty"))
			;
		if (ImGui::MenuItem("Empty Child"))
			;
		if (ImGui::BeginMenu("Light"))
		{
			flame::LightType light_type = flame::LightType(-1);
			if (ImGui::MenuItem("Parallax"))
				light_type = flame::LightTypeParallax;
			if (ImGui::MenuItem("Point"))
				light_type = flame::LightTypePoint;
			if (ImGui::MenuItem("Spot"))
				light_type = flame::LightTypeSpot;
			if (light_type != -1)
			{
				auto n = new flame::Node;
				n->name = "Light";
				n->set_coord(camera_node->get_world_coord());
				auto i = new flame::LightComponent;
				i->set_type(light_type);
				n->add_component(i);
				scene->add_child(n);
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("3D"))
		{
			const char *basic_model_names[] = {
				"Triangle",
				"Cube",
				"Sphere",
				"Cylinder",
				"Cone",
				"Arrow",
				"Torus",
				"Hammer"
			};
			for (int i = 0; i < TK_ARRAYSIZE(basic_model_names); i++)
			{
				if (ImGui::MenuItem(basic_model_names[i]))
				{
					auto m = flame::getModel(basic_model_names[i]);
					if (m)
					{
						auto n = new flame::Node(flame::NodeTypeNode);
						n->name = "Object";
						n->set_coord(camera_node->get_world_coord());
						auto i = new flame::ModelInstanceComponent;
						i->set_model(m);
						n->add_component(i);
						scene->add_child(n);
					}
				}
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Terrain"))
			{
				auto n = new flame::Node(flame::NodeTypeNode);
				n->set_coord(camera_node->get_world_coord());
				auto t = new flame::TerrainComponent;
				n->add_component(t);
				scene->add_child(n);
			}
			if (ImGui::MenuItem("Water"))
			{
				auto n = new flame::Node(flame::NodeTypeNode);
				n->set_coord(camera_node->get_world_coord());
				auto w = new flame::WaterComponent;
				n->add_component(w);
				scene->add_child(n);
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Camera"))
			;

		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Edit"))
	{
		if (ImGui::MenuItem("Cut", "Ctrl+X"))
			;
		if (ImGui::MenuItem("Copy", "Ctrl+C"))
			;
		if (ImGui::MenuItem("Paste", "Ctrl+V"))
			;
		if (ImGui::MenuItem("Delete", "Del"))
			on_delete();
		ImGui::Separator();
		if (ImGui::MenuItem("Target To Selected"))
			;

		ImGui::EndMenu();
	}
}

void SceneEditor::on_toolbar()
{
	if (ImGui::ImageButton_s(select_image.get(), ImVec2(16, 16), curr_tool == nullptr))
		curr_tool = nullptr;
	ImGui::SameLine();
	if (ImGui::ImageButton_s(move_image.get(), ImVec2(16, 16), curr_tool && curr_tool == transformerTool.get() && transformerTool->operation == TransformerTool::TRANSLATE))
	{
		curr_tool = transformerTool.get();
		transformerTool->operation = TransformerTool::TRANSLATE;
	}
	ImGui::SameLine();
	if (ImGui::ImageButton_s(rotate_image.get(), ImVec2(16, 16), curr_tool && curr_tool == transformerTool.get() && transformerTool->operation == TransformerTool::ROTATE))
	{
		curr_tool = transformerTool.get();
		transformerTool->operation = TransformerTool::ROTATE;
	}
	ImGui::SameLine();
	if (ImGui::ImageButton_s(scale_image.get(), ImVec2(16, 16), curr_tool && curr_tool == transformerTool.get() && transformerTool->operation == TransformerTool::SCALE))
	{
		curr_tool = transformerTool.get();
		transformerTool->operation = TransformerTool::SCALE;
	}
	ImGui::SameLine();
	if (curr_tool == transformerTool.get())
	{
		ImGui::Checkbox("snap", &transformerTool->enable_snap);
		ImGui::SameLine();
		if (transformerTool->enable_snap)
		{
			ImGui::PushItemWidth(100.f);
			switch (transformerTool->operation)
			{
				case TransformerTool::TRANSLATE:
					ImGui::DragFloat3("##snap_value", &transformerTool->translate_snap[0], 0.1f, 0.f, 10000.f);
					break;
				case TransformerTool::ROTATE:
					ImGui::DragFloat("##snap_value", &transformerTool->rotate_snap, 0.1f, 0.f, 10000.f);
					break;
				case TransformerTool::SCALE:
					ImGui::DragFloat("##snap_value", &transformerTool->scale_snap, 0.1f, 0.f, 10000.f);
					break;
			}
			ImGui::PopItemWidth();
		}
	}
}

void SceneEditor::on_show()
{
	auto size = ImGui::GetWindowContentRegionMax() - ImGui::GetWindowContentRegionMin();
	size.y -= ImGui::GetFrameHeightWithSpacing() + ImGui::GetItemRectSize().y + 1;
	size.x = glm::max(size.x, 1.f);
	size.y = glm::max(size.y, 1.f);
	if (flame::resolution.x() != size.x || flame::resolution.y() != size.y)
		flame::resolution.set(size.x, size.y);

	auto pos = ImGui::GetCursorScreenPos();

	ImGui::SetCursorScreenPos(pos);
	if (ImGui::IconButton(ICON_FA_CHEVRON_DOWN))
		ImGui::OpenPopup("ShowPopup");
	if (ImGui::BeginPopup("ShowPopup"))
	{
		ImGui::MenuItem("Enable Render", "", &enableRender);
		ImGui::MenuItem("Show Selected Wire Frame", "", &showSelectedWireframe);
		if (ImGui::MenuItem("View Physx", "", &viewPhysx))
		{
			//if (viewPhysx)
			//{
			//	scene->pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.f);
			//	////scene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 1.f);
			//	scene->pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 1.f);
			//	//scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_DYNAMIC, 1.f);
			//	//pxScene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.f);
			//	////scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.f);
			//}
			//else
			//	scene->pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 0.f);
		}
		ImGui::EndPopup();
	}
	pos += ImVec2(0, 20);

	ImGui::SetCursorScreenPos(pos);
	ImGui::InvisibleButton("canvas", ImVec2(flame::resolution.x(), flame::resolution.y()));
	auto draw_list = ImGui::GetWindowDrawList();
	auto canvas_size = ImVec2(flame::resolution.x(), flame::resolution.y());
	draw_list->AddImage(ImTextureID(layer.image->ui_index), pos, pos + canvas_size);
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("file"))
		{
			static char filename[260];
			strcpy(filename, (char*)payload->Data);
			std::experimental::filesystem::path path(filename);
			auto ext = path.extension();
			if (flame::is_model_file(ext.string()))
			{
				auto m = flame::getModel(filename);
				if (m)
				{
					auto n = new flame::Node(flame::NodeTypeNode);
					n->name = "Object";
					n->set_coord(camera_node->get_world_coord());
					auto i = new flame::ModelInstanceComponent;
					i->set_model(m);
					n->add_component(i);
					scene->add_child(n);
				}
			}
		}
		ImGui::EndDragDropTarget();
	}

	if (curr_tool == transformerTool.get())
	{
		transformerTool->target = selected.get_node();
		if (transformerTool->target == scene)
			transformerTool->target = nullptr;
		transformerTool->show(glm::vec2(pos.x, pos.y), glm::vec2(canvas_size.x, canvas_size.y), camera);
	}

	if (ImGui::IsItemHovered())
	{
		if (flame::mouse.disp_x != 0 || flame::mouse.disp_y != 0)
		{
			if (!curr_tool || !(curr_tool == transformerTool.get() && transformerTool->is_over()))
			{
				auto distX = (float)flame::mouse.disp_x / (float)flame::resolution.x();
				auto distY = (float)flame::mouse.disp_y / (float)flame::resolution.y();
				if (flame::mouse.button[1].pressing)
					camera_node->add_euler(-distX * 180.f, -distY * 180.f, 0.f);
			}
		}
		if (flame::mouse.button[0].just_down)
		{
			if (!flame::key_states[VK_SHIFT].pressing && !flame::key_states[VK_CONTROL].pressing)
			{
				auto x = (flame::mouse.x - 0) / flame::resolution.x();
				auto y = (flame::mouse.y - 0) / flame::resolution.y();
				if (!curr_tool || !(curr_tool == transformerTool.get() && transformerTool->is_over()))
				{
					//flame::PlainRenderer::DrawData draw_data;
					//draw_data.mode = flame::PlainRenderer::mode_just_color;
					//for (int i = 0; i < scene->objects.size(); i++)
					//{
					//	auto object = scene->objects[i].get();

					//	flame::PlainRenderer::DrawData::ObjData obj_data;
					//	obj_data.mat = object->get_matrix();
					//	obj_data.color = glm::vec4((i + 1) / 255.f, 0.f, 0.f, 0.f);
					//	obj_data.fill_with_model(object->model.get());
					//	if (object->model->vertex_skeleton)
					//		obj_data.bone_buffer = object->animationComponent->bone_buffer.get();
					//	draw_data.obj_data.push_back(obj_data);
					//}
					//auto index = flame::pick_up(x, y, std::bind(
					//	&flame::PlainRenderer::do_render,
					//	plain_renderer.get(), std::placeholders::_1, camera, &draw_data));
					//if (index == 0)
					//	selected.reset();
					//else
					//	selected = scene->objects[index - 1];
				}
			}
		}

		camera_controller->set_state(flame::ControllerStateForward, flame::key_states['W'].pressing);
		camera_controller->set_state(flame::ControllerStateBackward, flame::key_states['S'].pressing);
		camera_controller->set_state(flame::ControllerStateLeft, flame::key_states['A'].pressing);
		camera_controller->set_state(flame::ControllerStateRight, flame::key_states['D'].pressing);

		if (ImGui::IsKeyDown(VK_DELETE))
			on_delete();
	}
	else
		camera_controller->set_state(flame::ControllerStateStand, true);

	if (enableRender)
	{
		defe_renderer->render(scene, camera);
		defe_renderer->add_to_drawlist();
	}

	{
		//for (int i = 0; i < rb.getNbPoints(); i++)
		//{
		//	p.color = _intToRGB(rb.getPoints()[i].color);
		//	p.coord = _pxVec3ToVec3(rb.getPoints()[i].pos);
		//	tke3_debugBuffer.points.push_back(p);
		//}

		//glNamedBufferSubData(tke_dynamicVertexVBO, 0, sizeof(GLfloat) * 12 * lineCount, tke_dynamicVertexBuffer);
		//glBindVertexArray(tke3_dynamicVertexVAO);
		//glDrawArrays(GL_LINES, 0, lineCount * 2);

		//auto triangleCount = rb.getNbTriangles();
		//for (int i = 0; i < triangleCount; i++)
		//{
		//	auto &triangle = rb.getTriangles()[i];
		//	tke_dynamicVertexBuffer[i * 18 + 0] = triangle.pos0.x;
		//	tke_dynamicVertexBuffer[i * 18 + 1] = triangle.pos0.y;
		//	tke_dynamicVertexBuffer[i * 18 + 2] = triangle.pos0.z;
		//	tke_dynamicVertexBuffer[i * 18 + 3] = triangle.color0 % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 4] = (triangle.color0 / 256) % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 5] = (triangle.color0 / 65536) % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 6] = triangle.pos1.x;
		//	tke_dynamicVertexBuffer[i * 18 + 7] = triangle.pos1.y;
		//	tke_dynamicVertexBuffer[i * 18 + 8] = triangle.pos1.z;
		//	tke_dynamicVertexBuffer[i * 18 + 9] = triangle.color1 % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 10] = (triangle.color1 / 256) % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 11] = (triangle.color1 / 65536) % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 12] = triangle.pos2.x;
		//	tke_dynamicVertexBuffer[i * 18 + 13] = triangle.pos2.y;
		//	tke_dynamicVertexBuffer[i * 18 + 14] = triangle.pos2.z;
		//	tke_dynamicVertexBuffer[i * 18 + 15] = triangle.color2 % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 16] = (triangle.color2 / 256) % 256;
		//	tke_dynamicVertexBuffer[i * 18 + 17] = (triangle.color2 / 65536) % 256;
		//}
		//glNamedBufferSubData(tke_dynamicVertexVBO, 0, sizeof(GLfloat) * 18 * triangleCount, tke_dynamicVertexBuffer);
		//glBindVertexArray(tke3_dynamicVertexVAO);
		//glDrawArrays(GL_TRIANGLES, 0, triangleCount * 3);
	}
	if (viewPhysx)
	{
		//auto &rb = scene->pxScene->getRenderBuffer();
		//auto lineCount = rb.getNbLines();
		//if (lineCount > 0)
		//{
		//	auto vertex_count = lineCount * 2;
		//	auto size = vertex_count * sizeof(flame::LinesRenderer::Vertex);
		//	auto vtx_dst = (flame::LinesRenderer::Vertex*)physx_vertex_buffer->map(0, size);
		//	for (int i = 0; i < lineCount; i++)
		//	{
		//		auto &line = rb.getLines()[i];
		//		vtx_dst[0].position = flame::physx_vec3_to_vec3(line.pos0);
		//		vtx_dst[0].color = flame::physx_u32_to_vec3(line.color0);
		//		vtx_dst[1].position = flame::physx_vec3_to_vec3(line.pos1);
		//		vtx_dst[1].color = flame::physx_u32_to_vec3(line.color1);
		//		vtx_dst += 2;
		//	}
		//	physx_vertex_buffer->unmap();
		//	flame::LinesRenderer::DrawData data;
		//	data.vertex_buffer = physx_vertex_buffer.get();
		//	data.vertex_count = vertex_count;
		//	lines_renderer->render(layer.framebuffer.get(), false, camera, &data);
		//	lines_renderer->add_to_drawlist();
		//}
	}

	if (showSelectedWireframe)
	{
		//auto n = selected.get_node();
		//if (n && n->get_type() == flame::NodeTypeObject)
		//{
		//	auto obj = (flame::Object*)n;
		//	flame::PlainRenderer::DrawData data;
		//	data.mode = flame::PlainRenderer::mode_wireframe;
		//	flame::PlainRenderer::DrawData::ObjData obj_data;
		//	obj_data.mat = obj->get_matrix();
		//	obj_data.color = glm::vec4(0.f, 1.f, 0.f, 1.f);
		//	obj_data.fill_with_model(obj->model.get());
		//	if (obj->model->vertex_skeleton)
		//		obj_data.bone_buffer = obj->animationComponent->bone_buffer.get();
		//	data.obj_data.push_back(obj_data);
		//	plain_renderer->render(layer.framebuffer.get(), false, camera, &data);
		//	plain_renderer->add_to_drawlist();
		//}
	}
}

void SceneEditor::save(flame::XMLNode *n)
{
	n->attributes.emplace_back(new flame::XMLAttribute("filename", scene->get_filename()));
}
