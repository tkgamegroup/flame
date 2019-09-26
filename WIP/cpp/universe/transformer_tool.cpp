static const float quad_min = 0.5f;
static const float quad_max = 0.8f;
static const float quad_uv[8] = { 
	quad_min, quad_min, quad_min, quad_max, 
	quad_max, quad_max, quad_max, quad_min 
};

static void compute_snap(float *value, float snap)
{
	const auto snap_tension = 0.5f;

	if (snap <= FLT_EPSILON)
		return;
	auto modulo = glm::mod(*value, snap);
	auto modulo_ratio = glm::abs(modulo) / snap;
	if (modulo_ratio < snap_tension)
		*value -= modulo;
	else if (modulo_ratio > 1.f - snap_tension)
		*value = *value - modulo + snap * ((*value < 0.f) ? -1.f : 1.f);
}

static void compute_snap(glm::vec3 &value, const glm::vec3 &snap)
{
	for (int i = 0; i < 3; i++)
		compute_snap(&value[i], snap[i]);
}

bool can_activate()
{
	return ImGui::IsItemHovered() && ImGui::IsMouseClicked(0);
}

TransformerTool::TransformerTool() :
	operation(TRANSLATE),
	mode(LOCAL),
	type(NONE),
	enable_snap(false),
	using_(false),
	translate_snap(1.f),
	rotate_snap(45.f),
	scale_snap(0.5f),
	target(nullptr),
	enable(true)
{
}

bool TransformerTool::is_over()
{
	return get_move_type(nullptr) != NONE || get_rotate_type() != NONE || get_scale_type() != NONE || using_;
}

ImVec2 TransformerTool::world_to_screen(const glm::vec3 &coord, const glm::mat4 &mat)
{
	auto v = flame::transform(coord, mat);
	return window_pos + (ImVec2(v.x, v.y) / 2.f + ImVec2(0.5f, 0.5f)) * window_size;
}

void TransformerTool::compute_tripod_axis_and_visibility(int axis_index, glm::vec3 &dir_plane_x, glm::vec3 &dir_plane_y, bool &_below_axis_limit, bool &_below_plane_limit)
{
	const auto plan_normal = (axis_index + 2) % 3;
	dir_plane_x = direction_unary[axis_index];
	dir_plane_y = direction_unary[(axis_index + 1) % 3];

	auto dir_plane_normal_world = glm::normalize(glm::vec3(model_matrix * glm::vec4(direction_unary[plan_normal], 1.f)));
	auto dir_plane_x_World = glm::normalize(glm::vec3(model_matrix * glm::vec4(dir_plane_x, 1.f)));
	auto dir_plane_y_World = glm::normalize(glm::vec3(model_matrix * glm::vec4(dir_plane_y, 1.f)));

	auto camera_eye_to_tool = glm::normalize(model_position - camera_position);
	auto dot_camera_dir_x = glm::dot(camera_eye_to_tool, dir_plane_x_World);
	auto dot_camera_dir_y = glm::dot(camera_eye_to_tool, dir_plane_y_World);

	auto mul_axis_x = (dot_camera_dir_x > 0.f) ? -1.f : 1.f;
	auto mul_axis_y = (dot_camera_dir_y > 0.f) ? -1.f : 1.f;
	dir_plane_x *= mul_axis_x;
	dir_plane_y *= mul_axis_y;

	const auto angle_limit = 0.96f;
	const auto plane_limit = 0.2f;

	_below_axis_limit = glm::abs(dot_camera_dir_x) < angle_limit;
	_below_plane_limit = (glm::abs(glm::dot(camera_eye_to_tool, dir_plane_normal_world)) > plane_limit);

	axis_factor[axis_index] = mul_axis_x;
	axis_factor[(axis_index + 1) % 3] = mul_axis_y;
	below_axis_limit[axis_index] = _below_axis_limit;
	below_plane_limit[axis_index] = _below_plane_limit;
}

float TransformerTool::compute_angle_on_plan()
{
	const auto len = flame::ray_intersect_plane(ray_origin, ray_vector, translation_plane);
	auto local_pos = glm::normalize(ray_origin + ray_vector * len - model_position);

	auto perpendicular_vector = glm::normalize(glm::cross(rotation_vector_source, glm::vec3(translation_plane)));
	auto acos_angle = glm::clamp(glm::dot(local_pos, rotation_vector_source), -0.9999f, 0.9999f);
	auto angle = glm::acos(acos_angle);
	angle *= (glm::dot(local_pos, perpendicular_vector) < 0.f) ? 1.f : -1.f;
	return angle;
}

