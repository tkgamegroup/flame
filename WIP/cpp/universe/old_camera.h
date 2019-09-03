struct Camera
{
	Vec3 view_dir;
	Vec3 up_dir;
	Vec3 x_dir;

	Vec4 frustum_planes[6];

	void calc_ang()
	{
		y_ang = asin(view_dir.y) * RAD_ANG;
		x_ang = atan(view_dir.x / view_dir.z) * RAD_ANG;
		if (view_dir.y > 0.f)
			x_ang = 180.f - x_ang;
	}

	void update_frustum_plane()
	{
		frustum_planes[0].x = proj_view[0].w + proj_view[0].x;
		frustum_planes[0].y = proj_view[1].w + proj_view[1].x;
		frustum_planes[0].z = proj_view[2].w + proj_view[2].x;
		frustum_planes[0].w = proj_view[3].w + proj_view[3].x;
		frustum_planes[1].x = proj_view[0].w - proj_view[0].x;
		frustum_planes[1].y = proj_view[1].w - proj_view[1].x;
		frustum_planes[1].z = proj_view[2].w - proj_view[2].x;
		frustum_planes[1].w = proj_view[3].w - proj_view[3].x;
		frustum_planes[2].x = proj_view[0].w + proj_view[0].y;
		frustum_planes[2].y = proj_view[1].w + proj_view[1].y;
		frustum_planes[2].z = proj_view[2].w + proj_view[2].y;
		frustum_planes[2].w = proj_view[3].w + proj_view[3].y;
		frustum_planes[3].x = proj_view[0].w - proj_view[0].y;
		frustum_planes[3].y = proj_view[1].w - proj_view[1].y;
		frustum_planes[3].z = proj_view[2].w - proj_view[2].y;
		frustum_planes[3].w = proj_view[3].w - proj_view[3].y;
		frustum_planes[4].x = proj_view[0].w + proj_view[0].z;
		frustum_planes[4].y = proj_view[1].w + proj_view[1].z;
		frustum_planes[4].z = proj_view[2].w + proj_view[2].z;
		frustum_planes[4].w = proj_view[3].w + proj_view[3].z;
		frustum_planes[5].x = proj_view[0].w - proj_view[0].z;
		frustum_planes[5].y = proj_view[1].w - proj_view[1].z;
		frustum_planes[5].z = proj_view[2].w - proj_view[2].z;
		frustum_planes[5].w = proj_view[3].w - proj_view[3].z;

		for (auto i = 0; i < 6; i++)
		{
			auto l = Vec3(frustum_planes[i]).length();
			frustum_planes[i] /= l;
		}
	}

	bool frustum_check(const Vec3 & pos, float r)
	{
		auto p = Vec4(pos, 1.f);

		for (auto i = 0; i < 6; i++)
		{
			if (dot(frustum_planes[i], p) + r < 0.f)
				return false;
		}
		return true;
	}
};
