#include "../world_private.h"
#include "../components/element_private.h"
#include "layout_private.h"

namespace flame
{
	void sLayoutPrivate::on_added()
	{
		window = (Window*)world->find_object("flame::Window");
	}

	static void apply_basic_h(cElementPrivate* e, cElementPrivate* target, bool force_align)
	{
		auto alignx = target->alignx;
	 	if (force_align && alignx == AlignNone)
	 	   alignx = AlignMin;
		auto p = (target->align_absolute ? vec2(0.f) : e->padding.xz()) + target->margin.xz();
		switch (alignx)
		{
		case AlignMin:
			target->set_x(e->scroll.x + p[0]);
			break;
		case AlignMax:
			target->set_x(e->scroll.x + e->size.x - p[1] - target->size.x);
			break;
		case AlignMiddle:
			target->set_x(e->scroll.x + p[0] + (e->size.x - p[0] - p[1] - target->size.x) * 0.5f);
			break;
		case AlignMinMax:
			target->set_width(e->size.x - p[0] - p[1]);
			target->set_x(e->scroll.x + p[0]);
			target->desired_size.x = target->size.x;
			break;
		}
	}

	static void apply_basic_v(cElementPrivate* e, cElementPrivate* target, bool force_align)
	{
		auto aligny = target->aligny;
	 	if (force_align && aligny == AlignNone)
			aligny = AlignMin;
		auto p = (target->align_absolute ? vec2(0.f) : e->padding.yw()) + target->margin.yw();
		switch (aligny)
		{
		case AlignMin:
			target->set_y(e->scroll.y + p[0]);
			break;
		case AlignMax:
			target->set_y(e->scroll.y + e->size.y - p[1] - target->size.y);
			break;
		case AlignMiddle:
			target->set_y(e->scroll.y + p[0] + (e->size.y - p[0] - p[1] - target->size.y) * 0.5f);
			break;
		case AlignMinMax:
			target->set_height(e->size.y - p[0] - p[1]);
			target->set_y(e->scroll.y + p[0]);
			target->desired_size.y = target->size.y;
			break;
		}
	}

	static void judge_width(cElementPrivate* e, float w)
	{
		if (e->auto_width && e->alignx != AlignMinMax)
		{
			e->set_width(w);
			e->desired_size.x = w;
		}
		else
			e->desired_size.x = 0.f;
	}

	static void judge_height(cElementPrivate* e, float h)
	{
		if (e->auto_height && e->aligny != AlignMinMax)
		{
			e->set_height(h);
			e->desired_size.y = h;
		}
		else
			e->desired_size.y = 0.f;
	}

