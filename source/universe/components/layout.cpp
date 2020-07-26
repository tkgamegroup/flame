#include "../world_private.h"
#include "element_private.h"
#include "aligner_private.h"
#include "layout_private.h"
#include "../systems/type_setting_private.h"
namespace flame
{
//	cLayoutPrivate::cLayoutPrivate(LayoutType _type)
//	{
//		column = 0;

//		scroll_offset = Vec2f(0.f);
//		content_size = Vec2f(0.f);
//	}
//
//	void cLayoutPrivate::set_content_size(const Vec2f& s)
//	{
//		if (s == content_size)
//			return;
//		content_size = s;
//		report_data_changed(FLAME_CHASH("content_size"), this);
//	}

	void cLayoutPrivate::apply_basic_h(cElementPrivate* e, cAlignerPrivate* a, bool free)
	{
		auto alignx = a ? a->alignx : (free ? AlignNone : AlignMin);
		auto p = ((a && a->absolute) ? Vec2f(0.f) : element->padding.xz()) + (a ? a->margin.xz() : Vec2f(0.f));
		switch (alignx)
		{
		case AlignMin:
			e->set_x(p[0]);
			break;
		case AlignMax:
			e->set_x(element->width - p[1] - e->width);
			break;
		case AlignMiddle:
			e->set_x(p[0] + (element->width - p.sum() - e->width) * 0.5f);
			break;
		case AlignMinMax:
			e->set_width(element->width - p.sum());
			e->set_x(p[0]);
			if (a)
				a->desired_size.x() = e->width;
			break;
		}
	}

	void cLayoutPrivate::apply_basic_v(cElementPrivate* e, cAlignerPrivate* a, bool free)
	{
		auto aligny = a ? a->aligny : (free ? AlignNone : AlignMin);
		auto p = ((a && a->absolute) ? Vec2f(0.f) : element->padding.yw()) + (a ? a->margin.yw() : Vec2f(0.f));
		switch (aligny)
		{
		case AlignMin:
			e->set_y(p[0]);
			break;
		case AlignMax:
			e->set_y(element->height - p[1] - e->height);
			break;
		case AlignMiddle:
			e->set_y(p[0] + (element->height - p.sum() - e->height) * 0.5f);
			break;
		case AlignMinMax:
			e->set_height(element->height - p.sum());
			e->set_y(p[0]);
			if (a)
				a->desired_size.y() = e->height;
			break;
		}
	}

	void cLayoutPrivate::judge_width(float w)
	{
		if (!auto_width)
			w = 0.f;
		else
			element->set_width(w);
		if (aligner)
			aligner->desired_size.x() = w;
	}

	void cLayoutPrivate::judge_height(float h)
	{
		if (!auto_height)
			h = 0.f;
		else
			element->set_height(h);
		if (aligner)
			aligner->desired_size.y() = h;
	}

