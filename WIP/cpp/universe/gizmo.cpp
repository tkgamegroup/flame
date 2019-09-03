namespace flame
{
	namespace ui
	{
		struct GizmoPrivate
		{
			int sel_axis;

			Vec2 window_pos;
			Vec2 window_size;

			Mat4 proj_view;
			Vec3 obj_pos;
			Mat3 obj_axis;
			Mat3 obj_axis_inv;
			Mat4 mvp;

			Vec3 ray_ori;
			Vec3 ray_vec;

			float radius_square_center;
			Vec2 screen_square_center;
			Vec2 screen_square_min;
			Vec2 screen_square_max;

			float screen_factor;
			Vec3 relative_origin;

			Plane trans_plane;
			Vec3 trans_plane_origin;
			Vec3 matrix_origin;

			float scale;
			float scale_value_origin;

			Vec3 rotation_vector_source;
			float rotation_angle;
			float rotation_angle_origin;

			bool is_using;

			Vec2 get_screen_pos(const Vec3 &pos)
			{
				auto p = mvp * Vec4(pos, 1.f);
				p /= p.w;
				return window_pos + (Vec2(p) / 2.f + 0.5f) * window_size;
			}

			bool is_pos_on_screen(const Vec3 &pos)
			{
				auto p = mvp * Vec4(pos, 1.f);
				p /= p.w;
				return p.z > 0.f && p.z < 1.f &&
					p.x > -1.f && p.x < 1.f &&
					p.y > -1.f && p.y < 1.f;
			}

			float compute_angle_on_plane()
			{
				auto local_pos = (ray_ori + ray_vec * 
					trans_plane.intersect(ray_ori, ray_vec) - obj_pos);
				local_pos.normalize();

				auto perpendicular_vector = cross(rotation_vector_source, trans_plane.normal);
				perpendicular_vector.normalize();
				auto acos_angle = clamp(dot(local_pos, rotation_vector_source), -0.9999f, 0.9999f);
				auto angle = acos(acos_angle);
				angle *= (dot(local_pos, perpendicular_vector) < 0.f) ? 1.f : -1.f;
				return angle;
			}
		};

		bool Gizmo::is_using() const
		{
			return _priv->is_using;
		}

		bool Gizmo::show(Instance *ui, TransType type, graphics::Camera *camera, ThreeDWorld::Object *o)
		{
			auto changed = false;

			//auto wnd_rect = ui->get_curr_window_rect();
			//_priv->window_pos = wnd_rect.min;
			//_priv->window_size = Vec2(wnd_rect.width(), wnd_rect.height());
			//_priv->proj_view = camera->proj * camera->view;
			//_priv->obj_pos = o->pos;
			//_priv->obj_axis = Mat3(
			//	Vec3(o->mat[0]).get_normalized(),
			//	Vec3(o->mat[1]).get_normalized(),
			//	Vec3(o->mat[2]).get_normalized());
			//_priv->obj_axis_inv = _priv->obj_axis.get_inversed();
			//_priv->mvp = _priv->proj_view * (
			//	type == TransMove ?
			//	Mat4(Mat3(1.f), o->pos) : 
			//	Mat4(_priv->obj_axis, o->pos)
			//);
			//_priv->screen_factor = 0.1f * (_priv->proj_view * Vec4(o->pos, 1.f)).w;
			//_priv->screen_square_center = _priv->get_screen_pos(Vec3(0.f));
			//_priv->screen_square_min = _priv->screen_square_center - 10.f;
			//_priv->screen_square_max = _priv->screen_square_center + 10.f;

			//auto vp_inv = _priv->proj_view.get_inversed();
			//auto mpos_f = Vec2((Vec2(ui->get_mouse_pos()) - _priv->window_pos) / _priv->window_size) * 2.f - 1.f;
			//{
			//	auto p = vp_inv * Vec4(mpos_f, -1.f, 1.f);
			//	p /= p.w;
			//	_priv->ray_ori = p;
			//}
			//{
			//	auto p = vp_inv * Vec4(mpos_f, 1.f, 1.f);
			//	p /= p.w;
			//	_priv->ray_vec = Vec3(p) - _priv->ray_ori;
			//	_priv->ray_vec.normalize();
			//}

			//switch (type)
			//{
			//case TransMove:
			//	if (_priv->is_using)
			//	{
			//		auto new_pos = _priv->ray_ori + _priv->ray_vec *
			//			_priv->trans_plane.intersect(_priv->ray_ori, _priv->ray_vec);
			//		auto new_origin = new_pos - _priv->relative_origin * _priv->screen_factor;
			//		auto delta = new_origin - o->pos;

			//		auto axis = Vec3::axis(_priv->sel_axis);
			//		delta = axis * dot(axis, delta);

			//		o->pos += delta;
			//		changed = true;

			//		if (!ui->is_mouse_down(0))
			//			_priv->is_using = false;
			//	}
			//	else
			//	{
			//		if (ui->is_curr_window_hovered() && ui->is_mouse_just_down(0))
			//		{
			//			_priv->sel_axis = -1;
			//			for (auto i = 0; i < 3; i++)
			//			{
			//				auto dir_0 = Vec3::axis(i);
			//				auto dir_1 = Vec3::axis((i + 1) % 3);
			//				auto dir_2 = Vec3::axis((i + 2) % 3);

			//				_priv->trans_plane = Plane(dir_1, o->pos);
			//				_priv->trans_plane_origin = _priv->ray_ori + _priv->ray_vec *
			//					_priv->trans_plane.intersect(_priv->ray_ori, _priv->ray_vec);
			//				_priv->relative_origin = (_priv->trans_plane_origin - o->pos) / _priv->screen_factor;
			//				auto d0 = dot(dir_0, _priv->relative_origin);
			//				auto d1 = dot(dir_2, _priv->relative_origin);
			//				if (d1 > -0.1f && d1 < 0.1f && d0 > 0.f && d0 < 1.f)
			//				{
			//					_priv->sel_axis = i;
			//					_priv->is_using = true;
			//					break;
			//				}
			//			}
			//		}
			//	}
			//	break;
			//case TransRotate:
			//	if (!_priv->is_using)
			//	{
			//		if (ui->is_curr_window_hovered() && ui->is_mouse_just_down(0))
			//		{
			//			_priv->sel_axis = -1;
			//			for (auto i = 0; i < 3; i++)
			//			{
			//				_priv->trans_plane = Plane(_priv->obj_axis[i], o->pos);
			//				auto local_pos = _priv->ray_ori + _priv->ray_vec * 
			//					_priv->trans_plane.intersect(_priv->ray_ori, _priv->ray_vec) - o->pos;

			//				_priv->rotation_vector_source = local_pos.get_normalized();
			//				_priv->rotation_angle_origin = _priv->compute_angle_on_plane();
			//				auto distance = local_pos.length() / _priv->screen_factor;
			//				if (distance > 0.9f && distance < 1.1f)
			//				{
			//					_priv->sel_axis = i;
			//					_priv->is_using = true;
			//					break;
			//				}
			//			}
			//		}
			//	}
			//	if (_priv->is_using)
			//	{
			//		_priv->rotation_angle = _priv->compute_angle_on_plane();
			//		auto dist_ang = _priv->rotation_angle - _priv->rotation_angle_origin;
			//		dist_ang *= RAD_ANG;
			//		_priv->rotation_angle_origin = _priv->rotation_angle;

			//		switch (_priv->sel_axis)
			//		{
			//		case 0:
			//			o->euler.pitch += dist_ang;
			//			break;
			//		case 1:
			//			o->euler.yaw += dist_ang;
			//			break;
			//		case 2:
			//			o->euler.roll += dist_ang;
			//			break;
			//		}
			//		changed = true;

			//		if (!ui->is_mouse_down(0))
			//			_priv->is_using = false;
			//	}
			//	break;
			//case TransScale:
			//	if (!_priv->is_using)
			//	{
			//		if (ui->is_curr_window_hovered() && ui->is_mouse_just_down(0))
			//		{
			//			_priv->sel_axis = -1;
			//			for (auto i = 0; i < 3; i++)
			//			{
			//				auto dir_0 = _priv->obj_axis[i];
			//				auto dir_1 = _priv->obj_axis[(i + 1) % 3];
			//				auto dir_2 = _priv->obj_axis[(i + 2) % 3];

			//				_priv->trans_plane = Plane(dir_1, o->pos);
			//				_priv->trans_plane_origin = _priv->ray_ori + _priv->ray_vec *
			//					_priv->trans_plane.intersect(_priv->ray_ori, _priv->ray_vec);
			//				_priv->relative_origin = (_priv->trans_plane_origin - o->pos) / _priv->screen_factor;
			//				auto d0 = dot(dir_0, _priv->relative_origin);
			//				auto d1 = dot(dir_2, _priv->relative_origin);
			//				if (d1 > -0.1f && d1 < 0.1f && d0 > 0.f && d0 < 1.f)
			//				{
			//					_priv->sel_axis = i;
			//					_priv->is_using = true;
			//					_priv->scale = 1.f;
			//					_priv->scale_value_origin = o->scale[i];
			//					break;
			//				}
			//			}
			//		}
			//	}
			//	else
			//	{
			//		auto new_pos = _priv->ray_ori + _priv->ray_vec *
			//			_priv->trans_plane.intersect(_priv->ray_ori, _priv->ray_vec);
			//		auto new_origin = new_pos - _priv->relative_origin * _priv->screen_factor;
			//		auto delta = new_origin - o->pos;

			//		auto axis = _priv->obj_axis[_priv->sel_axis];
			//		delta = axis * dot(axis, delta);

			//		auto base_vector = _priv->trans_plane_origin - o->pos;
			//		auto ratio = dot(axis, base_vector + delta) / dot(axis, base_vector);

			//		_priv->scale = max(ratio, 0.001f);
			//		o->scale[_priv->sel_axis] = _priv->scale * _priv->scale_value_origin;
			//		changed = true;

			//		if (!ui->is_mouse_down(0))
			//			_priv->is_using = false;
			//	}
			//	break;
			//}

			//const Vec4 direction_color[] = {
			//	Vec4(1.f, 0.f, 0.f, 1.f),
			//	Vec4(0.f, 1.f, 0.f, 1.f),
			//	Vec4(0.f, 0.f, 1.f, 1.f)
			//};

			//auto dl = ui->get_curr_window_drawlist();

			//switch (type)
			//{
			//case TransMove:
			//	for (auto i = 0; i < 3; i++)
			//	{
			//		auto col = _priv->sel_axis == i ? Vec4(1.f, 1.f, 0.f, 1.f) :
			//			direction_color[i];

			//		auto a = _priv->get_screen_pos(Vec3(0.f));
			//		auto b = _priv->get_screen_pos(Vec3::axis(i) * _priv->screen_factor);
			//		dl.add_line(a, b, col, 3.f);

			//		auto dir(b - a);
			//		dir.normalize();
			//		dir *= 6.f;

			//		Vec2 ortogonalDir(dir.y, -dir.x);
			//		dl.add_triangle_filled(b + dir, b - dir + ortogonalDir, b - dir - ortogonalDir,
			//			col);
			//	}
			//	break;
			//case TransRotate:
			//{
			//	const auto screen_rotate_size = 0.06f;
			//	const auto half_circle_segment_count = 64;

			//	for (auto i = 0; i < 3; i++)
			//	{
			//		Vec2 circle_pos[half_circle_segment_count];
			//		for (auto j = 0; j < half_circle_segment_count; j++)
			//		{
			//			auto ng = 2.f * PI * ((float)j / (float)half_circle_segment_count);
			//			auto axis_pos = Vec3(0.f, cos(ng), sin(ng));
			//			auto pos = Vec3(axis_pos[(3 - i) % 3], axis_pos[(4 - i) % 3], axis_pos[(5 - i) % 3]) * _priv->screen_factor;
			//			circle_pos[j] = _priv->get_screen_pos(pos);
			//		}
			//		dl.add_polyline(circle_pos, half_circle_segment_count, 
			//			_priv->sel_axis == i ? Vec4(1.f, 1.f, 0.f, 1.f) : direction_color[i], 2.f);
			//	}

			//}
			//	break;
			//case TransScale:
			//{
			//	for (auto i = 0; i < 3; i++)
			//	{
			//		auto col = _priv->sel_axis == i ? Vec4(1.f, 1.f, 0.f, 1.f) :
			//			direction_color[i];

			//		auto a = _priv->get_screen_pos(Vec3(0.f));
			//		auto b = _priv->get_screen_pos(_priv->obj_axis[i] * 
			//			((_priv->is_using && _priv->sel_axis == i) ? _priv->scale : 1.f) * _priv->screen_factor);
			//		dl.add_line(a, b, col, 3.f);
			//		dl.add_circle_filled(b, 6.f, col);
			//	}
			//}
			//	break;
			//}

			return changed;
		}

		Gizmo *create_gizmo() 
		{
			auto g = new Gizmo;

			g->_priv = new GizmoPrivate;
			g->_priv->is_using = false;
			g->_priv->sel_axis = -1;

			return g;
		}

		void destroy_gizmo(Gizmo *g)
		{
			delete g->_priv;
			delete g;
		}
	}
}
