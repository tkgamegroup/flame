#include <flame/graphics/font.h>
#include <flame/universe/world.h>
#include <flame/universe/systems/layout_management.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/aligner.h>
#include "layout_private.h"

#include <limits>

namespace flame
{
	cLayoutPrivate::cLayoutPrivate(LayoutType _type)
	{
		element = nullptr;
		aligner = nullptr;

		type = _type;
		column = 0;
		item_padding = 0.f;
		width_fit_children = true;
		height_fit_children = true;
		fence = std::numeric_limits<int>::max();
		scroll_offset = Vec2f(0.f);

		content_size = Vec2f(0.f);

		management = nullptr;
		pending_update = false;

		element_data_listener = nullptr;

		als_dirty = true;
	}

	cLayoutPrivate::~cLayoutPrivate()
	{
		if (management)
			management->remove_from_update_list(this);
		if (!entity->dying_)
			element->data_changed_listeners.remove(element_data_listener);
	}

	void cLayoutPrivate::set_content_size(const Vec2f& s)
	{
		if (s == content_size)
			return;
		content_size = s;
		data_changed(FLAME_CHASH("content_size"), this);
	}

	void cLayoutPrivate::apply_h_free_layout(const Aligner& a, bool free)
	{
		auto x_flags = a.aligner ? a.aligner->x_align_flags : (free ? (AlignFlag)0 : AlignMin);
		auto p = (x_flags & AlignAbsolute) ? Vec2f(0.f) : element->padding.xz();
		if (x_flags & AlignMinMax)
		{
			auto w = element->size.x() - p.xy().sum();
			if (!(x_flags & AlignGreedy) || w >= a.aligner->min_width)
				a.element->set_width(w, this);
			a.element->set_x(p.x(), this);
		}
		if (x_flags & AlignMin)
			a.element->set_x(p.x(), this);
		else if (x_flags & AlignMax)
			a.element->set_x(element->size.x() - p.y() - a.element->size.x(), this);
		else if (x_flags & AlignMiddle)
			a.element->set_x(p.x() + (element->size.x() - p.xy().sum() - a.element->size.x()) * 0.5f, this);
	}

	void cLayoutPrivate::apply_v_free_layout(const Aligner& a, bool free)
	{
		auto y_flags = a.aligner ? a.aligner->y_align_flags : (free ? (AlignFlag)0 : AlignMin);
		auto p = (y_flags & AlignAbsolute) ? Vec2f(0.f) : element->padding.yw();
		if (y_flags & AlignMinMax)
		{
			auto h = element->size.y() - p.xy().sum();
			if (!(y_flags & AlignGreedy) || h >= a.aligner->min_height)
				a.element->set_height(h, this);
			a.element->set_y(p.x(), this);
		}
		if (y_flags & AlignMin)
			a.element->set_y(p.x(), this);
		else if (y_flags & AlignMax)
			a.element->set_y(element->size.y() - p.y() - a.element->size.y(), this);
		else if (y_flags & AlignMiddle)
			a.element->set_y(p.x() + (element->size.y() - p.xy().sum() - a.element->size.y()) * 0.5f, this);
	}

	void cLayoutPrivate::use_children_width(float w)
	{
		auto x_flags = aligner ? aligner->x_align_flags : (AlignFlag)0;
		if (!(x_flags & AlignMinMax))
			element->set_width(w, this);
		else if (x_flags & AlignGreedy)
		{
			aligner->set_min_width(w);
			element->set_width(max(element->size.x(), w), this);
		}
	}

	void cLayoutPrivate::use_children_height(float h)
	{
		auto y_flags = aligner ? aligner->y_align_flags : (AlignFlag)0;
		if (!(y_flags & AlignMinMax))
			element->set_height(h, this);
		else if (y_flags & AlignGreedy)
		{
			aligner->set_min_height(h);
			element->set_height(max(element->size.y(), h), this);
		}
	}

	void cLayoutPrivate::on_entered_world()
	{
		management = entity->world()->get_system(sLayoutManagement);
		management->add_to_update_list(this);
	}

