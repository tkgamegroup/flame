namespace flame
{
	bool CameraComponent::on_message(Object *sender, Message msg)
	{
		switch (msg)
		{
			case MessageResolutionChange:
				update_proj();
				update_frustum();
				return true;
		}
	}

	void CameraComponent::on_update()
	{
		if (aux_updated_frame < get_parent()->get_transform_dirty_frame())
		{
			view_matrix = glm::inverse(get_parent()->get_matrix());
			update_frustum();
		}
	}

	void CameraComponent::update_frustum()
	{
		auto tanHfFovy = glm::tan(glm::radians(fovy * 0.5f));

		auto _y1 = near_plane * tanHfFovy;
		auto _z1 = _y1 * resolution.aspect();
		auto _y2 = far_plane * tanHfFovy;
		auto _z2 = _y2 * resolution.aspect();
		auto axis = get_parent()->get_axis();
		auto coord = get_parent()->get_coord();
		frustum_points[0] = -_z1 * axis[2] + _y1 * axis[1] + near_plane * axis[0] + coord;
		frustum_points[1] = _z1 * axis[2] + _y1 * axis[1] + near_plane * axis[0] + coord;
		frustum_points[2] = _z1 * axis[2] + -_y1 * axis[1] + near_plane * axis[0] + coord;
		frustum_points[3] = -_z1 * axis[2] + -_y1 * axis[1] + near_plane * axis[0] + coord;
		frustum_points[4] = -_z2 * axis[2] + _y2 * axis[1] + far_plane * axis[0] + coord;
		frustum_points[5] = _z2 * axis[2] + _y2 * axis[1] + far_plane * axis[0] + coord;
		frustum_points[6] = _z2 * axis[2] + -_y2 * axis[1] + far_plane * axis[0] + coord;
		frustum_points[7] = -_z2 * axis[2] + -_y2 * axis[1] + far_plane * axis[0] + coord;
		for (int i = 0; i < 4; i++)
		{
			auto y = frustum_points[i + 4].y;
			if (y < 0.f)
			{
				auto py = frustum_points[i + 4].y - frustum_points[i].y;
				if (py != 0.f)
				{
					frustum_points[i + 4].x -= y * ((frustum_points[i + 4].x - frustum_points[i].x) / py);
					frustum_points[i + 4].z -= y * ((frustum_points[i + 4].z - frustum_points[i].z) / py);
					frustum_points[i + 4].y = 0.f;
				}
			}
		}
	}
}
