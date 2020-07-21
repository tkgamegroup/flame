//#include <flame/universe/world.h>
//#include <flame/universe/components/aligner.h>

#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>
#include "../entity_private.h"
#include "../world_private.h"
#include "text_private.h"

namespace flame
{
	void cTextPrivate::set_text(const std::wstring& _text)
	{
		text = _text;
		if (element)
		{
			element->mark_drawing_dirty();
			if (type_setting && true/*auto_size*/)
				type_setting->add_to_sizing_list(this, (EntityPrivate*)entity);
		}
	}

	void cTextPrivate::on_added()
	{
		element = (cElementPrivate*)((EntityPrivate*)entity)->get_component(cElement::type_hash);
		element->drawers.push_back(this);
		if (type_setting && true/*auto_size*/)
			type_setting->add_to_sizing_list(this, (EntityPrivate*)entity);
	}

	void cTextPrivate::on_removed()
	{
		std::erase_if(element->drawers, [&](const auto& i) {
			return i == this;
		});
	}

	void cTextPrivate::on_entered_world()
	{
		auto world = ((EntityPrivate*)entity)->world;
		type_setting = (sTypeSettingPrivate*)world->get_system(sTypeSetting::type_hash);
		//if (auto_size)
			type_setting->add_to_sizing_list(this, (EntityPrivate*)entity);
		canvas = (graphics::Canvas*)world->find_object("Canvas");
	}

	void cTextPrivate::on_left_world()
	{
		if (type_setting)
			type_setting->remove_from_sizing_list(this);
		type_setting = nullptr;
		canvas = nullptr;
	}

	void cTextPrivate::on_entity_visibility_changed()
	{
		if (type_setting && true/*auto_size*/)
			type_setting->add_to_sizing_list(this, (EntityPrivate*)entity);
	}

	void cTextPrivate::draw(graphics::Canvas* canvas)
	{
		canvas->add_text(0, text.c_str(), 14, element->p00, Vec4c(0, 0, 0, 255));
	}

	Vec2f cTextPrivate::measure()
	{
		if (!canvas)
			return Vec2f(0.f);
		return Vec2f(canvas->get_resource(0)->get_font_atlas()->text_size(14, text.c_str()));
	}

	//cTextPrivate::cTextPrivate()
	//{
	//	font_atlas = nullptr;
	//	text.resize(1);
	//	text.v[0] = 0;
	//	font_size = 0;
	//	color = 0;
	//	auto_size = true;
	//}

	//void cTextPrivate::draw(graphics::Canvas* canvas)
	//{
	//	if (!element->clipped)
	//	{
	//		canvas->add_text(font_atlas, text.v, nullptr, font_size * element->global_scale, element->content_min(),
	//			color.copy().factor_w(element->alpha));
	//	}
	//}

	//void cTextPrivate::auto_set_size()
	//{
	//	auto s = Vec2f(font_atlas->text_size(font_size, text.v, nullptr)) + Vec2f(element->padding.xz().sum(), element->padding.yw().sum());
	//	element->set_size(s, this);
	//	auto aligner = entity->get_component(cAligner);
	//	if (aligner)
	//	{
	//		aligner->set_min_width(s.x(), this);
	//		aligner->set_min_height(s.y(), this);
	//	}
	//}

	//void cText::set_text(const wchar_t* text, int length, void* sender)
	//{
	//	auto thiz = (cTextPrivate*)this;
	//	if (text)
	//	{
	//		if (length == -1)
	//			length = wcslen(text);
	//		if (thiz->text.compare(text, length))
	//			return;
	//		thiz->text.assign(text, length);
	//	}
	//	data_changed(FLAME_CHASH("text"), sender);
	//}

	//void cText::set_font_size(uint s, void* sender)
	//{
	//	if (s == font_size)
	//		return;
	//	font_size = s;
	//	if (element)
	//	{
	//		element->mark_dirty();
	//		if (management && auto_size)
	//			management->add_to_sizing_list(this);
	//	}
	//	data_changed(FLAME_CHASH("font_size"), sender);
	//}

	//void cText::set_color(const Vec4c& c, void* sender)
	//{
	//	if (c == color)
	//		return;
	//	color = c;
	//	if (element)
	//	{
	//		element->mark_dirty();
	//		if (management && auto_size)
	//			management->add_to_sizing_list(this);
	//	}
	//	data_changed(FLAME_CHASH("color"), sender);
	//}

	//void cText::set_auto_size(bool v, void* sender)
	//{
	//	if (v == auto_size)
	//		return;
	//	auto_size = v;
	//	auto thiz = (cTextPrivate*)this;
	//	if (thiz->auto_size)
	//	{
	//		if (element)
	//		{
	//			element->mark_dirty();
	//			if (management && auto_size)
	//				management->add_to_sizing_list(this);
	//		}
	//	}
	//	data_changed(FLAME_CHASH("auto_size"), sender);
	//}

	cTextPrivate* cTextPrivate::create()
	{
		return f_new<cTextPrivate>();
	}

	cText* cText::create() { return cTextPrivate::create(); }
}
