//#include <flame/universe/world.h>
//#include <flame/universe/components/element.h>
//#include <flame/universe/components/aligner.h>
#include "../world_private.h"
#include "element_private.h"
#include "aligner_private.h"
#include "../systems/type_setting_private.h"
#include "layout_private.h"
//
//#include <limits>

namespace flame
{
//	cLayoutPrivate::cLayoutPrivate(LayoutType _type)
//	{
//		column = 0;
//		item_padding = 0.f;
//		width_fit_children = true;
//		height_fit_children = true;
//		fence = std::numeric_limits<int>::max();
//
//		scroll_offset = Vec2f(0.f);
//		content_size = Vec2f(0.f);
//	}
//
//	void cLayoutPrivate::set_content_size(const Vec2f& s)
//	{
//		if (s == content_size)
//			return;
//		content_size = s;
//		data_changed(FLAME_CHASH("content_size"), this);
//	}

	void cLayoutPrivate::apply_h_free_layout(cElementPrivate* e, cAlignerPrivate* a, bool free)
	{
		//auto x_flags = a ? a->x_align_flags : (free ? (AlignFlags)0 : AlignMin);
		auto x_flags = (AlignFlags)0;
		//auto p = (x_flags & AlignAbsolute) ? Vec2f(0.f) : element->padding.xz() + (a ? a->margin.xz() : Vec2f(0.f));
		auto p = Vec2f(0.f);
		if (x_flags & AlignMinMax)
		{
			//auto w = element->size.x() - p.xy().sum();
			//if (!(x_flags & AlignGreedy) || w >= a->min_width)
			//	e->set_width(w);
			//e->set_x(p.x());
		}
		if (x_flags & AlignMin)
			e->set_x(p.x());
		//else if (x_flags & AlignMax)
		//	e->set_x(element->size.x() - p.y() - e->width);
		//else if (x_flags & AlignMiddle)
		//	e->set_x(p.x() + (element->size.x() - p.xy().sum() - e->width) * 0.5f);
	}

	void cLayoutPrivate::apply_v_free_layout(cElementPrivate* e, cAlignerPrivate* a, bool free)
	{
		//auto y_flags = a ? a->y_align_flags : (free ? (AlignFlags)0 : AlignMin);
		auto y_flags = (AlignFlags)0;
		//auto p = (y_flags & AlignAbsolute) ? Vec2f(0.f) : element->padding.yw() + (a ? a->margin.yw() : Vec2f(0.f));
		auto p = Vec2f(0.f);
		if (y_flags & AlignMinMax)
		{
			//auto h = element->size.y() - p.xy().sum();
			//if (!(y_flags & AlignGreedy) || h >= a->min_height)
			//	e->set_height(h);
			//e->set_y(p.x());
		}
		if (y_flags & AlignMin)
			e->set_y(p.x());
		//else if (y_flags & AlignMax)
		//	e->set_y(element->size.y() - p.y() - e->height);
		//else if (y_flags & AlignMiddle)
		//	e->set_y(p.x() + (element->size.y() - p.xy().sum() - e->height) * 0.5f);
	}

//	void cLayoutPrivate::use_children_width(float w)
//	{
//		auto x_flags = aligner ? aligner->x_align_flags : (AlignFlags)0;
//		if (!(x_flags & AlignMinMax))
//			element->set_width(w);
//		else if (x_flags & AlignGreedy)
//		{
//			aligner->set_min_width(w);
//			element->set_width(max(element->size.x(), w));
//		}
//	}
//
//	void cLayoutPrivate::use_children_height(float h)
//	{
//		auto y_flags = aligner ? aligner->y_align_flags : (AlignFlags)0;
//		if (!(y_flags & AlignMinMax))
//			element->set_height(h, this);
//		else if (y_flags & AlignGreedy)
//		{
//			aligner->set_min_height(h);
//			element->set_height(max(element->size.y(), h), this);
//		}
//	}
//
//	void cLayoutPrivate::on_event(EntityEvent e, void* t)
//	{
//		switch (e)
//		{
//		case EntityChildVisibilityChanged:
//		case EntityChildPositionChanged:
//			if (management)
//				management->add_to_update_list(this);
//			break;
//		case EntityChildComponentAdded:
//		case EntityChildComponentRemoved:
//		{
//			auto h = ((Component*)t)->name_hash;
//			if (h == FLAME_CHASH("cElement") ||
//				h == FLAME_CHASH("cAligner"))
//			{
//				if (management)
//					management->add_to_update_list(this);
//			}
//		}
//			break;
//		}
//	}
//
//	void cLayoutPrivate::on_sibling_data_changed(Component* t, uint hash, void* sender)
//	{
//		if (t == element)
//		{
//			switch (hash)
//			{
//			case FLAME_CHASH("scale"):
//			case FLAME_CHASH("size"):
//			case FLAME_CHASH("inner_padding"):
//				if (management && !updating)
//					management->add_to_update_list(this);
//				break;
//			}
//		}
//	}
//
//	void cLayoutPrivate::on_child_data_changed(Component * t, uint hash, void* sender)
//	{
//		switch (t->name_hash)
//		{
//		case FLAME_CHASH("cElement"):
//			switch (hash)
//			{
//			case FLAME_CHASH("pos"):
//			case FLAME_CHASH("scale"):
//			case FLAME_CHASH("size"):
//				if (management)
//					management->add_to_update_list(this);
//				break;
//			}
//			break;
//		case FLAME_CHASH("cAligner"):
//			if (management)
//				management->add_to_update_list(this);
//			break;
//		}
//	}
//
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

