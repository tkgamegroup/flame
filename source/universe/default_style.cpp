#include <flame/math.h>
#include <flame/universe/universe.h>
#include <flame/universe/default_style.h>

namespace flame
{
	DefaultStyle::DefaultStyle()
	{
		set_to_dark();
	}

	void DefaultStyle::set_to_light()
	{
		text_color_normal = Vec4c(0, 0, 0, 255);
		text_color_hovering_or_active = Vec4c(255, 255, 255, 255);
		window_color = Vec4c(0.94f, 0.94f, 0.94f, 1.00f);
		frame_color_normal = Vec4c(1.00f, 1.00f, 1.00f, 1.00f);
		frame_color_hovering = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.40f);
		frame_color_active = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.67f);
		button_color_normal = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.40f);
		button_color_hovering = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 1.00f);
		button_color_active = Vec4c(color(Vec3f(45.f, 0.73f, 0.97f)), 1.00f);
		header_color_normal = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.31f);
		header_color_hovering = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.80f);
		header_color_active = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 1.00f);
		sdf_scale = 1.f;
	}

	void DefaultStyle::set_to_dark()
	{
		text_color_normal = Vec4c(255, 255, 255, 255);
		text_color_hovering_or_active = Vec4c(180, 180, 180, 255);
		window_color = Vec4c(0.06f, 0.06f, 0.06f, 0.94f);
		frame_color_normal = Vec4c(color(Vec3f(55.f, 0.67f, 0.47f)), 0.54f);
		frame_color_hovering = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.40f);
		frame_color_active = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.67f);
		button_color_normal = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.40f);
		button_color_hovering = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 1.00f);
		button_color_active = Vec4c(color(Vec3f(49.f, 0.93f, 0.97f)), 1.00f);
		header_color_normal = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.31f);
		header_color_hovering = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.80f);
		header_color_active = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 1.00f);
		sdf_scale = 1.f;
	}

	DefaultStyle default_style;
}