void TransformerTool::compute_colors(ImU32 *colors, int type)
{
	if (enable)
	{
		const Vec4 direction_color[] = {
			Vec4(0.66f, 0.f, 0.f, 1.f),
			Vec4(0.f, 0.66f, 0.f, 1.f),
			Vec4(0.f, 0.f, 0.66f, 1.f)
		};

		const Vec4 plane_color[] = { 
			Vec4(0.f, 0.f, 0.66f, 0.38f),
			Vec4(0.f, 0.66f, 0.f, 0.38f),
			Vec4(0.66f, 0.f, 0.f, 0.38f)
		};

		const Vec4 selection_color = Vec4(0.06f, 0.06f, 0.5f, 0.54f);

		const Vec4 inactive_color = Vec4(0.6f);

		switch (operation)
		{
			case TRANSLATE:
				colors[0] = (type == MOVE_SCREEN) ? selection_color : 0xFFFFFFFF;
				for (int i = 0; i < 3; i++)
				{
					int index = (i + 2) % 3;
					colors[i + 1] = (type == MOVE_X + i) ? selection_color : direction_color[i];
					colors[i + 4] = (type == MOVE_XY + i) ? selection_color : plane_color[index];
					colors[i + 4] = (type == MOVE_SCREEN) ? selection_color : colors[i + 4];
				}
				break;
			case ROTATE:
				colors[0] = (type == ROTATE_SCREEN) ? selection_color : 0xFFFFFFFF;
				for (int i = 0; i < 3; i++)
					colors[i + 1] = (type == ROTATE_X + i) ? selection_color : direction_color[i];
				break;
			case SCALE:
				colors[0] = (type == SCALE_XYZ) ? selection_color : 0xFFFFFFFF;
				for (int i = 0; i < 3; i++)
					colors[i + 1] = (type == SCALE_X + i) ? selection_color : direction_color[i];
				break;
		}
	}
	else
	{
		for (int i = 0; i < 7; i++)
			colors[i] = inactive_color;
	}
}

int TransformerTool::get_move_type( glm::vec3 *hit_proportion)
{
	auto &io = ImGui::GetIO();
	int type = TransformerTool::NONE;

	if (io.MousePos.x >= screen_square_min.x && io.MousePos.x <= screen_square_max.x &&
		io.MousePos.y >= screen_square_min.y && io.MousePos.y <= screen_square_max.y)
		type = TransformerTool::MOVE_SCREEN;

	const glm::vec3 direction[3] = { model_matrix[0], model_matrix[1], model_matrix[2] };

	for (auto i = 0; i < 3 && type == TransformerTool::NONE; i++)
	{
		glm::vec3 dir_plane_x, dir_plane_y;
		auto below_axis_limit = false;
		auto below_plane_limit = false;
		compute_tripod_axis_and_visibility(i, dir_plane_x, dir_plane_y, below_axis_limit, below_plane_limit);
		dir_plane_x = model_matrix * glm::vec4(dir_plane_x, 0.f);
		dir_plane_y = model_matrix * glm::vec4(dir_plane_y, 0.f);

		const auto plan_normal = (i + 2) % 3;

		const auto len = flame::ray_intersect_plane(ray_origin, ray_vector, flame::plane(model_position, direction[plan_normal]));
		auto pos_on_plane = ray_origin + ray_vector * len;

		const auto dx = glm::dot(dir_plane_x, (pos_on_plane - model_position) / screen_factor);
		const auto dy = glm::dot(dir_plane_y, (pos_on_plane - model_position) / screen_factor);
		if (below_axis_limit && dy > -0.1f && dy < 0.1f && dx > 0.1f  && dx < 1.f)
			type = TransformerTool::MOVE_X + i;

		if (below_plane_limit && dx >= quad_uv[0] && dx <= quad_uv[4] && dy >= quad_uv[1] && dy <= quad_uv[3])
			type = TransformerTool::MOVE_XY + i;

		if (hit_proportion)
			*hit_proportion = glm::vec3(dx, dy, 0.f);
	}
	return type;
}