	void cLayoutPrivate::update()
	{
//#ifdef _DEBUG
//		if (debug_level > 0)
//		{
//			debug_break();
//			debug_level = 0;
//		}
//#endif
//
		updating = true;

		std::vector<std::pair<cElementPrivate*, cAlignerPrivate*>> als[2];
		for (auto& c : ((EntityPrivate*)entity)->children)
		{
			if (c->global_visibility)
			{
				auto e = (cElementPrivate*)c->get_component(cElement::type_hash);
				auto a = (cAlignerPrivate*)c->get_component(cAligner::type_hash);
				if (e || a)
				{
					if (a && a->only_basic)
						als[1].emplace_back(e, a);
					else
						als[0].emplace_back(e, a);
				}
			}
		}

		switch (type)
		{
		case LayoutBasic:
			for (auto i = 0; i < 2; i++)
			{
				for (auto& al : als[i])
				{
					apply_basic_h(al.first, al.second, true);
					apply_basic_v(al.first, al.second, true);
				}
			}
			break;
		case LayoutHorizontal:
		{
			auto w = 0.f;
			auto h = 0.f;
			auto factor = 0.f;
			for (auto& al : als[0])
			{
				auto element = al.first;
				auto aligner = al.second;
				auto alignx = aligner ? aligner->alignx : AlignNone;
				auto aligny = aligner ? aligner->aligny : AlignNone;
				auto m = aligner ? aligner->margin : Vec4f(0.f);

				if (alignx == AlignMinMax)
				{
					factor += aligner->width_factor;
					w += aligner->desired_size.x();
				}
				else
					w += element->width;
				w += gap + m.xz().sum();
				h = max((aligny == AlignMinMax ? aligner->desired_size.y() : element->height) + m.yw().sum(), h);

			}
			if (!als[0].empty())
				w -= gap;
			//set_content_size(Vec2f(w, h));
			w += element->padding.xz().sum();
			h += element->padding.yw().sum();
			judge_width(w);
			judge_height(h);

			w = element->width - w;
			if (w > 0.f && factor > 0.f)
				w /= factor;
			else
				w = 0.f;
			auto x = element->padding[0];
			for (auto& al : als[0])
			{
				auto element = al.first;
				auto aligner = al.second;
				auto alignx = aligner ? aligner->alignx : AlignNone;
				auto m = (aligner ? aligner->margin.xz() : Vec2f(0.f));

				if (alignx == AlignMinMax)
					element->set_width(w * aligner->width_factor + aligner->desired_size.x());
				x += m[0];
				element->set_x(scrollx + x);
				x += element->width + gap + m[1];
			}
			for (auto& al : als[0])
				apply_basic_v(al.first, al.second, false);
			for (auto& al : als[1])
			{
				apply_basic_h(al.first, al.second, true);
				apply_basic_v(al.first, al.second, true);
			}
		}
			break;
		case LayoutVertical:
		{
			auto w = 0.f;
			auto h = 0.f;
			auto factor = 0.f;
			for (auto& al : als[0])
			{
				auto element = al.first;
				auto aligner = al.second;
				auto alignx = aligner ? aligner->alignx : AlignNone;
				auto aligny = aligner ? aligner->aligny : AlignNone;
				auto m = aligner ? aligner->margin : Vec4f(0.f);

				w = max((alignx == AlignMinMax ? aligner->desired_size.x() : element->width) + m.xz().sum(), w);
				if (aligny == AlignMinMax)
				{
					factor += aligner->height_factor;
					h += aligner->desired_size.y();
				}
				else
					h += element->height;
				h += gap + m.yw().sum();
			}
			if (!als[0].empty())
				h -= gap;
			//set_content_size(Vec2f(w, h));
			w += element->padding.xz().sum();
			h += element->padding.yw().sum();
			judge_width(w);
			judge_height(h);

			for (auto& al : als[0])
				apply_basic_h(al.first, al.second, false);
			h = element->height - h;
			if (h > 0.f && factor > 0.f)
				h /= factor;
			else
				h = 0.f;
			auto y = element->padding[1];
			for (auto& al : als[0])
			{
				auto element = al.first;
				auto aligner = al.second;
				auto aligny = aligner ? aligner->aligny : AlignNone;
				auto m = (aligner ? aligner->margin.yw() : Vec2f(0.f));

				if (aligny == AlignMinMax)
					element->set_height(h * aligner->height_factor + aligner->desired_size.y());
				y += m[0];
				element->set_y(scrolly + y);
				y += element->height + gap + m[1];
			}
			for (auto& al : als[1])
			{
				apply_basic_h(al.first, al.second, true);
				apply_basic_v(al.first, al.second, true);
			}
		}
			break;
		case LayoutTile:
		{
//			auto n = fence >= 0 ? min(fence, (int)als.size()) : max(0, (int)als.size() + fence);
//
//			if (column == 0)
//			{
//				set_content_size(Vec2f(0.f));
//				judge_width(element->padding.xz().sum());
//				judge_height(element->padding.yw().sum());
//				for (auto i = 0; i < n; i++)
//				{
//					auto& al = als[i];
//					auto element = al.first;
//					auto aligner = al.second;
//
//					element->set_x(scroll_offset.x() + element->padding.x(), this);
//					element->set_y(scroll_offset.y() + element->padding.y(), this);
//				}
//				for (auto i = n; i < als.size(); i++)
//				{
//					apply_basic_h(als[i].first, als[i].second, true);
//					apply_basic_v(als[i].first, als[i].second, true);
//				}
//			}
//			else
//			{
//				auto w = 0.f;
//				auto h = 0.f;
//				auto lw = 0.f;
//				auto lh = 0.f;
//				for (auto i = 0; i < n; i++)
//				{
//					auto& al = als[i];
//					auto element = al.first;
//					auto aligner = al.second;
//
//					lw += element->size.x() + gap;
//					lh = max(element->size.y(), lh);
//
//					if ((i + 1) % column == 0)
//					{
//						w = max(lw - gap, w);
//						h += lh + gap;
//						lw = 0.f;
//						lh = 0.f;
//					}
//				}
//				if (fence != 0 && !als.empty())
//				{
//					if (n % column != 0)
//					{
//						w = max(lw - gap, w);
//						h += lh + gap;
//					}
//					h -= gap;
//				}
//				set_content_size(Vec2f(w, h));
//				w += element->padding.xz().sum();
//				h += element->padding.yw().sum();
//				judge_width(w);
//				judge_height(h);
//
//				auto x = element->padding[0];
//				auto y = element->padding[1];
//				lh = 0.f;
//				for (auto i = 0; i < n; i++)
//				{
//					auto& al = als[i];
//					auto element = al.first;
//					auto aligner = al.second;
//
//					element->set_x(scroll_offset.x() + x, this);
//					element->set_y(scroll_offset.y() + y, this);
//
//					x += element->size.x() + gap;
//					lh = max(element->size.y(), lh);
//
//					if ((i + 1) % column == 0)
//					{
//						x = element->padding[0];
//						y += lh + gap;
//						lh = 0.f;
//					}
//				}
//				for (auto i = n; i < als.size(); i++)
//				{
//					apply_basic_h(als[i].first, als[i].second, true);
//					apply_basic_v(als[i].first, als[i].second, true);
//				}
//			}
		}
			break;
		}
//
		updating = false;
	}
//
//	void cLayout::set_scrollx(float x)
//	{
//		if (scroll_offset.x() == x)
//			return;
//		scroll_offset.x() = x;
//		auto thiz = (cLayoutPrivate*)this;
//		if (thiz->management)
//			thiz->management->add_to_update_list(thiz);
//	}
//
//	void cLayout::set_scrolly(float y)
//	{
//		if (scroll_offset.y() == y)
//			return;
//		scroll_offset.y() = y;
//		auto thiz = (cLayoutPrivate*)this;
//		if (thiz->management)
//			thiz->management->add_to_update_list(thiz);
//	}
//
//	void cLayout::set_column(uint c)
//	{
//		if (column == c)
//			return;
//		column = c;
//		auto thiz = (cLayoutPrivate*)this;
//		if (thiz->management)
//			thiz->management->add_to_update_list(thiz);
//	}

