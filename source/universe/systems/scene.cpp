#include "../../foundation/window.h"
#include "../world_private.h"
#include "../components/element_private.h"
#include "../components/node_private.h"
#include "scene_private.h"

namespace flame
{
	void sScenePrivate::setup(NativeWindow* _window)
	{
		assert(!window);
		window = _window;
	}

	void sScenePrivate::add_to_sizing(cElementPrivate* e)
	{
		auto depth = e->entity->depth;
		auto it = sizing_list.begin();
		for (; it != sizing_list.end(); it++)
		{
			if (it->first == depth)
			{
				it->second.push_back(e);
				return;
			}
			if (it->first < depth)
				break;
		}
		std::pair<uint, std::deque<cElementPrivate*>> v;
		v.first = depth;
		v.second.push_back(e);
		sizing_list.insert(it, v);
	}

	void sScenePrivate::remove_from_sizing(cElementPrivate* e)
	{
		auto depth = e->entity->depth;
		for (auto& v : sizing_list)
		{
			if (v.first == depth)
			{
				std::erase_if(v.second, [&](const auto& i) {
					return i == e;
				});
				break;
			}
		}
	}
	void sScenePrivate::add_to_layout(cElementPrivate* e)
	{
		auto depth = e->entity->depth;
		auto it = layout_list.begin();
		for (; it != layout_list.end(); it++)
		{
			if (it->first == depth)
			{
				it->second.push_back(e);
				return;
			}
			if (it->first > depth)
				break;
		}
		std::pair<uint, std::deque<cElementPrivate*>> v;
		v.first = depth;
		v.second.push_back(e);
		layout_list.insert(it, v);
	}

	void sScenePrivate::remove_from_layout(cElementPrivate* e)
	{
		auto depth = e->entity->depth;
		for (auto& v : layout_list)
		{
			if (v.first == depth)
			{
				std::erase_if(v.second, [&](const auto& i) {
					return i == e;
				});
				break;
			}
		}
	}

	void sScenePrivate::add_to_update_bounds(cNodePrivate* n)
	{
		auto depth = n->entity->depth;
		auto it = update_bounds_list.begin();
		for (; it != update_bounds_list.end(); it++)
		{
			if (it->first == depth)
			{
				it->second.push_back(n);
				return;
			}
			if (it->first < depth)
				break;
		}
		std::pair<uint, std::deque<cNodePrivate*>> v;
		v.first = depth;
		v.second.push_back(n);
		update_bounds_list.insert(it, v);
	}

