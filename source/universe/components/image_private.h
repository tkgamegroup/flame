#pragma once

#include "element_private.h"
#include <flame/universe/components/image.h>
#include "../systems/type_setting_private.h"

namespace flame
{
	struct cElementPrivate;

	struct cImageBridge : cImage
	{
		void set_src(const char* src) override;
	};

	struct cImagePrivate : cImageBridge, cElement::Drawer, sTypeSetting::AutoSizer // R ~ on_*
	{
		int res_id = -1;
		int tile_id = -1;
		Vec2f uv0 = Vec2f(0.f);
		Vec2f uv1 = Vec2f(1.f);
		Vec3c color = Vec3c(255);

		std::string src;

		cElementPrivate* element = nullptr; // R ref
		sTypeSettingPrivate* type_setting = nullptr; // R ref
		graphics::Canvas* canvas = nullptr; // R ref

		int get_res_id() const override { return res_id; }
		void set_res_id(int id) override;
		int get_tile_id() const override { return tile_id; }
		void set_tile_id(int id) override;

		const char* get_src() const override { return src.c_str(); }
		void set_src(const std::string& src);

		bool get_auto_size() const override { return auto_width || auto_height; }
		void set_auto_size(bool a) override { auto_width = auto_height = a; }

		void on_gain_element();
		void on_lost_element();
		void on_gain_type_setting();
		void on_lost_type_setting();
		void on_gain_canvas();

		void apply_src();

		void mark_size_dirty();

		void draw(graphics::Canvas* canvas) override;

		Vec2f measure() override;

		void on_added() override;
		void on_local_message(Message msg, void* p) override;
	};

	inline void cImageBridge::set_src(const char* src)
	{
		((cImagePrivate*)this)->set_src(src);
	}
}
