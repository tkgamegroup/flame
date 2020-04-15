#include <flame/graphics/font.h>
#include <flame/universe/world.h>
#include <flame/universe/systems/layout_management.h>
#include <flame/universe/components/element.h>
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

	void cLayoutPrivate::apply_h_free_layout(cElement* _element, cAligner* _aligner, bool lock = false)
	{
		auto padding = (lock || (_aligner ? _aligner->using_padding_ : false)) ? element->padding.xz() : Vec2f(0.f);
		auto w = element->size.x() - padding[0] - padding[1];
		switch (_aligner ? _aligner->width_policy_ : SizeFixed)
		{
		case SizeFitParent:
			_element->set_width(w, false, this);
			break;
		case SizeGreedy:
			if (w > _aligner->min_width_)
				_element->set_width(w, false, this);
			break;
		}
		switch (_aligner ? _aligner->x_align_ : AlignxFree)
		{
		case AlignxFree:
			if (!lock)
				break;
		case AlignxLeft:
			_element->set_x(scroll_offset.x() + padding[0], false, this);
			break;
		case AlignxMiddle:
			_element->set_x(scroll_offset.x() + padding[0] + (w - _element->size.x()) * 0.5f, false, this);
			break;
		case AlignxRight:
			_element->set_x(scroll_offset.x() + element->size.x() - padding[1] - _element->size.x(), false, this);
			break;
		}
	}

	void cLayoutPrivate::apply_v_free_layout(cElement* _element, cAligner* _aligner, bool lock = false)
	{
		auto padding = (lock || (_aligner ? _aligner->using_padding_ : false)) ? element->padding.yw() : Vec2f(0.f);
		auto h = element->size.y() - padding[0] - padding[1];
		switch (_aligner ? _aligner->height_policy_ : SizeFixed)
		{
		case SizeFitParent:
			_element->set_height(h, false, this);
			break;
		case SizeGreedy:
			if (h > _aligner->min_height_)
				_element->set_height(h, false, this);
			break;
		}
		switch (_aligner ? _aligner->y_align_ : AlignyFree)
		{
		case AlignyFree:
			if (!lock)
				break;
		case AlignyTop:
			_element->set_y(scroll_offset.y() + padding[0], false, this);
			break;
		case AlignyMiddle:
			_element->set_y(scroll_offset.y() + padding[0] + (h - _element->size.y()) * 0.5f, false, this);
			break;
		case AlignyBottom:
			_element->set_y(scroll_offset.y() + element->size.y() - padding[1] - _element->size.y(), false, this);
			break;
		}
	}

	void cLayoutPrivate::use_children_width(float w)
	{
		if (!aligner || aligner->width_policy_ == SizeFixed)
			element->set_width(w, false, this);
		else if (aligner && aligner->width_policy_ == SizeGreedy)
		{
			aligner->set_min_width(w);
			element->set_width(max(element->size.x(), w), false, this);
		}
	}

	void cLayoutPrivate::use_children_height(float h)
	{
		if (!aligner || aligner->height_policy_ == SizeFixed)
			element->set_height(h, false, this);
		else if (aligner && aligner->height_policy_ == SizeGreedy)
		{
			aligner->set_min_height(h);
			element->set_height(max(element->size.y(), h), false, this);
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
					al.element = nullptr;
					return;
				}
				if (c == al.aligner)
				{
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
					auto element = e->get_component(cElement);
					auto aligner = e->get_component(cAligner);
					void* element_data_listener = nullptr;
					if (element)
					{
						element_data_listener = element->data_changed_listeners.add([](void* c, uint hash, void* sender) {
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
					void* aligner_data_listener = nullptr;
					if (aligner)
					{
						aligner_data_listener = aligner->data_changed_listeners.add([](void* c, uint hash, void* sender) {
							auto thiz = *(cLayoutPrivate**)c;
							if (sender == thiz)
								return true;
							if (thiz->management)
								thiz->management->add_to_update_list(thiz);
							return true;
						}, Mail::from_p(this));
					}
					als.push_back({ element, aligner,
						element_data_listener,
						aligner_data_listener});
				}
			}
			als_dirty = false;
		}

		switch (type)
		{
		case LayoutFree:
			for (auto al : als)
			{
				apply_h_free_layout(al.element, al.aligner);
				apply_v_free_layout(al.element, al.aligner);
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
				auto element = al.element;
				auto aligner = al.aligner;

				switch (aligner ? aligner->width_policy_ : SizeFixed)
				{
				case SizeFixed:
					w += element->size.x();
					break;
				case SizeFitParent:
					factor += aligner->width_factor_;
					break;
				case SizeGreedy:
					factor += aligner->width_factor_;
					w += aligner->min_width_;
					break;
				}
				switch (aligner ? aligner->height_policy_ : SizeFixed)
				{
				case SizeFixed:
					h = max(element->size.y(), h);
					break;
				case SizeFitParent:
					break;
				case SizeGreedy:
					h = max(aligner->min_height_, h);
					break;
				}
				w += item_padding;
			}
			if (fence > 0 && !als.empty())
				w -= item_padding;
			set_content_size(Vec2f(w, h));
			w += element->padding_h();
			h += element->padding_v();
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

				if (aligner)
				{
					if (aligner->width_policy_ == SizeFitParent)
						element->set_width(w * aligner->width_factor_, false, this);
					else if (aligner->width_policy_ == SizeGreedy)
						element->set_width(aligner->min_width_ + w * aligner->width_factor_, false, this);
				}
				assert(!aligner || aligner->x_align_ == AlignxFree);
				element->set_x(scroll_offset.x() + x, false, this);
				x += element->size.x() + item_padding;
			}
			for (auto i = n; i < als.size(); i++)
				apply_h_free_layout(als[i].element, als[i].aligner, false);
			for (auto i = 0; i < n; i++)
				apply_v_free_layout(als[i].element, als[i].aligner, true);
			for (auto i = n; i < als.size(); i++)
				apply_v_free_layout(als[i].element, als[i].aligner, false);
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
				auto element = al.element;
				auto aligner = al.aligner;

				switch (aligner ? aligner->width_policy_ : SizeFixed)
				{
				case SizeFixed:
					w = max(element->size.x(), w);
					break;
				case SizeFitParent:
					break;
				case SizeGreedy:
					w = max(aligner->min_width_, w);
					break;
				}
				switch (aligner ? aligner->height_policy_ : SizeFixed)
				{
				case SizeFixed:
					h += element->size.y();
					break;
				case SizeFitParent:
					factor += aligner->height_factor_;
					break;
				case SizeGreedy:
					factor += aligner->height_factor_;
					h += aligner->min_height_;
					break;
				}
				h += item_padding;
			}
			if (fence > 0 && !als.empty())
				h -= item_padding;
			set_content_size(Vec2f(w, h));
			w += element->padding_h();
			h += element->padding_v();
			if (width_fit_children)
				use_children_width(w);
			if (height_fit_children)
				use_children_height(h);

			for (auto i = 0; i < n; i++)
				apply_h_free_layout(als[i].element, als[i].aligner, true);
			for (auto i = n; i < als.size(); i++)
				apply_h_free_layout(als[i].element, als[i].aligner, false);
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

				if (aligner)
				{
					if (aligner->height_policy_ == SizeFitParent)
						element->set_height(h * aligner->height_factor_, false, this);
					else if (aligner->height_policy_ == SizeGreedy)
						element->set_height(aligner->min_height_ + h * aligner->height_factor_, false, this);
				}
				assert(!aligner || aligner->y_align_ == AlignyFree);
				element->set_y(scroll_offset.y() + y, false, this);
				y += element->size.y() + item_padding;
			}
			for (auto i = n; i < als.size(); i++)
				apply_v_free_layout(als[i].element, als[i].aligner, false);
		}
			break;
		case LayoutGrid:
		{
			auto n = min(fence, (uint)als.size());

			if (column == 0)
			{
				set_content_size(Vec2f(0.f));
				if (width_fit_children)
					use_children_width(element->padding_h());
				if (height_fit_children)
					use_children_height(element->padding_v());
				for (auto i = 0; i < n; i++)
				{
					auto& al = als[i];
					auto element = al.element;
					auto aligner = al.aligner;

					assert(!aligner || (aligner->x_align_ == AlignxFree && aligner->y_align_ == AlignyFree));

					element->set_x(scroll_offset.x() + element->padding[0], false, this);
					element->set_y(scroll_offset.y() + element->padding[1], false, this);
				}
				for (auto i = n; i < als.size(); i++)
				{
					apply_h_free_layout(als[i].element, als[i].aligner, false);
					apply_v_free_layout(als[i].element, als[i].aligner, false);
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

					assert(!aligner || (aligner->width_policy_ == SizeFixed && aligner->height_policy_ == SizeFixed));

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
				if (fence > 0 && !als.empty())
				{
					if (n % column != 0)
					{
						w = max(lw - item_padding, w);
						h += lh + item_padding;
					}
					h -= item_padding;
				}
				set_content_size(Vec2f(w, h));
				w += element->padding_h();
				h += element->padding_v();
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

					assert(!aligner || (aligner->x_align_ == AlignxFree && aligner->y_align_ == AlignyFree));

					element->set_x(scroll_offset.x() + x, false, this);
					element->set_y(scroll_offset.y() + y, false, this);

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
					apply_h_free_layout(als[i].element, als[i].aligner, false);
					apply_v_free_layout(als[i].element, als[i].aligner, false);
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
