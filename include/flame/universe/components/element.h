#pragma once

#include <flame/universe/component.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct cElement : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cElement";
		inline static auto type_hash = ch(type_name);

		struct Drawer
		{
			virtual void draw(graphics::Canvas* canvas) = 0;
		};

		cElement() :
			Component(type_name, type_hash)
		{
		}

		virtual float get_x() const = 0;
		virtual void set_x(float x) = 0;

		virtual float get_y() const = 0;
		virtual void set_y(float y) = 0;

		virtual float get_width() const = 0;
		virtual void set_width(float w) = 0;

		virtual float get_height() const = 0;
		virtual void set_height(float h) = 0;

		// left top right bottom
		virtual Vec4f get_padding() = 0;
		// left top right bottom
		virtual void set_padding(const Vec4f& p) = 0;

		// 0 - 1
		virtual float get_pivotx() const = 0;
		// 0 - 1
		virtual void set_pivotx(float p) = 0;

		// 0 - 1
		virtual float get_pivoty() const = 0;
		// 0 - 1
		virtual void set_pivoty(float p) = 0;

		virtual float get_scalex() const = 0;
		virtual void set_scalex(float s) = 0;

		virtual float get_scaley() const = 0;
		virtual void set_scaley(float s) = 0;

		virtual float get_rotation() const = 0;
		virtual void set_rotation(float angle) = 0;

		virtual float get_skewx() const = 0;
		virtual void set_skewx(float angle) = 0;

		virtual float get_skewy() const = 0;
		virtual void set_skewy(float angle) = 0;

		// 0-3: edge points, 4-7: content points, left-top, right-top, right-bottom, left-bottom
		virtual Vec2f get_point(uint idx) = 0;

		virtual Vec4c get_fill_color() = 0;
		virtual void set_fill_color(const Vec4c& c) = 0;

		virtual float get_border() = 0;
		virtual void set_border(float b) = 0;
		virtual Vec4c get_border_color() = 0;
		virtual void set_border_color(const Vec4c& c) = 0;

		virtual bool get_clipping() const = 0;
		virtual void set_clipping(bool c) = 0;

		virtual bool contains(const Vec2f& p) = 0;

		//FLAME_RV(float, alpha);
		//FLAME_RV(Vec4f, roundness);
		//FLAME_RV(uint, roundness_lod);

		//Vec4f clipped_rect;

		//Vec2f center() const
		//{
		//	return global_pos + global_size * 0.5f;
		//}

		//Vec2f content_min() const
		//{
		//	return global_pos + padding.xy() * global_scale;
		//}

		//Vec2f content_max() const
		//{
		//	return global_pos + global_size - padding.zw() * global_scale;
		//}

		//Vec2f content_size() const
		//{
		//	return global_size - Vec2f(padding.xz().sum(), padding.yw().sum()) * global_scale;
		//}

		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_alpha)(float a);
		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_roundness)(const Vec4f& r);
		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_frame_thickness)(float t);
		//FLAME_UNIVERSE_EXPORTS void FLAME_RF(set_frame_color)(const Vec4c& c);

		FLAME_UNIVERSE_EXPORTS static cElement* create();
	};
}