	void cLayoutPrivate::on_left_world()
	{
		management->remove_from_update_list(this);
		management = nullptr;
	}

	void cLayoutPrivate::on_component_added(Component* c)
	{
		if (c->name_hash == FLAME_CHASH("cElement"))
		{
			element = (cElement*)c;
			element_data_listener = element->data_changed_listeners.add([](void* c, uint hash, void* sender) {
				auto thiz = *(cLayoutPrivate**)c;
				if (sender == thiz)
					return true;
				switch (hash)
				{
				case FLAME_CHASH("scale"):
				case FLAME_CHASH("size"):
				case FLAME_CHASH("inner_padding"):
					if (thiz->management)
						thiz->management->add_to_update_list(thiz);
				}
				return true;
			}, Mail::from_p(this));
		}
		else if (c->name_hash == FLAME_CHASH("cAligner"))
			aligner = (cAligner*)c;
	}

	void cLayoutPrivate::on_visibility_changed()
	{
		if (management)
			management->add_to_update_list(this);
	}

	void cLayoutPrivate::on_child_visibility_changed()
	{
		als_dirty = true;
		if (management)
			management->add_to_update_list(this);
	}

	void cLayoutPrivate::on_child_component_added(Component* c)
	{
		if (c->name_hash == FLAME_CHASH("cElement") ||
			c->name_hash == FLAME_CHASH("cAligner"))
		{
			als_dirty = true;
			if (management)
				management->add_to_update_list(this);
		}
	}

	void cLayoutPrivate::on_child_component_removed(Component* c)
	{
		if (c->name_hash == FLAME_CHASH("cElement") ||
			c->name_hash == FLAME_CHASH("cAligner"))
		{
			als_dirty = true;
			if (management)
				management->add_to_update_list(this);
			for (auto& al : als)
			{
				if (c == al.element)
				{
					al.element->data_changed_listeners.remove(al.element_listener);
					al.element = nullptr;
					return;
				}
				if (c == al.aligner)
				{
					al.aligner->data_changed_listeners.remove(al.aligner_listener);
					al.aligner = nullptr;
					return;
				}
			}
		}
	}

	void cLayoutPrivate::on_child_position_changed()
	{
		als_dirty = true;
		if (management)
			management->add_to_update_list(this);
	}