		std::vector<std::pair<cElementPrivate*, cAlignerPrivate*>> als;
		for (auto& c : ((EntityPrivate*)entity)->children)
		{
			if (c->global_visibility)
			{
				auto e = (cElementPrivate*)c->get_component(cElement::type_hash);
				auto a = (cAlignerPrivate*)c->get_component(cAligner::type_hash);
				if (e || a)
					als.emplace_back(e, a);
			}
		}

		switch (type)
		{
		case LayoutFree:
			for (auto& al : als)
			{
				apply_h_free_layout(al.first, al.second, true);
				apply_v_free_layout(al.first, al.second, true);
			}
			break;
		case LayoutHorizontal:
		{
//			auto n = fence >= 0 ? min(fence, (int)als.size()) : max(0, (int)als.size() + fence);
//
//			auto w = 0.f;
//			auto h = 0.f;
//			auto factor = 0.f;
//			for (auto i = 0; i < n; i++)
//			{
//				auto& al = als[i];
//				auto element = al.first;
//				auto aligner = al.second;
//				auto x_flags = aligner ? aligner->x_align_flags : (AlignFlags)0;
//				auto y_flags = aligner ? aligner->y_align_flags : (AlignFlags)0;
//
//				if (x_flags & AlignMinMax)
//				{
//					factor += aligner->width_factor;
//					if (x_flags & AlignGreedy)
//						w += aligner->min_width;
//				}
//				else
//					w += element->size.x();
//				w += item_padding;
//				if (y_flags & AlignMinMax)
//				{
//					if (y_flags & AlignGreedy)
//						h = max(aligner->min_height, h);
//				}
//				else
//					h = max(element->size.y(), h);
//			}
//			if (fence != 0 && !als.empty())
//				w -= item_padding;
//			set_content_size(Vec2f(w, h));
//			w += element->padding.xz().sum();
//			h += element->padding.yw().sum();
//			if (width_fit_children)
//				use_children_width(w);
//			if (height_fit_children)
//				use_children_height(h);
//
//			w = element->size.x() - w;
//			if (w > 0.f && factor > 0)
//				w /= factor;
//			else
//				w = 0.f;
//			auto x = element->padding[0];
//			for (auto i = 0; i < n; i++)
//			{
//				auto& al = als[i];
//				auto element = al.first;
//				auto aligner = al.second;
//				auto x_flags = aligner ? aligner->x_align_flags : (AlignFlags)0;
//
//				if (x_flags & AlignMinMax)
//				{
//					auto _w = w * aligner->width_factor;
//					if (x_flags & AlignGreedy)
//						_w += aligner->min_width;
//					element->set_width(_w);
//				}
//				element->set_x(scroll_offset.x() + x, this);
//				x += element->size.x() + item_padding;
//			}
//			for (auto i = 0; i < n; i++)
//				apply_v_free_layout(als[i].first, als[i].second, false);
//			for (auto i = n; i < als.size(); i++)
//			{
//				apply_h_free_layout(als[i].first, als[i].second, true);
//				apply_v_free_layout(als[i].first, als[i].second, true);
//			}
		}
			break;
		case LayoutVertical:
		{
//			auto n = fence >= 0 ? min(fence, (int)als.size()) : max(0, (int)als.size() + fence);
			auto n = (int)als.size();
//
			auto w = 0.f;
			auto h = 0.f;
			auto factor = 0.f;
//			for (auto i = 0; i < n; i++)
//			{
//				auto& al = als[i];
//				auto element = al.first;
//				auto aligner = al.second;
//				auto x_flags = aligner ? aligner->x_align_flags : (AlignFlags)0;
//				auto y_flags = aligner ? aligner->y_align_flags : (AlignFlags)0;
//
//				if (x_flags & AlignMinMax)
//				{
//					if (x_flags & AlignGreedy)
//						w = max(aligner->min_width, w);
//				}
//				else
//					w = max(element->size.x() + (aligner ? aligner->margin.xz().sum() : 0.f), w);
//				if (y_flags & AlignMinMax)
//				{
//					factor += aligner->height_factor;
//					if (y_flags & AlignGreedy)
//						h += aligner->min_height;
//				}
//				else
//					h += element->size.y();
//				h += item_padding;
//			}
//			if (fence != 0 && !als.empty())
//				h -= item_padding;
//			set_content_size(Vec2f(w, h));
//			w += element->padding.xz().sum();
//			h += element->padding.yw().sum();
//			if (width_fit_children)
//				use_children_width(w);
//			if (height_fit_children)
//				use_children_height(h);
//
//			for (auto i = 0; i < n; i++)
//				apply_h_free_layout(als[i].first, als[i].second, false);
//			h = element->size.y() - h;
//			if (h > 0.f && factor > 0)
//				h /= factor;
//			else
//				h = 0.f;
//			auto y = element->padding[1];
			auto y = 0;
			for (auto i = 0; i < n; i++)
			{
				auto& al = als[i];
				auto element = al.first;
				auto aligner = al.second;
//				auto y_flags = aligner ? aligner->y_align_flags : (AlignFlags)0;
				auto y_flags = (AlignFlags)0;
//
//				if (y_flags & AlignMinMax)
//				{
//					auto _h = h * aligner->height_factor;
//					if (y_flags & AlignGreedy)
//						_h += aligner->min_height;
//					element->set_height(_h, this);
//				}
//				element->set_y(scroll_offset.y() + y, this);
				element->set_y(y);
//				y += element->size.y() + item_padding;
				y += element->height;
			}
//			for (auto i = n; i < als.size(); i++)
//			{
//				apply_h_free_layout(als[i].first, als[i].second, true);
//				apply_v_free_layout(als[i].first, als[i].second, true);
//			}
		}
			break;
		case LayoutGrid:
		{
//			auto n = fence >= 0 ? min(fence, (int)als.size()) : max(0, (int)als.size() + fence);
//
//			if (column == 0)
//			{
//				set_content_size(Vec2f(0.f));
//				if (width_fit_children)
//					use_children_width(element->padding.xz().sum());
//				if (height_fit_children)
//					use_children_height(element->padding.yw().sum());
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
//					apply_h_free_layout(als[i].first, als[i].second, true);
//					apply_v_free_layout(als[i].first, als[i].second, true);
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
//					lw += element->size.x() + item_padding;
//					lh = max(element->size.y(), lh);
//
//					if ((i + 1) % column == 0)
//					{
//						w = max(lw - item_padding, w);
//						h += lh + item_padding;
//						lw = 0.f;
//						lh = 0.f;
//					}
//				}
//				if (fence != 0 && !als.empty())
//				{
//					if (n % column != 0)
//					{
//						w = max(lw - item_padding, w);
//						h += lh + item_padding;
//					}
//					h -= item_padding;
//				}
//				set_content_size(Vec2f(w, h));
//				w += element->padding.xz().sum();
//				h += element->padding.yw().sum();
//				if (width_fit_children)
//					use_children_width(w);
//				if (height_fit_children)
//					use_children_height(h);
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
//					x += element->size.x() + item_padding;
//					lh = max(element->size.y(), lh);
//
//					if ((i + 1) % column == 0)
//					{
//						x = element->padding[0];
//						y += lh + item_padding;
//						lh = 0.f;
//					}
//				}
//				for (auto i = n; i < als.size(); i++)
//				{
//					apply_h_free_layout(als[i].first, als[i].second, true);
//					apply_v_free_layout(als[i].first, als[i].second, true);
//				}
//			}
		}
			break;
		}
//
		updating = false;
	}
//
//	void cLayout::set_x_scroll_offset(float x)
//	{
//		if (scroll_offset.x() == x)
//			return;
//		scroll_offset.x() = x;
//		auto thiz = (cLayoutPrivate*)this;
//		if (thiz->management)
//			thiz->management->add_to_update_list(thiz);
//	}
//
//	void cLayout::set_y_scroll_offset(float y)
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
		mark_dirty();
	}

	void cLayoutPrivate::mark_dirty()
	{
		if (type_setting)
			type_setting->add_to_layouting_list(this);
	}

	void cLayoutPrivate::on_added()
	{

	}

	void cLayoutPrivate::on_removed()
	{

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

	cLayoutPrivate* cLayoutPrivate::create()
	{
		auto ret = _allocate(sizeof(cLayoutPrivate));
		new (ret) cLayoutPrivate;
		return (cLayoutPrivate*)ret;
	}

	cLayout* cLayout::create() { return cLayoutPrivate::create(); }
}