	void sScenePrivate::remove_from_update_bounds(cNodePrivate* n)
	{
		auto depth = n->entity->depth;
		for (auto& v : update_bounds_list)
		{
			if (v.first == depth)
			{
				std::erase_if(v.second, [&](const auto& i) {
					return i == n;
				});
				break;
			}
		}
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
			target->set_x(e->scroll.x + p[0] + floor((e->size.x - p[0] - p[1] - target->size.x) * 0.5f));
			break;
		case AlignMinMax:
			target->set_width(e->size.x - p[0] - p[1]);
			target->set_x(e->scroll.x + p[0]);
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
			target->set_y(e->scroll.y + p[0] + floor((e->size.y - p[0] - p[1] - target->size.y) * 0.5f));
			break;
		case AlignMinMax:
			target->set_height(e->size.y - p[0] - p[1]);
			target->set_y(e->scroll.y + p[0]);
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

	void sScenePrivate::update()
	{
		if (window)
		{
			auto element = world->root->get_component_i<cElementPrivate>(0);
			if (element)
				element->set_size(window->size);
		}

		while (!sizing_list.empty())
		{
			auto& v = sizing_list.front();
			if (v.second.empty())
			{
				sizing_list.pop_front();
				continue;
			}

			auto e = v.second.front();
			v.second.pop_front();

			auto size = vec2(-1.f);
			for (auto m : e->measurers.list)
			{
				vec2 s;
				if (m(&s))
				{
					size.x = max(size.x, s.x);
					size.y = max(size.y, s.y);
				}
			}

			if (size.x >= 0.f)
			{
				size.x += e->padding_size[0];
			}
			if (size.y >= 0.f)
			{
				size.y += e->padding_size[1];
			}
			if (any(greaterThan(size, vec2(0.f))))
			{
				auto s = e->size;
				if (e->auto_width)
					s.x = size.x;
				if (e->auto_height)
					s.y = size.y;
				e->set_size(s);
			}

			e->desired_size = size;
			e->pending_sizing = false;
		}

		while (!layout_list.empty())
		{
			auto& v = layout_list.front();
			if (v.second.empty())
			{
				layout_list.pop_front();
				continue;
			}

			auto l = v.second.front();
			v.second.pop_front();

			std::vector<cElementPrivate*> als[2];
			for (auto& c : l->entity->children)
			{
				if (c->global_visibility)
				{
					auto e = c->get_component_i<cElementPrivate>(0);
					if (e)
					{
						if (!e->align_in_layout)
							als[1].push_back(e);
						else
							als[0].push_back(e);
					}
				}
			}

			switch (l->layout_type)
			{
			case LayoutFree:
				for (auto i = 0; i < 2; i++)
				{
					for (auto al : als[i])
					{
						al->layer_policy = 1;
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
					al->layer_policy = 0;
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
					al->layer_policy = 1;
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
					al->layer_policy = 0;
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
					al->layer_policy = 1;
					apply_basic_h(l, al, false);
					apply_basic_v(l, al, false);
				}
			}
				break;
			case LayoutTile:
			{
				auto w = 0.f;
				auto h = 0.f;
				auto lw = 0.f;
				auto lh = 0.f;

				if (/*auto column*/true)
				{
					if (als[0].size() > 0)
						l->layout_columns = (l->size.x - l->padding_size[0]) / (als[0][0]->size.x + l->layout_gap);
					l->layout_columns = max(1U, l->layout_columns);
				}

				for (auto i = 0; i < als[0].size(); i++)
				{
					auto& al = als[0][i];
					al->layer_policy = 0;

					lw += al->size.x + l->layout_gap;
					lh = max(al->size.y, lh);

					if ((i + 1) % l->layout_columns == 0)
					{
						w = max(lw - l->layout_gap, w);
						h += lh + l->layout_gap;
						lw = 0.f;
						lh = 0.f;
					}
				}
				if (!als[0].empty())
				{
					if (als[0].size() % l->layout_columns != 0)
					{
						w = max(lw - l->layout_gap, w);
						h += lh + l->layout_gap;
					}
					h -= l->layout_gap;
				}
				w += l->padding_size[0];
				h += l->padding_size[1];
				judge_width(l, w);
				judge_height(l, h);

				auto x = l->padding[0];
				auto y = l->padding[1];
				lh = 0.f;
				for (auto i = 0; i < als[0].size(); i++)
				{
					auto& al = als[0][i];

					al->set_x(l->scroll.x + x);
					al->set_y(l->scroll.y + y);

					x += al->size.x + l->layout_gap;
					lh = max(al->size.y, lh);

					if ((i + 1) % l->layout_columns == 0)
					{
						x = l->padding[0];
						y += lh + l->layout_gap;
						lh = 0.f;
					}
				}
				for (auto al : als[1])
				{
					al->layer_policy = 1;
					apply_basic_h(l, al, false);
					apply_basic_v(l, al, false);
				}
			}
				break;
			}
			
			l->pending_layout = false;
		}

		while (!update_bounds_list.empty())
		{
			auto& v = update_bounds_list.front();
			if (v.second.empty())
			{
				update_bounds_list.pop_front();
				continue;
			}

			auto n = v.second.front();
			v.second.pop_front();

			n->update_transform();

			n->bounds.reset();
			for (auto m : n->measurers.list)
			{
				AABB b;
				b.reset();
				if (m(&b) && !b.invalid())
					n->bounds.expand(b);
			}

			if (!n->assemble_sub)
			{
				for (auto& c : n->entity->children)
				{
					auto node = c->get_component_i<cNodePrivate>(0);
					if (node && !n->bounds.invalid())
						n->bounds.expand(node->bounds);
				}
			}

			n->data_changed(S<"bounds"_h>);

			if (n->octnode.first)
			{
				if (!n->octnode.second)
					n->octnode.first->add(n);
				else
				{
					n->octnode.second->remove(n);
					n->octnode.first->add(n);
				}
			}

			n->pending_update_bounds = false;
		}
	}

	sScene* sScene::create()
	{
		return new sScenePrivate();
	}
}