	void cLayoutPrivate::update()
	{
		if (als_dirty)
		{
			for (auto& al : als)
			{
				if (al.element)
					al.element->data_changed_listeners.remove(al.element_listener);
				if (al.aligner)
					al.aligner->data_changed_listeners.remove(al.aligner_listener);
			}
			als.clear();
			for (auto i = 0; i < entity->child_count(); i++)
			{
				auto e = entity->child(i);
				if (e->global_visibility)
				{
					Aligner a;
					a.element = e->get_component(cElement);
					a.aligner = e->get_component(cAligner);

					if (a.element)
					{
						a.element_listener = a.element->data_changed_listeners.add([](void* c, uint hash, void* sender) {
							auto thiz = *(cLayoutPrivate**)c;
							if (sender == thiz)
								return true;
							switch (hash)
							{
							case FLAME_CHASH("pos"):
							case FLAME_CHASH("scale"):
							case FLAME_CHASH("size"):
								if (thiz->management)
									thiz->management->add_to_update_list(thiz);
								break;
							}
							return true;
						}, Mail::from_p(this));
					}
					if (a.aligner)
					{
						a.aligner_listener = a.aligner->data_changed_listeners.add([](void* c, uint hash, void* sender) {
							auto thiz = *(cLayoutPrivate**)c;
							if (sender == thiz)
								return true;
							if (thiz->management)
								thiz->management->add_to_update_list(thiz);
							return true;
						}, Mail::from_p(this));
					}
					als.push_back(a);
				}
			}
			als_dirty = false;
		}

		switch (type)
		{
		case LayoutFree:
			for (auto i = 0; i < als.size(); i++)
			{
				apply_h_free_layout(als[i], true);
				apply_v_free_layout(als[i], true);
			}
			break;
		case LayoutHorizontal:
		{
			auto n = fence >= 0 ? min(fence, (int)als.size()) : max(0, (int)als.size() + fence);

			auto w = 0.f;
			auto h = 0.f;
			auto factor = 0.f;
			for (auto i = 0; i < n; i++)
			{
				auto& al = als[i];
				auto element = al.element;
				auto aligner = al.aligner;
				auto x_flags = aligner ? aligner->x_align_flags : (AlignFlag)0;
				auto y_flags = aligner ? aligner->y_align_flags : (AlignFlag)0;

				if (x_flags & AlignMinMax)
				{
					factor += aligner->width_factor;
					if (x_flags & AlignGreedy)
						w += aligner->min_width;
				}
				else
					w += element->size.x();
				w += item_padding;
				if (y_flags & AlignMinMax)
				{
					if (y_flags & AlignGreedy)
						h = max(aligner->min_height, h);
				}
				else
					h = max(element->size.y(), h);
			}
			if (fence != 0 && !als.empty())
				w -= item_padding;
			set_content_size(Vec2f(w, h));
			w += element->padding.xz().sum();
			h += element->padding.yw().sum();
			if (width_fit_children)
				use_children_width(w);
			if (height_fit_children)
				use_children_height(h);

			w = element->size.x() - w;
			if (w > 0.f && factor > 0)
				w /= factor;
			else
				w = 0.f;
			auto x = element->padding[0];
			for (auto i = 0; i < n; i++)
			{
				auto& al = als[i];
				auto element = al.element;
				auto aligner = al.aligner;
				auto x_flags = aligner ? aligner->x_align_flags : (AlignFlag)0;

				if (x_flags & AlignMinMax)
				{
					auto _w = w * aligner->width_factor;
					if (x_flags & AlignGreedy)
						_w += aligner->min_width;
					element->set_width(_w, this);
				}
				element->set_x(scroll_offset.x() + x, this);
				x += element->size.x() + item_padding;
			}
			for (auto i = 0; i < n; i++)
				apply_v_free_layout(als[i], false);
			for (auto i = n; i < als.size(); i++)
			{
				apply_h_free_layout(als[i], true);
				apply_v_free_layout(als[i], true);
			}
		}
			break;
		case LayoutVertical:
		{
			auto n = fence >= 0 ? min(fence, (int)als.size()) : max(0, (int)als.size() + fence);

			auto w = 0.f;
			auto h = 0.f;
			auto factor = 0.f;
			for (auto i = 0; i < n; i++)
			{
				auto& al = als[i];
				auto element = al.element;
				auto aligner = al.aligner;
				auto x_flags = aligner ? aligner->x_align_flags : (AlignFlag)0;
				auto y_flags = aligner ? aligner->y_align_flags : (AlignFlag)0;

				if (x_flags & AlignMinMax)
				{
					if (x_flags & AlignGreedy)
						w = max(aligner->min_width, w);
				}
				else
					w = max(element->size.x(), w);
				if (y_flags & AlignMinMax)
				{
					factor += aligner->height_factor;
					if (y_flags & AlignGreedy)
						h += aligner->min_height;
				}
				else
					h += element->size.y();
				h += item_padding;
			}
			if (fence != 0 && !als.empty())
				h -= item_padding;
			set_content_size(Vec2f(w, h));
			w += element->padding.xz().sum();
			h += element->padding.yw().sum();
			if (width_fit_children)
				use_children_width(w);
			if (height_fit_children)
				use_children_height(h);

			for (auto i = 0; i < n; i++)
				apply_h_free_layout(als[i], false);
			h = element->size.y() - h;
			if (h > 0.f && factor > 0)
				h /= factor;
			else
				h = 0.f;
			auto y = element->padding[1];
			for (auto i = 0; i < n; i++)
			{
				auto& al = als[i];
				auto element = al.element;
				auto aligner = al.aligner;
				auto y_flags = aligner ? aligner->y_align_flags : (AlignFlag)0;

				if (y_flags & AlignMinMax)
				{
					auto _h = h * aligner->height_factor;
					if (y_flags & AlignGreedy)
						_h += aligner->min_height;
					element->set_height(_h, this);
				}
				element->set_y(scroll_offset.y() + y, this);
				y += element->size.y() + item_padding;
			}
			for (auto i = n; i < als.size(); i++)
			{
				apply_h_free_layout(als[i], true);
				apply_v_free_layout(als[i], true);
			}
		}
			break;
		case LayoutGrid:
		{
			auto n = fence >= 0 ? min(fence, (int)als.size()) : max(0, (int)als.size() + fence);

			if (column == 0)
			{
				set_content_size(Vec2f(0.f));
				if (width_fit_children)
					use_children_width(element->padding.xz().sum());
				if (height_fit_children)
					use_children_height(element->padding.yw().sum());
				for (auto i = 0; i < n; i++)
				{
					auto& al = als[i];
					auto element = al.element;
					auto aligner = al.aligner;

					element->set_x(scroll_offset.x() + element->padding.x(), this);
					element->set_y(scroll_offset.y() + element->padding.y(), this);
				}
				for (auto i = n; i < als.size(); i++)
				{
					apply_h_free_layout(als[i], true);
					apply_v_free_layout(als[i], true);
				}
			}
			else
			{
				auto w = 0.f;
				auto h = 0.f;
				auto lw = 0.f;
				auto lh = 0.f;
				for (auto i = 0; i < n; i++)
				{
					auto& al = als[i];
					auto element = al.element;
					auto aligner = al.aligner;

					lw += element->size.x() + item_padding;
					lh = max(element->size.y(), lh);

					if ((i + 1) % column == 0)
					{
						w = max(lw - item_padding, w);
						h += lh + item_padding;
						lw = 0.f;
						lh = 0.f;
					}
				}
				if (fence != 0 && !als.empty())
				{
					if (n % column != 0)
					{
						w = max(lw - item_padding, w);
						h += lh + item_padding;
					}
					h -= item_padding;
				}
				set_content_size(Vec2f(w, h));
				w += element->padding.xz().sum();
				h += element->padding.yw().sum();
				if (width_fit_children)
					use_children_width(w);
				if (height_fit_children)
					use_children_height(h);

				auto x = element->padding[0];
				auto y = element->padding[1];
				lh = 0.f;
				for (auto i = 0; i < n; i++)
				{
					auto& al = als[i];
					auto element = al.element;
					auto aligner = al.aligner;

					element->set_x(scroll_offset.x() + x, this);
					element->set_y(scroll_offset.y() + y, this);

					x += element->size.x() + item_padding;
					lh = max(element->size.y(), lh);

					if ((i + 1) % column == 0)
					{
						x = element->padding[0];
						y += lh + item_padding;
						lh = 0.f;
					}
				}
				for (auto i = n; i < als.size(); i++)
				{
					apply_h_free_layout(als[i], true);
					apply_v_free_layout(als[i], true);
				}
			}
		}
			break;
		}
	}

	void cLayout::set_x_scroll_offset(float x)
	{
		if (scroll_offset.x() == x)
			return;
		scroll_offset.x() = x;
		auto thiz = (cLayoutPrivate*)this;
		if (thiz->management)
			thiz->management->add_to_update_list(thiz);
	}

	void cLayout::set_y_scroll_offset(float y)
	{
		if (scroll_offset.y() == y)
			return;
		scroll_offset.y() = y;
		auto thiz = (cLayoutPrivate*)this;
		if (thiz->management)
			thiz->management->add_to_update_list(thiz);
	}

	void cLayout::set_column(uint c)
	{
		if (column == c)
			return;
		column = c;
		auto thiz = (cLayoutPrivate*)this;
		if (thiz->management)
			thiz->management->add_to_update_list(thiz);
	}

	cLayout* cLayout::create(LayoutType type)
	{
		return new cLayoutPrivate(type);
	}
}
