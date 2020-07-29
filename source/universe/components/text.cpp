#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>
#include "../world_private.h"
#include "aligner_private.h"
#include "text_private.h"

namespace flame
{
	void cTextPrivate::set_text(const std::wstring& _text)
	{
		text = _text;
		if (element)
			element->mark_drawing_dirty();
		mark_size_dirty();
		((EntityPrivate*)entity)->report_data_changed(this, S<ch("text")>::v);
	}

	void cTextPrivate::set_font_size(uint fs)
	{
		if (font_size == fs)
			return;
		font_size = fs;
		if (element)
			element->mark_drawing_dirty();
		mark_size_dirty();
		((EntityPrivate*)entity)->report_data_changed(this, S<ch("font_size")>::v);
	}

	void cTextPrivate::mark_size_dirty()
	{
		if (type_setting && element && true/*auto_size*/)
			type_setting->add_to_sizing_list(this, (EntityPrivate*)entity);
	}

	void cTextPrivate::on_gain_element()
	{
		element->drawers.push_back(this);
		mark_size_dirty();
	}

	void cTextPrivate::on_lost_element()
	{
		std::erase_if(element->drawers, [&](const auto& i) {
			return i == this;
		});
	}

	void cTextPrivate::on_entered_world()
	{
		auto world = ((EntityPrivate*)entity)->world;
		type_setting = (sTypeSettingPrivate*)world->get_system(sTypeSetting::type_hash);
		mark_size_dirty();
		canvas = (graphics::Canvas*)world->find_object("Canvas");
		if (canvas)
			atlas = canvas->get_resource(0)->get_font_atlas();
	}

	void cTextPrivate::on_left_world()
	{
		if (type_setting)
			type_setting->remove_from_sizing_list(this);
		type_setting = nullptr;
		canvas = nullptr;
		atlas = nullptr;
	}

	void cTextPrivate::on_entity_component_added(Component* c)
	{
		if (c->type_hash == cAligner::type_hash)
			mark_size_dirty();
	}

	void cTextPrivate::draw(graphics::Canvas* canvas)
	{
		canvas->add_text(0, text.c_str(), nullptr, font_size, Vec4c(0, 0, 0, 255), element->points[4], Mat2f(element->transform));
	}

	Vec2f cTextPrivate::measure()
	{
		if (!atlas)
			return Vec2f(0.f);
		return Vec2f(atlas->text_size(font_size, text.c_str()));
	}

	//cTextPrivate::cTextPrivate()
	//{
	//	color = 0;
	//	auto_size = true;
	//}

	//void cTextPrivate::auto_set_size()
	//{
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
	//	report_data_changed(FLAME_CHASH("text"), sender);
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
	//	report_data_changed(FLAME_CHASH("color"), sender);
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
	//	report_data_changed(FLAME_CHASH("auto_size"), sender);
	//}

	cText* cText::create()
	{
		return f_new<cTextPrivate>();
	}
}