int TransformerTool::get_rotate_type()
{
	auto &io = ImGui::GetIO();
	int type = NONE;

	glm::vec3 delta_screen(io.MousePos.x - screen_square_center.x, io.MousePos.y - screen_square_center.y, 0.f);
	auto dist = glm::length(delta_screen);
	if (dist >= (radius_square_center - 1.f) && dist < (radius_square_center + 1.f))
		type = ROTATE_SCREEN;

	const glm::vec3 plan_normals[] = { model_matrix[0], model_matrix[1], model_matrix[2] };

	for (auto i = 0; i < 3 && type == NONE; i++)
	{
		auto pickup_plan = flame::plane(model_position, plan_normals[i]);

		const auto len = flame::ray_intersect_plane(ray_origin, ray_vector, pickup_plan);
		auto local_pos = ray_origin + ray_vector * len - model_position;

		if (glm::dot(glm::normalize(local_pos), ray_vector) > FLT_EPSILON)
			continue;

		auto distance = glm::length(local_pos) / screen_factor;
		if (distance > 0.9f && distance < 1.1f)
			type = ROTATE_X + i;
	}

	return type;
}

int TransformerTool::get_scale_type()
{
	auto &io = ImGui::GetIO();
	int type = NONE;

	if (io.MousePos.x >= screen_square_min.x && io.MousePos.x <= screen_square_max.x &&
		io.MousePos.y >= screen_square_min.y && io.MousePos.y <= screen_square_max.y)
		type = SCALE_XYZ;

	const glm::vec3 direction[] = { model_matrix[0], model_matrix[1], model_matrix[2] };

	for (auto i = 0; i < 3 && type == NONE; i++)
	{
		glm::vec3 dir_plane_x, dir_plane_y;
		bool below_axis_limit, below_plane_limit;
		compute_tripod_axis_and_visibility(i, dir_plane_x, dir_plane_y, below_axis_limit, below_plane_limit);
		dir_plane_x = flame::transform(dir_plane_x, model_matrix);
		dir_plane_y = flame::transform(dir_plane_y, model_matrix);

		const auto planNormal = (i + 2) % 3;

		const auto len = flame::ray_intersect_plane(ray_origin, ray_vector, flame::plane(model_position, direction[planNormal]));
		auto pos_on_plan = ray_origin + ray_vector * len;

		const auto dx = glm::dot(dir_plane_x, (pos_on_plan - model_position) / screen_factor);
		const auto dy = glm::dot(dir_plane_y, (pos_on_plan - model_position) / screen_factor);
		if (below_axis_limit && dy > -0.1f && dy < 0.1f && dx > 0.1f  && dx < 1.f)
			type = SCALE_X + i;
	}

	return type;
}

void TransformerTool::draw_hatched_axis(const glm::vec3 &axis)
{
	for (int j = 1; j < 10; j++)
	{
		ImVec2 baseSSpace2 = world_to_screen(axis * 0.05f * (float)(j * 2) * screen_factor, mvp);
		ImVec2 worldDirSSpace2 = world_to_screen(axis * 0.05f * (float)(j * 2 + 1) * screen_factor, mvp);
		draw_list->AddLine(baseSSpace2, worldDirSSpace2, 0x80000000, 6.f);
	}
}

