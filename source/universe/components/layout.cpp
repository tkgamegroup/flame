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
		scroll_offset_ = Vec2f(0.f);
		column = 0;

		content_size = Vec2f(0.f);

		management = nullptr;
		pending_update = false;

		element_data_listener = nullptr;

		als_dirty = true;
	}

	cLayoutPrivate::~cLayoutPrivate()
	{
		management->remove_from_update_list(this);
		if (!entity->dying_)
			element->data_changed_listeners.remove(element_data_listener);
	}

	void cLayoutPrivate::apply_h_free_layout(cElement* _element, cAligner* _aligner, bool lock = false)
	{
		auto padding = (lock || (_aligner ? _aligner->using_padding_ : false)) ? element->inner_padding_.xz() : Vec2f(0.f);
		auto w = element->size_.x() - padding[0] - padding[1];
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
			_element->set_x(scroll_offset_.x() + padding[0], false, this);
			break;
		case AlignxMiddle:
			_element->set_x(scroll_offset_.x() + (w - _element->size_.x()) * 0.5f, false, this);
			break;
		case AlignxRight:
			_element->set_x(scroll_offset_.x() + element->size_.x() - padding[1] - _element->size_.x(), false, this);
			break;
		}
	}

	void cLayoutPrivate::apply_v_free_layout(cElement* _element, cAligner* _aligner, bool lock = false)
	{
		auto padding = (lock || (_aligner ? _aligner->using_padding_ : false)) ? element->inner_padding_.yw() : Vec2f(0.f);
		auto h = element->size_.y() - padding[0] - padding[1];
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
			_element->set_y(scroll_offset_.y() + padding[0], false, this);
			break;
		case AlignyMiddle:
			_element->set_y(scroll_offset_.y() + (h - _element->size_.y()) * 0.5f, false, this);
			break;
		case AlignyBottom:
			_element->set_y(scroll_offset_.y() + element->size_.y() - padding[1] - _element->size_.y(), false, this);
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
			element->set_width(max(element->size_.x(), w), false, this);
		}
	}

	void cLayoutPrivate::use_children_height(float h)
	{
		if (!aligner || aligner->height_policy_ == SizeFixed)
			element->set_height(h, false, this);
		else if (aligner && aligner->height_policy_ == SizeGreedy)
		{
			aligner->set_min_height(h);
			element->set_height(max(element->size_.y(), h), false, this);
		}
	}

	void cLayoutPrivate::on_entered_world()
	{
		management = entity->world_->get_system(LayoutManagement);
		management->add_to_update_list(this);
	}

	void cLayoutPrivate::on_left_world()
	{
		management->remove_from_update_list(this);
		management = nullptr;
	}

	void cLayoutPrivate::on_component_added(Component* c)
	{
		if (c->name_hash == cH("Element"))
		{
			element = (cElement*)c;
			element_data_listener = element->data_changed_listeners.add([](void* c, Component*, uint hash, void* sender) {
				auto thiz = *(cLayoutPrivate**)c;
				if (sender == thiz)
					return;
				switch (hash)
				{
				case cH("size"):
				case cH("inner_padding"):
					thiz->management->add_to_update_list(thiz);
				}
			}, new_mail_p(this));
		}
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
		if (c->name_hash == cH("Element") ||
			c->name_hash == cH("Aligner") ||
			c->name_hash == cH("Text"))
		{
			als_dirty = true;
			if (management)
				management->add_to_update_list(this);
		}
	}

	void cLayoutPrivate::on_child_component_removed(Component* c)
	{
		if (c->name_hash == cH("Element") ||
			c->name_hash == cH("Aligner") ||
			c->name_hash == cH("Text"))
		{
			als_dirty = true;
			if (management)
				management->add_to_update_list(this);
			for (auto& al : als)
			{
				if (c == std::get<0>(al))
				{
					std::get<0>(al) = nullptr;
					return;
				}
				if (c == std::get<1>(al))
				{
					std::get<1>(al) = nullptr;
					return;
				}
				if (c == std::get<2>(al))
				{
					std::get<2>(al) = nullptr;
					return;
				}
			}
		}
	}

	void cLayoutPrivate::update()
	{
		if (als_dirty)
		{
			for (auto& al : als)
			{
				if (std::get<0>(al))
					std::get<0>(al)->data_changed_listeners.remove(std::get<3>(al));
				if (std::get<1>(al))
					std::get<1>(al)->data_changed_listeners.remove(std::get<4>(al));
				if (std::get<2>(al))
					std::get<2>(al)->data_changed_listeners.remove(std::get<5>(al));
			}
			als.clear();
			for (auto i = 0; i < entity->child_count(); i++)
			{
				auto e = entity->child(i);
				if (e->global_visibility_)
				{
					auto element = e->get_component(Element);
					auto aligner = e->get_component(Aligner);
					auto text = e->get_component(Text);
					void* element_data_listener = nullptr;
					if (element)
					{
						element_data_listener = element->data_changed_listeners.add([](void* c, Component*, uint hash, void* sender) {
							auto thiz = *(cLayoutPrivate**)c;
							if (sender == thiz)
								return;
							switch (hash)
							{
							case cH("pos"):
							case cH("size"):
								thiz->management->add_to_update_list(thiz);
								break;
							}
						}, new_mail_p(this));
					}
					void* aligner_data_listener = nullptr;
					if (aligner)
					{
						aligner_data_listener = aligner->data_changed_listeners.add([](void* c, Component*, uint hash, void* sender) {
							auto thiz = *(cLayoutPrivate**)c;
							if (sender == thiz)
								return;
							thiz->management->add_to_update_list(thiz);
						}, new_mail_p(this));
					}
					void* text_data_listener = nullptr;
					if (text)
					{
						text_data_listener = text->data_changed_listeners.add([](void* c, Component*, uint hash, void* sender) {
							auto thiz = *(cLayoutPrivate**)c;
							if (sender == thiz)
								return;
							switch (hash)
							{
							case cH("text"):
							case cH("sdf_scale"):
							case cH("auto_width"):
							case cH("auto_height"):
								thiz->management->add_to_update_list(thiz);
								break;
							}
						}, new_mail_p(this));
					}
					als.emplace_back(element, aligner, text, 
						element_data_listener,
						aligner_data_listener,
						text_data_listener);
				}
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

				auto s = Vec2f(text->font_atlas->get_text_size(text->text())) * text->sdf_scale_;
				if (text->auto_width_)
				{
					auto w = s.x() + element->inner_padding_horizontal();
					if (aligner && aligner->width_policy_ == SizeGreedy)
						aligner->set_min_width(w);
					element->set_width(w, false, this);
				}
				if (text->auto_height_)
				{
					auto h = s.y() + element->inner_padding_vertical();
					if (aligner && aligner->height_policy_ == SizeGreedy)
						aligner->set_min_height(h);
					element->set_height(h, false, this);
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

				switch (aligner ? aligner->width_policy_ : SizeFixed)
				{
				case SizeFixed:
					w += element->size_.x();
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
					h = max(element->size_.y(), h);
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
			content_size = Vec2f(w, h);
			w += element->inner_padding_horizontal();
			h += element->inner_padding_vertical();
			if (width_fit_children)
				use_children_width(w);
			if (height_fit_children)
				use_children_height(h);

			w = element->size_.x() - w;
			if (w > 0.f && factor > 0)
				w /= factor;
			else
				w = 0.f;
			auto x = element->inner_padding_[0];
			for (auto i = 0; i < n; i++)
			{
				auto& al = als[i];
				auto element = std::get<0>(al);
				auto aligner = std::get<1>(al);

				if (aligner)
				{
					if (aligner->width_policy_ == SizeFitParent)
						element->set_width(w * aligner->width_factor_, false, this);
					else if (aligner->width_policy_ == SizeGreedy)
						element->set_width(aligner->min_width_ + w * aligner->width_factor_, false, this);
				}
				assert(!aligner || aligner->x_align_ == AlignxFree);
				element->set_x(scroll_offset_.x() + x, false, this);
				x += element->size_.x() + item_padding;
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

				switch (aligner ? aligner->width_policy_ : SizeFixed)
				{
				case SizeFixed:
					w = max(element->size_.x(), w);
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
					h += element->size_.y();
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
			h = element->size_.y() - h;
			if (h > 0.f && factor > 0)
				h /= factor;
			else
				h = 0.f;
			auto y = element->inner_padding_[1];
			for (auto i = 0; i < n; i++)
			{
				auto& al = als[i];
				auto element = std::get<0>(al);
				auto aligner = std::get<1>(al);

				if (aligner)
				{
					if (aligner->height_policy_ == SizeFitParent)
						element->set_height(h * aligner->height_factor_, false, this);
					else if (aligner->height_policy_ == SizeGreedy)
						element->set_height(aligner->min_height_ + h * aligner->height_factor_, false, this);
				}
				assert(!aligner || aligner->y_align_ == AlignyFree);
				element->set_y(scroll_offset_.y() + y, false, this);
				y += element->size_.y() + item_padding;
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

					assert(!aligner || (aligner->x_align_ == AlignxFree && aligner->y_align_ == AlignyFree));

					element->set_x(scroll_offset_.x() + element->inner_padding_[0], false, this);
					element->set_y(scroll_offset_.y() + element->inner_padding_[1], false, this);
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

					assert(!aligner || (aligner->width_policy_ == SizeFixed && aligner->height_policy_ == SizeFixed));

					lw += element->size_.x() + item_padding;
					lh = max(element->size_.y(), lh);

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

				auto x = element->inner_padding_[0];
				auto y = element->inner_padding_[1];
				lh = 0.f;
				c = 0;
				for (auto i = 0; i < n; i++)
				{
					auto& al = als[i];
					auto element = std::get<0>(al);
					auto aligner = std::get<1>(al);

					assert(!aligner || (aligner->x_align_ == AlignxFree && aligner->y_align_ == AlignyFree));

					element->set_x(scroll_offset_.x() + x, false, this);
					element->set_y(scroll_offset_.y() + y, false, this);

					x += element->size_.x() + item_padding;
					lh = max(element->size_.y(), lh);

					c++;
					if (c == column)
					{
						x = element->inner_padding_[0];
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

	void cLayout::set_x_scroll_offset(float x)
	{
		scroll_offset_.x() = x;
		((cLayoutPrivate*)this)->management->add_to_update_list(this);
	}

	void cLayout::set_y_scroll_offset(float y)
	{
		scroll_offset_.y() = y;
		((cLayoutPrivate*)this)->management->add_to_update_list(this);
	}

	cLayout* cLayout::create(LayoutType type)
	{
		return new cLayoutPrivate(type);
	}
}
