#include <flame/graphics/font.h>
#include <flame/universe/world.h>
#include <flame/universe/systems/layout_management.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/aligner.h>
#include "layout_private.h"

namespace flame
{
	cLayoutPrivate::cLayoutPrivate(LayoutType _type)
	{
		element = nullptr;
		aligner = nullptr;

		type = _type;
		item_padding = 0.f;
		width_fit_children = true;
		height_fit_children = true;
		fence = -1;
		scroll_offset = Vec2f(0.f);
		column = 0;

		content_size = Vec2f(0.f);

		management = nullptr;
		pending_update = false;

		als_dirty = true;
	}

	cLayoutPrivate::~cLayoutPrivate()
	{
		management->remove_from_update_list(this);
	}

	void cLayoutPrivate::apply_h_free_layout(cElement* _element, cAligner* _aligner, bool lock = false)
	{
		auto padding = (lock || (_aligner ? _aligner->using_padding : false)) ? element->inner_padding.xz() : Vec2f(0.f);
		auto w = element->size.x() - padding[0] - padding[1];
		switch (_aligner ? _aligner->width_policy : SizeFixed)
		{
		case SizeFitParent:
			_element->size.x() = w;
			break;
		case SizeGreedy:
			if (w > _aligner->min_size.x())
				_element->size.x() = w;
			break;
		}
		switch (_aligner ? _aligner->x_align : AlignxFree)
		{
		case AlignxFree:
			if (!lock)
				break;
		case AlignxLeft:
			_element->pos.x() = scroll_offset.x() + padding[0];
			break;
		case AlignxMiddle:
			_element->pos.x() = scroll_offset.x() + (w - _element->size.x()) * 0.5f;
			break;
		case AlignxRight:
			_element->pos.x() = scroll_offset.x() + element->size.x() - padding[1] - _element->size.x();
			break;
		}
	}

	void cLayoutPrivate::apply_v_free_layout(cElement* _element, cAligner* _aligner, bool lock = false)
	{
		auto padding = (lock || (_aligner ? _aligner->using_padding : false)) ? element->inner_padding.yw() : Vec2f(0.f);
		auto h = element->size.y() - padding[0] - padding[1];
		switch (_aligner ? _aligner->height_policy : SizeFixed)
		{
		case SizeFitParent:
			_element->size.y() = h;
			break;
		case SizeGreedy:
			if (h > _aligner->min_size.y())
				_element->size.y() = h;
			break;
		}
		switch (_aligner ? _aligner->y_align : AlignyFree)
		{
		case AlignyFree:
			if (!lock)
				break;
		case AlignyTop:
			_element->pos.y() = scroll_offset.y() + padding[0];
			break;
		case AlignyMiddle:
			_element->pos.y() = scroll_offset.y() + (h - _element->size.y()) * 0.5f;
			break;
		case AlignyBottom:
			_element->pos.y() = scroll_offset.y() + element->size.y() - padding[1] - _element->size.y();
			break;
		}
	}

	void cLayoutPrivate::use_children_width(float w)
	{
		if (aligner && aligner->width_policy == SizeGreedy)
		{
			aligner->min_size.x() = w;
			element->size.x() = max(element->size.x(), w);
		}
		else
			element->size.x() = w;
	}

	void cLayoutPrivate::use_children_height(float h)
	{
		if (aligner && aligner->height_policy == SizeGreedy)
		{
			aligner->min_size.y() = h;
			element->size.y() = max(element->size.y(), h);
		}
		else
			element->size.y() = h;
	}

	void cLayoutPrivate::on_into_world()
	{
		management = entity->world_->get_system(LayoutManagement);
		management->add_to_update_list(this);
	}

	void cLayoutPrivate::on_component_added(Component* c)
	{
		if (c->name_hash == cH("Element"))
			element = (cElement*)c;
		else if (c->name_hash == cH("Aligner"))
			aligner = (cAligner*)c;
	}

	void cLayoutPrivate::on_child_visibility_changed()
	{
		als_dirty = true;
		if (management)
			management->add_to_update_list(this);
	}

	void cLayoutPrivate::on_child_component_added(Component* c)
	{
		if (c->name_hash == cH("Element"))
		{
			als_dirty = true;
			if (management)
				management->add_to_update_list(this);
		}
		else if (c->name_hash == cH("Aligner"))
		{
			als_dirty = true;
			if (management)
				management->add_to_update_list(this);
		}
	}

	void cLayoutPrivate::on_child_component_removed(Component* c)
	{
		if (c->name_hash == cH("Element"))
		{
			als_dirty = true;
			if (management)
				management->add_to_update_list(this);
		}
		else if (c->name_hash == cH("Aligner"))
		{
			als_dirty = true;
			if (management)
				management->add_to_update_list(this);
		}
		else if (c->name_hash == cH("Text"))
		{
			auto t = (cText*)c;
			if (t->auto_width || t->auto_height)
			{
				als_dirty = true;
				if (management)
					management->add_to_update_list(this);
			}
		}
	}