	void sLayoutPrivate::update()
	{
		if (window)
		{
			auto element = world->root->get_component_t<cElementPrivate>();
			auto size = window->get_size();
			element->set_width(size.x);
			element->set_height(size.y);
		}

		while (!sizing_list.empty())
		{
			auto e = sizing_list.front();
			auto size = vec2(-1.f);
			for (auto& m : e->measurables)
			{
				vec2 r;
				m.second(m.first, r);
				if (e->auto_width)
					size.x = max(size.x, r.x);
				if (e->auto_height)
					size.y = max(size.y, r.y);
			}

			auto w = 0.f, h = 0.f;
			if (size.x >= 0.f)
			{
				w = size.x + e->padding_size[0];
				e->set_width(w);
			}
			if (size.y >= 0.f)
			{
				h = size.y + e->padding_size[1];
				e->set_height(h);
			}

			e->desired_size = vec2(w, h);
			e->pending_sizing = false;
			sizing_list.pop_front();
		}

		while (!layout_list.empty())
		{
			auto l = layout_list.front();
			std::vector<cElementPrivate*> als[2];
			for (auto& c : l->entity->children)
			{
				if (c->global_visibility)
				{
					auto e = c->get_component_t<cElementPrivate>();
					if (!e->align_in_layout)
						als[1].push_back(e);
					else
						als[0].push_back(e);
				}
			}

			switch (l->layout_type)
			{
			case LayoutFree:
				for (auto i = 0; i < 2; i++)
				{
					for (auto al : als[i])
					{
						apply_basic_h(l, al, false);
						apply_basic_v(l, al, false);
					}
				}
				break;
			case LayoutHorizontal:
			{
				auto w = 0.f;
				auto h = 0.f;
				auto factor = 0.f;
				for (auto al : als[0])
				{
					if (al->alignx == AlignMinMax)
					{
						factor += al->width_factor;
						w += al->desired_size.x;
					}
					else
						w += al->size.x;
					w += l->layout_gap + al->margin_size[0];
					h = max((al->aligny == AlignMinMax ? al->desired_size.y : al->size.y) + al->margin_size[1], h);

				}
				if (!als[0].empty())
					w -= l->layout_gap;
				w += l->padding_size[0];
				h += l->padding_size[1];
				judge_width(l, w);
				judge_height(l, h);

				w = l->size.x - w;
				if (w > 0.f && factor > 0.f)
					w /= factor;
				else
					w = 0.f;
				auto x = l->padding[0];
				for (auto al : als[0])
				{
					if (al->alignx == AlignMinMax)
						al->set_width(w * al->width_factor + al->desired_size.x);
					x += al->margin.x;
					al->set_x(l->scroll.x + x);
					x += al->size.x + l->layout_gap + al->margin.z;
				}
				for (auto al : als[0])
					apply_basic_v(l, al, true);
				for (auto al : als[1])
				{
					apply_basic_h(l, al, false);
					apply_basic_v(l, al, false);
				}
			}
				break;
			case LayoutVertical:
			{
				auto w = 0.f;
				auto h = 0.f;
				auto factor = 0.f;
				for (auto al : als[0])
				{
					w = max((al->alignx == AlignMinMax ? al->desired_size.x : al->size.x) + al->margin_size[0], w);
					if (al->aligny == AlignMinMax)
					{
						factor += al->height_factor;
						h += al->desired_size.y;
					}
					else
						h += al->size.y;
					h += l->layout_gap + al->margin_size[1];
				}
				if (!als[0].empty())
					h -= l->layout_gap;
				w += l->padding_size[0];
				h += l->padding_size[1];
				judge_width(l, w);
				judge_height(l, h);

				for (auto al : als[0])
					apply_basic_h(l, al, true);
				h = l->size.y - h;
				if (h > 0.f && factor > 0.f)
					h /= factor;
				else
					h = 0.f;
				auto y = l->padding[1];
				for (auto al : als[0])
				{
					if (al->aligny == AlignMinMax)
						al->set_height(h * al->height_factor + al->desired_size.y);
					y += al->margin.y;
					al->set_y(l->scroll.y + y);
					y += al->size.y + l->layout_gap + al->margin.w;
				}
				for (auto al : als[1])
				{
					apply_basic_h(l, al, false);
					apply_basic_v(l, al, false);
				}
			}
				break;
			case LayoutTile:
			{
				//			auto n = fence >= 0 ? min(fence, (int)als.size()) : max(0, (int)als.size() + fence);
				//
				//			if (column == 0)
				//			{
				//				judge_width(l, l->padding_size[0]);
				//				judge_height(l, l->padding_size[1]);
				//				for (auto i = 0; i < n; i++)
				//				{
				//					auto& al = als[i];
				//					auto element = al.first;
				//					auto aligner = al.second;
				//
				//					l->set_x(scroll_offset.x + l->padding.x, this);
				//					l->set_y(scroll_offset.y + l->padding.y, this);
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
				//					lw += l->size.x + l->layout_gap;
				//					lh = max(l->size.y, lh);
				//
				//					if ((i + 1) % column == 0)
				//					{
				//						w = max(lw - l->layout_gap, w);
				//						h += lh + l->layout_gap;
				//						lw = 0.f;
				//						lh = 0.f;
				//					}
				//				}
				//				if (fence != 0 && !als.empty())
				//				{
				//					if (n % column != 0)
				//					{
				//						w = max(lw - l->layout_gap, w);
				//						h += lh + l->layout_gap;
				//					}
				//					h -= l->layout_gap;
				//				}
				//				w += l->padding_size[0];
				//				h += l->padding_size[1];
				//				judge_width(l, w);
				//				judge_height(l, h);
				//
				//				auto x = l->padding[0];
				//				auto y = l->padding[1];
				//				lh = 0.f;
				//				for (auto i = 0; i < n; i++)
				//				{
				//					auto& al = als[i];
				//					auto element = al.first;
				//					auto aligner = al.second;
				//
				//					l->set_x(scroll_offset.x + x, this);
				//					l->set_y(scroll_offset.y + y, this);
				//
				//					x += l->size.x + l->layout_gap;
				//					lh = max(l->size.y, lh);
				//
				//					if ((i + 1) % column == 0)
				//					{
				//						x = l->padding[0];
				//						y += lh + l->layout_gap;
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
			l->pending_layout = false; 
			layout_list.pop_front();
		}
	}

	sLayout* sLayout::create()
	{
		return f_new<sLayoutPrivate>();
	}
}
