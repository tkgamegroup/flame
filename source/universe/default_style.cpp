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
		text_color_else = Vec4c(255, 255, 255, 255);
		window_color = Vec4c(0.94f * 255.f, 0.94f * 255.f, 0.94f * 255.f, 1.00f * 255.f);
		frame_color_normal = Vec4c(255, 255, 255, 255);
		frame_color_hovering = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.40f * 255.f);
		frame_color_active = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.67f * 255.f);
		button_color_normal = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.40f * 255.f);
		button_color_hovering = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 1.00f * 255.f);
		button_color_active = Vec4c(color(Vec3f(45.f, 0.73f, 0.97f)), 1.00f * 255.f);
		header_color_normal = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.31f * 255.f);
		header_color_hovering = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.80f * 255.f);
		header_color_active = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 1.00f * 255.f);
		selected_color_normal = Vec4c(color(Vec3f(22.f, 0.73f, 0.97f)), 0.31f * 255.f);
		selected_color_hovering = Vec4c(color(Vec3f(22.f, 0.73f, 0.97f)), 0.80f * 255.f);
		selected_color_active = Vec4c(color(Vec3f(22.f, 0.73f, 0.97f)), 1.00f * 255.f);
		sdf_scale = 1.f;
	}

	void DefaultStyle::set_to_dark()
	{
		text_color_normal = Vec4c(255, 255, 255, 255);
		text_color_else = Vec4c(180, 180, 180, 255);
		window_color = Vec4c(0.06f * 255.f, 0.06f * 255.f, 0.06f * 255.f, 0.94f * 255.f);
		frame_color_normal = Vec4c(color(Vec3f(55.f, 0.67f, 0.47f)), 0.54f * 255.f);
		frame_color_hovering = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.40f * 255.f);
		frame_color_active = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.67f * 255.f);
		button_color_normal = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.40f * 255.f);
		button_color_hovering = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 1.00f * 255.f);
		button_color_active = Vec4c(color(Vec3f(49.f, 0.93f, 0.97f)), 1.00f * 255.f);
		header_color_normal = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.31f * 255.f);
		header_color_hovering = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 0.80f * 255.f);
		header_color_active = Vec4c(color(Vec3f(52.f, 0.73f, 0.97f)), 1.00f * 255.f);
		selected_color_normal = Vec4c(color(Vec3f(22.f, 0.73f, 0.97f)), 0.31f * 255.f);
		selected_color_hovering = Vec4c(color(Vec3f(22.f, 0.73f, 0.97f)), 0.80f * 255.f);
		selected_color_active = Vec4c(color(Vec3f(22.f, 0.73f, 0.97f)), 1.00f * 255.f);
		sdf_scale = 1.f;
	}

	DefaultStyle default_style;
}