void TransformerTool::show(glm::vec2 _window_pos, glm::vec2 _window_size, flame::CameraComponent *camera)
{
	if (!target)
		return;

	draw_list = ImGui::GetWindowDrawList();
	if (!draw_list)
		return;

	auto &io = ImGui::GetIO();

	window_pos = ImVec2(_window_pos.x, _window_pos.y);
	window_size = ImVec2(_window_size.x, _window_size.y);

	draw_list->PushClipRect(window_pos, window_pos + window_size);

	model_position = target->get_world_coord();
	model_source = target->get_world_matrix();
	if (mode == LOCAL)
	{
		model_matrix = model_source;
		flame::ortho_normalize(model_matrix);
	}
	else
		model_matrix = glm::translate(model_position);
	model_matrix_inverse = glm::inverse(model_matrix);
	proj_view = camera->get_proj_matrix() * camera->get_view_matrix();
	mvp = proj_view * model_matrix;

	camera_position = camera->get_parent()->get_world_coord();
	camera_dir = camera->get_parent()->get_axis()[2];
	{
		auto proj_view_inverse = glm::inverse(proj_view);

		ImVec2 mo = ImVec2((io.MousePos - window_pos) / window_size) * 2.f - ImVec2(1.f, 1.f);

		ray_origin = flame::transform(glm::vec3(mo.x, mo.y, -1.f), proj_view_inverse);
		ray_vector = glm::normalize(flame::transform(glm::vec3(mo.x, mo.y, 1.f), proj_view_inverse) - ray_origin);
	}

	screen_factor = 0.1f * (proj_view * glm::vec4(model_position, 1.f)).w;
	screen_square_center = world_to_screen(glm::vec3(0.f), mvp);
	screen_square_min = ImVec2(screen_square_center.x - 10.f, screen_square_center.y - 10.f);
	screen_square_max = ImVec2(screen_square_center.x + 10.f, screen_square_center.y + 10.f);

	int type = NONE;
	if (enable)
	{
		switch (operation)
		{
			case TRANSLATE:
			{
				auto apply_rotation_localy = mode == LOCAL || type == MOVE_SCREEN;
				if (using_)
				{
					ImGui::CaptureMouseFromApp();
					const float len = flame::ray_intersect_plane(ray_origin, ray_vector, translation_plane);
					auto new_pos = ray_origin + ray_vector * len;

					auto new_origin = new_pos - relative_origin * screen_factor;
					auto delta = new_origin - model_position;

					if (this->type >= MOVE_X && this->type <= MOVE_Z)
					{
						auto axis_index = this->type - MOVE_X;
						const auto &axis_value = *(glm::vec3*)&model_matrix[axis_index];
						delta = axis_value * glm::dot(axis_value, delta);
					}

					if (enable_snap)
					{
						auto cumulative_delta = model_position + delta - matrix_origin;
						if (apply_rotation_localy)
						{
							auto model_source_normalized = model_source;
							flame::ortho_normalize(model_source_normalized);
							auto model_source_normalized_inverse = glm::inverse(model_source_normalized);
							cumulative_delta = flame::transform(cumulative_delta, model_source_normalized_inverse);
							compute_snap(cumulative_delta, translate_snap);
							cumulative_delta = flame::transform(cumulative_delta, model_source_normalized);
						}
						else
							compute_snap(cumulative_delta, translate_snap);
						delta = matrix_origin + cumulative_delta - model_position;
					}

					target->add_coord(delta);

					if (!io.MouseDown[0])
						using_ = false;

					type = this->type;
				}
				else
				{
					glm::vec3 hit_proportion;
					type = (TransType)get_move_type(&hit_proportion);
					if (can_activate() && type != NONE)
					{
						ImGui::CaptureMouseFromApp();
						using_ = true;
						this->type = (TransType)type;
						const glm::vec3 move_plan_normal[] = { model_matrix[1], model_matrix[2], model_matrix[0], model_matrix[2], model_matrix[0], model_matrix[1], -camera_dir };
						translation_plane = flame::plane(model_position, move_plan_normal[type - MOVE_X]);
						const auto len = flame::ray_intersect_plane(ray_origin, ray_vector, translation_plane);
						translation_plane_origin = ray_origin + ray_vector * len;
						matrix_origin = model_position;

						relative_origin = (translation_plane_origin - model_position) * (1.f / screen_factor);
					}
				}
				break;
			}
			case ROTATE:
			{
				bool apply_rotation_localy = mode == LOCAL;

				if (!using_)
				{
					type = get_rotate_type();

					if (type == ROTATE_SCREEN)
						apply_rotation_localy = true;

					if (can_activate() && type != NONE)
					{
						ImGui::CaptureMouseFromApp();
						using_ = true;
						this->type = (TransType)type;
						const glm::vec3 rotate_plan_normal[] = { model_matrix[0], model_matrix[1], model_matrix[2], -camera_dir };

						if (apply_rotation_localy)
							translation_plane = flame::plane(model_position, rotate_plan_normal[type - ROTATE_X]);
						else
							translation_plane = flame::plane(model_source[3], direction_unary[type - ROTATE_X]);

						const auto len = flame::ray_intersect_plane(ray_origin, ray_vector, translation_plane);
						auto local_pos = ray_origin + ray_vector * len - model_position;
						rotation_vector_source = glm::normalize(local_pos);
						rotation_angle_origin = compute_angle_on_plan();
					}
				}

				if (using_)
				{
					ImGui::CaptureMouseFromApp();
					rotation_angle = compute_angle_on_plan();
					if (enable_snap)
					{
						auto snap_in_radian = glm::radians(rotate_snap);
						compute_snap(&rotation_angle, snap_in_radian);
					}

					auto rotation_axis_local_space = glm::normalize(flame::transform(glm::vec3(translation_plane), model_matrix_inverse));

					auto delta_rotation = glm::rotate(rotation_angle - rotation_angle_origin, rotation_axis_local_space);
					rotation_angle_origin = rotation_angle;

					if (apply_rotation_localy)
						target->set_axis(model_matrix * delta_rotation);
					else
						target->right_rotate(delta_rotation);

					if (!io.MouseDown[0])
						using_ = false;

					type = this->type;
				}
				break;
			}
			case SCALE:
			{
				if (!using_)
				{
					type = get_scale_type();
					if (can_activate() && type != NONE)
					{
						ImGui::CaptureMouseFromApp();
						using_ = true;
						this->type = (TransType)type;
						const glm::vec3 move_plane_normal[] = { model_matrix[1], model_matrix[2], model_matrix[0], model_matrix[2], model_matrix[1], model_matrix[0], -camera_dir };

						translation_plane = flame::plane(model_position, move_plane_normal[type - SCALE_X]);
						const auto len = flame::ray_intersect_plane(ray_origin, ray_vector, translation_plane);
						translation_plane_origin = ray_origin + ray_vector * len;
						matrix_origin = model_position;
						scale = glm::vec3(1.f);
						relative_origin = (translation_plane_origin - model_position) / screen_factor;
						scale_value_origin = glm::vec3(glm::length(glm::vec3(model_source[0])), glm::length(glm::vec3(model_source[1])), glm::length(glm::vec3(model_source[2])));
						save_mouse_pos_x = io.MousePos.x;
					}
				}

				if (using_)
				{
					ImGui::CaptureMouseFromApp();
					const auto len = flame::ray_intersect_plane(ray_origin, ray_vector, translation_plane);
					auto new_pos = ray_origin + ray_vector * len;
					auto new_origin = new_pos - relative_origin * screen_factor;
					auto delta = new_origin - model_position;

					if (this->type >= SCALE_X && this->type <= SCALE_Z)
					{
						auto axis_index = this->type - SCALE_X;
						const auto &axis_value = *(glm::vec3*)&model_matrix[axis_index];
						auto length_on_axis = glm::dot(axis_value, delta);
						delta = axis_value * length_on_axis;

						auto base_vector = translation_plane_origin - model_position;
						auto ratio = glm::dot(axis_value, base_vector + delta) / glm::dot(axis_value, base_vector);

						scale[axis_index] = glm::max(ratio, 0.001f);
					}
					else
					{
						auto scale_delta = (io.MousePos.x - save_mouse_pos_x)  * 0.01f;
						scale = glm::vec3(glm::max(1.f + scale_delta, 0.001f));
					}

					if (enable_snap)
					{
						glm::vec3 scale_snap(scale_snap);
						compute_snap(scale, scale_snap);
					}

					scale = glm::max(scale, glm::vec3(0.001f));

					target->set_scale(scale * scale_value_origin);

					if (!io.MouseDown[0])
						using_ = false;

					type = this->type;
				}
				break;
			}
		}
	}

	ImU32 colors[7];
	compute_colors(colors, type);

	const int translation_info_index[] = { 
		0,0,0, 
		1,0,0, 
		2,0,0, 
		0,1,0, 
		1,2,0, 
		0,2,1, 
		0,1,2 
	};

	switch (operation)
	{
		case TRANSLATE:
		{
			const auto origin = world_to_screen(model_position, proj_view);
			const Vec4 plane_border_color[] = {
				Vec4(0.7f, 0.f, 0.f, 1.f),
				Vec4(0.f, 0.f, 0.ff, 1.f),
				Vec4(0.f, 0.ff, 0.f, 1.f)
			};

			auto below_axis_limit = false;
			auto below_plane_limit = false;
			for (auto i = 0; i < 3; ++i)
			{
				glm::vec3 dir_plane_x, dir_plane_y;
				compute_tripod_axis_and_visibility(i, dir_plane_x, dir_plane_y, below_axis_limit, below_plane_limit);

				if (below_axis_limit)
				{
					auto base_s_space = world_to_screen(dir_plane_x * 0.1f * screen_factor, mvp);
					auto world_dir_s_space = world_to_screen(dir_plane_x * screen_factor, mvp);

					draw_list->AddLine(base_s_space, world_dir_s_space, colors[i + 1], 3.f);

					auto dir(origin - world_dir_s_space);

					auto d = glm::sqrt(ImLengthSqr(dir));
					dir /= d;
					dir *= 6.f;

					ImVec2 ortogonalDir(dir.y, -dir.x);
					auto a(world_dir_s_space + dir);
					draw_list->AddTriangleFilled(world_dir_s_space - dir, a + ortogonalDir, a - ortogonalDir, colors[i + 1]);

					if (axis_factor[i] < 0.f)
						draw_hatched_axis(dir_plane_x);
				}

				if (below_plane_limit)
				{
					ImVec2 screen_quad_pts[4];
					for (int j = 0; j < 4; ++j)
					{
						glm::vec3 corner_world_pos = (dir_plane_x * quad_uv[j * 2] + dir_plane_y * quad_uv[j * 2 + 1]) * screen_factor;
						screen_quad_pts[j] = world_to_screen(corner_world_pos, mvp);
					}
					draw_list->AddPolyline(screen_quad_pts, 4, plane_border_color[i], true, 1.0f);
					draw_list->AddConvexPolyFilled(screen_quad_pts, 4, colors[i + 4]);
				}
			}

			draw_list->AddCircleFilled(screen_square_center, 6.f, colors[0], 16);

			if (using_)
			{
				const Vec4 translation_line_color = Vec4(0.66f);

				auto source_pos_on_screen = world_to_screen(matrix_origin, proj_view);
				auto destination_pos_on_screen = world_to_screen(model_position, proj_view);
				glm::vec2 dif(destination_pos_on_screen.x - source_pos_on_screen.x, destination_pos_on_screen.y - source_pos_on_screen.y);
				dif = glm::normalize(dif);
				dif *= 5.f;
				draw_list->AddCircle(source_pos_on_screen, 6.f, translation_line_color);
				draw_list->AddCircle(destination_pos_on_screen, 6.f, translation_line_color);
				draw_list->AddLine(ImVec2(source_pos_on_screen.x + dif.x, source_pos_on_screen.y + dif.y), ImVec2(destination_pos_on_screen.x - dif.x, destination_pos_on_screen.y - dif.y), translation_line_color, 2.f);

				const char *translation_infos[] = { 
					"X: %5.3f", 
					"Y: %5.3f", 
					"Z: %5.3f", 
					"X: %5.3f Y: %5.3f", 
					"Y: %5.3f Z: %5.3f", 
					"X: %5.3f Z: %5.3f", 
					"X: %5.3f Y: %5.3f Z: %5.3f" 
				};
				char tmps[512];
				auto delta_info = model_position - matrix_origin;
				int component_info_index = (type - MOVE_X) * 3;
				ImFormatString(tmps, sizeof(tmps), translation_infos[type - MOVE_X], delta_info[translation_info_index[component_info_index]], delta_info[translation_info_index[component_info_index + 1]], delta_info[translation_info_index[component_info_index + 2]]);
				draw_list->AddText(ImVec2(destination_pos_on_screen.x + 15, destination_pos_on_screen.y + 15), 0xFF000000, tmps);
				draw_list->AddText(ImVec2(destination_pos_on_screen.x + 14, destination_pos_on_screen.y + 14), 0xFFFFFFFF, tmps);
			}
			break;
		}
		case ROTATE:
		{
			const auto screen_rotate_size = 0.06f;
			const auto half_circle_segment_count = 64;

			auto camera_to_model_normalized = flame::transform(glm::normalize(model_position - camera_position), model_matrix_inverse);

			radius_square_center = screen_rotate_size * window_size.y;
			for (auto axis = 0; axis < 3; axis++)
			{
				ImVec2 circle_pos[half_circle_segment_count];

				auto angle_start = atan2f(camera_to_model_normalized[(4 - axis) % 3], camera_to_model_normalized[(3 - axis) % 3]) + M_PI * 0.5f;

				for (auto i = 0; i < half_circle_segment_count; i++)
				{
					auto ng = angle_start + M_PI * ((float)i / (float)half_circle_segment_count);
					auto axis_pos = glm::vec3(glm::cos(ng), glm::sin(ng), 0.f);
					auto pos = glm::vec3(axis_pos[axis], axis_pos[(axis + 1) % 3], axis_pos[(axis + 2) % 3]) * screen_factor;
					circle_pos[i] = world_to_screen(pos, mvp);
				}

				float radiusAxis = sqrtf((ImLengthSqr(world_to_screen(model_position, proj_view) - circle_pos[0])));
				if (radiusAxis > radius_square_center)
					radius_square_center = radiusAxis;

				draw_list->AddPolyline(circle_pos, half_circle_segment_count, colors[3 - axis], false, 2);
			}
			draw_list->AddCircle(world_to_screen(model_position, proj_view), radius_square_center, colors[0], 64, 3.f);

			if (using_)
			{
				ImVec2 circle_pos[half_circle_segment_count + 1];

				circle_pos[0] = world_to_screen(model_position, proj_view);
				for (auto i = 1; i < half_circle_segment_count; i++)
				{
					auto ng = rotation_angle * ((float)(i - 1) / (float)(half_circle_segment_count - 1));
					auto rotate_vector_matrix = glm::rotate(ng, glm::vec3(translation_plane));
					auto pos = flame::transform(rotation_vector_source, rotate_vector_matrix);
					pos *= screen_factor;
					circle_pos[i] = world_to_screen(pos + model_position, proj_view);
				}
				draw_list->AddConvexPolyFilled(circle_pos, half_circle_segment_count, 0x801080FF);
				draw_list->AddPolyline(circle_pos, half_circle_segment_count, 0xFF1080FF, true, 2);

				auto destination_pos_on_screen = circle_pos[1];
				const char *rotation_infos[] = { 
					"X: %5.2f deg %5.2f rad", 
					"Y: %5.2f deg %5.2f rad", 
					"Z: %5.2f deg %5.2f rad", 
					"Screen: %5.2f deg %5.2f rad" 
				};
				char tmps[512];
				ImFormatString(tmps, sizeof(tmps), rotation_info_mask[type - ROTATE_X], (rotation_angle / M_PI) * 180.f, rotation_angle);
				draw_list->AddText(ImVec2(destination_pos_on_screen.x + 15, destination_pos_on_screen.y + 15), 0xFF000000, tmps);
				draw_list->AddText(ImVec2(destination_pos_on_screen.x + 14, destination_pos_on_screen.y + 14), 0xFFFFFFFF, tmps);
			}
			break;
		}
		case SCALE:
		{
			glm::vec3 scale_display(1.f);

			if (using_)
				scale_display = scale;

			for (auto i = 0; i < 3; i++)
			{
				glm::vec3 dir_plane_x, dir_plane_y;
				bool below_axis_limit, below_plane_limit;
				compute_tripod_axis_and_visibility(i, dir_plane_x, dir_plane_y, below_axis_limit, below_plane_limit);

				if (below_axis_limit)
				{
					auto base_s_space = world_to_screen(dir_plane_x * 0.1f * screen_factor, mvp);
					auto world_dir_s_space_no_scale = world_to_screen(dir_plane_x * screen_factor, mvp);
					auto world_dir_s_space = world_to_screen((dir_plane_x * scale_display[i]) * screen_factor, mvp);

					if (using_)
					{
						draw_list->AddLine(base_s_space, world_dir_s_space_no_scale, 0xFF404040, 3.f);
						draw_list->AddCircleFilled(world_dir_s_space_no_scale, 6.f, 0xFF404040);
					}

					draw_list->AddLine(base_s_space, world_dir_s_space, colors[i + 1], 3.f);
					draw_list->AddCircleFilled(world_dir_s_space, 6.f, colors[i + 1]);

					if (axis_factor[i] < 0.f)
						draw_hatched_axis(dir_plane_x * scale_display[i]);
				}
			}

			draw_list->AddCircleFilled(screen_square_center, 6.f, colors[0], 32);

			if (using_)
			{
				auto destination_pos_on_screen = world_to_screen(model_position, proj_view);

				char tmps[512];
				const char *scale_infos[] = { 
					"X: %5.2f", 
					"Y: %5.2f", 
					"Z: %5.2f", 
					"XYZ: %5.2f" 
				};
				int component_info_index = (type - SCALE_X) * 3;
				ImFormatString(tmps, sizeof(tmps), scale_info_mask[type - SCALE_X], scale_display[translation_info_index[component_info_index]]);
				draw_list->AddText(ImVec2(destination_pos_on_screen.x + 15, destination_pos_on_screen.y + 15), 0xFF000000, tmps);
				draw_list->AddText(ImVec2(destination_pos_on_screen.x + 14, destination_pos_on_screen.y + 14), 0xFFFFFFFF, tmps);
			}
			break;
		}
	}

	draw_list->PopClipRect();
}