	void cLayoutPrivate::set_type(LayoutType t)
	{
		if (type == t)
			return;
		type = t;
		mark_layout_dirty();
		((EntityPrivate*)entity)->report_data_changed(this, S<ch("type")>::v);
	}

	void cLayoutPrivate::set_gap(float g)
	{
		if (gap == g)
			return;
		gap = g;
		mark_layout_dirty();
		((EntityPrivate*)entity)->report_data_changed(this, S<ch("gap")>::v);
	}

	void cLayoutPrivate::set_auto_width(bool a)
	{
		if (auto_width == a)
			return;
		auto_width = a;
		mark_layout_dirty();
		((EntityPrivate*)entity)->report_data_changed(this, S<ch("auto_width")>::v);
	}

	void cLayoutPrivate::set_auto_height(bool a)
	{
		if (auto_height == a)
			return;
		auto_height = a;
		mark_layout_dirty();
		((EntityPrivate*)entity)->report_data_changed(this, S<ch("auto_height")>::v);
	}

	void cLayoutPrivate::mark_layout_dirty()
	{
		if (type_setting)
			type_setting->add_to_layouting_list(this);
	}

	void cLayoutPrivate::on_added()
	{
		element = (cElementPrivate*)((EntityPrivate*)entity)->get_component(cElement::type_hash);
		aligner = (cAlignerPrivate*)((EntityPrivate*)entity)->get_component(cAligner::type_hash);
	}

	void cLayoutPrivate::on_entered_world()
	{
		type_setting = (sTypeSettingPrivate*)((WorldPrivate*)((EntityPrivate*)entity)->world)->get_system(sTypeSetting::type_hash);
		type_setting->add_to_layouting_list(this);
	}

	void cLayoutPrivate::on_left_world()
	{
		if (type_setting)
			type_setting->remove_from_layouting_list(this);
		type_setting = nullptr;
	}

	void cLayoutPrivate::on_entity_component_removed(Component* c)
	{
		if (c->type_hash == cAligner::type_hash)
			aligner = nullptr;
	}

	void cLayoutPrivate::on_entity_component_added(Component* c)
	{
		if (c->type_hash == cAligner::type_hash)
		{
			aligner = (cAlignerPrivate*)c;
			mark_layout_dirty();
		}
	}

	void cLayoutPrivate::on_entity_component_data_changed(Component* c, uint data_name_hash)
	{
		if (c == element)
		{
			switch (data_name_hash)
			{
			case S<ch("width")>::v:
			case S<ch("height")>::v:
			case S<ch("padding")>::v:
				mark_layout_dirty();
				break;
			}
		}
	}

	void cLayoutPrivate::on_entity_child_visibility_changed(Entity* e)
	{
		mark_layout_dirty();
	}

	void cLayoutPrivate::on_entity_child_position_changed(Entity* e)
	{
		mark_layout_dirty();
	}

	void cLayoutPrivate::on_entity_child_component_added(Component* c)
	{
		if (c->type_hash == cElement::type_hash || c->type_hash == cAligner::type_hash)
			mark_layout_dirty();
	}

	void cLayoutPrivate::on_entity_child_component_removed(Component* c)
	{
		if (c->type_hash == cElement::type_hash || c->type_hash == cAligner::type_hash)
			mark_layout_dirty();
	}

	void cLayoutPrivate::on_entity_child_component_data_changed(Component* c, uint data_name_hash)
	{
		if (c->type_hash == cElement::type_hash)
		{
			switch (data_name_hash)
			{
			case S<ch("x")>::v:
			case S<ch("y")>::v:
			case S<ch("width")>::v:
			case S<ch("height")>::v:
			case S<ch("padding")>::v:
				mark_layout_dirty();
				break;
			}
		}
		else if (c->type_hash == cAligner::type_hash)
		{
			switch (data_name_hash)
			{
			case S<ch("alignx")>::v:
			case S<ch("aligny")>::v:
			case S<ch("width_factor")>::v:
			case S<ch("height_factor")>::v:
			case S<ch("margin")>::v:
			case S<ch("only_basic")>::v:
				mark_layout_dirty();
				break;
			}
		}
	}

	cLayout* cLayout::create()
	{
		return f_new<cLayoutPrivate>();
	}
}