	void cLayoutPrivate::update()
	{
		if (als_dirty)
		{
			als.clear();
			for (auto i = 0; i < entity->child_count(); i++)
			{
				auto e = entity->child(i);
				if (e->global_visibility_)
					als.emplace_back(e->get_component(Element), e->get_component(Aligner), e->get_component(Text));
			}
			als_dirty = false;
		}

		for (auto& al : als)
		{
			auto text = std::get<2>(al);
			if (text)
			{
				auto element = std::get<0>(al);
				auto aligner = std::get<1>(al);

				auto s = Vec2f(text->font_atlas->get_text_size(text->text())) * text->sdf_scale;
				if (text->auto_width)
				{
					auto w = s.x() + element->inner_padding_horizontal();
					if (aligner && aligner->width_policy == SizeGreedy)
						aligner->min_size.x() = w;
					element->size.x() = w;
				}
				if (text->auto_height)
				{
					auto h = s.y() + element->inner_padding_vertical();
					if (aligner && aligner->width_policy == SizeGreedy)
						aligner->min_size.y() = h;
					element->size.y() = h;
				}
			}
		}

		switch (type)
		{
		case LayoutFree:
			for (auto al : als)
			{
				apply_h_free_layout(std::get<0>(al), std::get<1>(al));
				apply_v_free_layout(std::get<0>(al), std::get<1>(al));
			}
			break;
		case LayoutHorizontal:
		{
			auto n = min(fence, (uint)als.size());

			auto w = 0.f;
			auto h = 0.f;
			auto factor = 0.f;
			for (auto i = 0; i < n; i++)
			{
				auto& al = als[i];
				auto element = std::get<0>(al);
				auto aligner = std::get<1>(al);

				switch (aligner ? aligner->width_policy : SizeFixed)
				{
				case SizeFixed:
					w += element->size.x();
					break;
				case SizeFitParent:
					factor += aligner->width_factor;
					break;
				case SizeGreedy:
					factor += aligner->width_factor;
					w += aligner->min_size.x();
					break;
				}
				switch (aligner ? aligner->height_policy : SizeFixed)
				{
				case SizeFixed:
					h = max(element->size.y(), h);
					break;
				case SizeFitParent:
					break;
				case SizeGreedy:
					h = max(aligner->min_size.y(), h);
					break;
				}
				w += item_padding;
			}
			if (fence > 0 && !als.empty())
				w -= item_padding;
			content_size = Vec2f(w, h);
			w += element->inner_padding_horizontal();
			h += element->inner_padding_vertical();
			if (width_fit_children)
				use_children_width(w);
			if (height_fit_children)
				use_children_height(h);

			w = element->size.x() - w;
			if (w > 0.f && factor > 0)
				w /= factor;
			else
				w = 0.f;
			auto x = element->inner_padding[0];
			for (auto i = 0; i < n; i++)
			{
				auto& al = als[i];
				auto element = std::get<0>(al);
				auto aligner = std::get<1>(al);

				if (aligner)
				{
					if (aligner->width_policy == SizeFitParent)
						element->size.x() = w * aligner->width_factor;
					else if (aligner->width_policy == SizeGreedy)
						element->size.x() = aligner->min_size.x() + w * aligner->width_factor;
				}
				assert(!aligner || aligner->x_align == AlignxFree);
				element->pos.x() = scroll_offset.x() + x;
				x += element->size.x() + item_padding;
			}
			for (auto i = n; i < als.size(); i++)
				apply_h_free_layout(std::get<0>(als[i]), std::get<1>(als[i]), false);
			for (auto i = 0; i < n; i++)
				apply_v_free_layout(std::get<0>(als[i]), std::get<1>(als[i]), true);
			for (auto i = n; i < als.size(); i++)
				apply_v_free_layout(std::get<0>(als[i]), std::get<1>(als[i]), false);
		}
			break;
		case LayoutVertical:
		{
			auto n = min(fence, (uint)als.size());

			auto w = 0.f;
			auto h = 0.f;
			auto factor = 0.f;
			for (auto i = 0; i < n; i++)
			{
				auto& al = als[i];
				auto element = std::get<0>(al);
				auto aligner = std::get<1>(al);

				switch (aligner ? aligner->width_policy : SizeFixed)
				{
				case SizeFixed:
					w = max(element->size.x(), w);
					break;
				case SizeFitParent:
					break;
				case SizeGreedy:
					w = max(aligner->min_size.x(), w);
					break;
				}
				switch (aligner ? aligner->height_policy : SizeFixed)
				{
				case SizeFixed:
					h += element->size.y();
					break;
				case SizeFitParent:
					factor += aligner->height_factor;
					break;
				case SizeGreedy:
					factor += aligner->height_factor;
					h += aligner->min_size.y();
					break;
				}
				h += item_padding;
			}
			if (fence > 0 && !als.empty())
				h -= item_padding;
			content_size = Vec2f(w, h);
			w += element->inner_padding_horizontal();
			h += element->inner_padding_vertical();
			if (width_fit_children)
				use_children_width(w);
			if (height_fit_children)
				use_children_height(h);

			for (auto i = 0; i < n; i++)
				apply_h_free_layout(std::get<0>(als[i]), std::get<1>(als[i]), true);
			for (auto i = n; i < als.size(); i++)
				apply_h_free_layout(std::get<0>(als[i]), std::get<1>(als[i]), false);
			h = element->size.y() - h;
			if (h > 0.f && factor > 0)
				h /= factor;
			else
				h = 0.f;
			auto y = element->inner_padding[1];
			for (auto i = 0; i < n; i++)
			{
				auto& al = als[i];
				auto element = std::get<0>(al);
				auto aligner = std::get<1>(al);

				if (aligner)
				{
					if (aligner->height_policy == SizeFitParent)
						element->size.y() = h * aligner->height_factor;
					else if (aligner->height_policy == SizeGreedy)
						element->size.y() = aligner->min_size.y() + h * aligner->height_factor;
				}
				assert(!aligner || aligner->y_align == AlignyFree);
				element->pos.y() = scroll_offset.y() + y;
				y += element->size.y() + item_padding;
			}
			for (auto i = n; i < als.size(); i++)
				apply_v_free_layout(std::get<0>(als[i]), std::get<1>(als[i]), false);
		}
			break;
		case LayoutGrid:
		{
			auto n = min(fence, (uint)als.size());

			if (column == 0)
			{
				content_size = Vec2f(0.f);
				if (width_fit_children)
					use_children_width(element->inner_padding_horizontal());
				if (height_fit_children)
					use_children_height(element->inner_padding_vertical());
				for (auto i = 0; i < n; i++)
				{
					auto& al = als[i];
					auto element = std::get<0>(al);
					auto aligner = std::get<1>(al);

					assert(!aligner || (aligner->x_align == AlignxFree && aligner->y_align == AlignyFree));

					element->pos.x() = scroll_offset.x() + element->inner_padding[0];
					element->pos.y() = scroll_offset.y() + element->inner_padding[1];
				}
				for (auto i = n; i < als.size(); i++)
				{
					apply_h_free_layout(std::get<0>(als[i]), std::get<1>(als[i]), false);
					apply_v_free_layout(std::get<0>(als[i]), std::get<1>(als[i]), false);
				}
			}
			else
			{
				auto w = 0.f;
				auto h = 0.f;
				auto lw = 0.f;
				auto lh = 0.f;
				auto c = 0;
				for (auto i = 0; i < n; i++)
				{
					auto& al = als[i];
					auto element = std::get<0>(al);
					auto aligner = std::get<1>(al);

					assert(!aligner || (aligner->width_policy == SizeFixed && aligner->height_policy == SizeFixed));

					lw += element->size.x() + item_padding;
					lh = max(element->size.y(), lh);

					c++;
					if (c == column)
					{
						w = max(lw - item_padding, w);
						h += lh + item_padding;
						lw = 0.f;
						lh = 0.f;
						c = 0;
					}
				}
				if (fence > 0 && !als.empty())
				{
					if (n % column != 0)
					{
						w = max(lw - item_padding, w);
						h += lh + item_padding;
					}
					h -= item_padding;
				}
				content_size = Vec2f(w, h);
				w += element->inner_padding_horizontal();
				h += element->inner_padding_vertical();
				if (width_fit_children)
					use_children_width(w);
				if (height_fit_children)
					use_children_height(h);

				auto x = element->inner_padding[0];
				auto y = element->inner_padding[1];
				lh = 0.f;
				c = 0;
				for (auto i = 0; i < n; i++)
				{
					auto& al = als[i];
					auto element = std::get<0>(al);
					auto aligner = std::get<1>(al);

					assert(!aligner || (aligner->x_align == AlignxFree && aligner->y_align == AlignyFree));

					element->pos.x() = scroll_offset.x() + x;
					element->pos.y() = scroll_offset.y() + y;

					x += element->size.x() + item_padding;
					lh = max(element->size.y(), lh);

					c++;
					if (c == column)
					{
						x = element->inner_padding[0];
						y += lh + item_padding;
						lh = 0.f;
						c = 0;
					}
				}
				for (auto i = n; i < als.size(); i++)
				{
					apply_h_free_layout(std::get<0>(als[i]), std::get<1>(als[i]), false);
					apply_v_free_layout(std::get<0>(als[i]), std::get<1>(als[i]), false);
				}
			}
		}
			break;
		}
	}

	Component* cLayoutPrivate::copy()
	{
		auto copy = new cLayoutPrivate(type);

		copy->item_padding = item_padding;
		copy->width_fit_children = width_fit_children;
		copy->height_fit_children = height_fit_children;

		return copy;
	}

	cLayout* cLayout::create(LayoutType type)
	{
		return new cLayoutPrivate(type);
	}
}
