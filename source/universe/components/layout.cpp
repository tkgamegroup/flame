#include <flame/universe/components/element.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>

namespace flame
{
	struct cLayoutPrivate : cLayout
	{
		bool als_dirty;
		std::vector<std::pair<cElement*, cAligner*>> als;

		cLayoutPrivate(LayoutType _type)
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

			als_dirty = true;
		}

		void apply_h_free_layout(const std::pair<cElement*, cAligner*>& al, bool lock = false)
		{
			auto padding = (lock || (al.second ? al.second->using_padding : false)) ? element->inner_padding.xz() : Vec2f(0.f);
			auto w = element->size.x() - padding[0] - padding[1];
			switch (al.second ? al.second->width_policy : SizeFixed)
			{
			case SizeFitParent:
				al.first->size.x() = w;
				break;
			case SizeGreedy:
				if (w > al.second->min_size.x())
					al.first->size.x() = w;
				break;
			}
			switch (al.second ? al.second->x_align : AlignxFree)
			{
			case AlignxFree:
				if (!lock)
					break;
			case AlignxLeft:
				al.first->pos.x() = scroll_offset.x() + padding[0];
				break;
			case AlignxMiddle:
				al.first->pos.x() = scroll_offset.x() + (w - al.first->size.x()) * 0.5f;
				break;
			case AlignxRight:
				al.first->pos.x() = scroll_offset.x() + element->size.x() - padding[1] - al.first->size.x();
				break;
			}
		}

		void apply_v_free_layout(const std::pair<cElement*, cAligner*>& al, bool lock = false)
		{
			auto padding = (lock || (al.second ? al.second->using_padding : false)) ? element->inner_padding.yw() : Vec2f(0.f);
			auto h = element->size.y() - padding[0] - padding[1];
			switch (al.second ? al.second->height_policy : SizeFixed)
			{
			case SizeFitParent:
				al.first->size.y() = h;
				break;
			case SizeGreedy:
				if (h > al.second->min_size.y())
					al.first->size.y() = h;
				break;
			}
			switch (al.second ? al.second->y_align : AlignyFree)
			{
			case AlignyFree:
				if (!lock)
					break;
			case AlignyTop:
				al.first->pos.y() = scroll_offset.y() + padding[0];
				break;
			case AlignyMiddle:
				al.first->pos.y() = scroll_offset.y() + (h - al.first->size.y()) * 0.5f;
				break;
			case AlignyBottom:
				al.first->pos.y() = scroll_offset.y() + element->size.y() - padding[1] - al.first->size.y();
				break;
			}
		}

		void use_children_width(float w)
		{
			if (aligner && aligner->width_policy == SizeGreedy)
			{
				aligner->min_size.x() = w;
				element->size.x() = max(element->size.x(), w);
			}
			else
				element->size.x() = w;
		}

		void use_children_height(float h)
		{
			if (aligner && aligner->height_policy == SizeGreedy)
			{
				aligner->min_size.y() = h;
				element->size.y() = max(element->size.y(), h);
			}
			else
				element->size.y() = h;
		}

		virtual void on_component_added(Component* c) override
		{
			if (c->type_hash == cH("Element"))
				element = (cElement*)c;
			else if (c->type_hash == cH("Aligner"))
				aligner = (cAligner*)c;
		}

		virtual void on_child_visible_changed() override
		{
			als_dirty = true;
		}

		virtual void on_child_component_added(Component* c) override
		{
			if (c->type_hash == cH("Element"))
				als_dirty = true;
			else if (c->type_hash == cH("Aligner"))
				als_dirty = true;
		}

		virtual void on_child_component_removed(Component* c) override
		{
			if (c->type_hash == cH("Element"))
				als_dirty = true;
			else if (c->type_hash == cH("Aligner"))
				als_dirty = true;
		}

		virtual void update() override
		{
			if (als_dirty)
			{
				als.clear();
				for (auto i = 0; i < entity->child_count(); i++)
				{
					auto e = entity->child(i);
					if (e->visible_)
					{
						auto al = (cAligner*)e->find_component(cH("Aligner"));
						als.emplace_back(al ? al->element : (cElement*)entity->child(i)->find_component(cH("Element")), al);
					}
				}
				als_dirty = false;
			}

			switch (type)
			{
			case LayoutFree:
				for (auto al : als)
				{
					apply_h_free_layout(al);
					apply_v_free_layout(al);
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

					switch (al.second ? al.second->width_policy : SizeFixed)
					{
					case SizeFixed:
						w += al.first->size.x();
						break;
					case SizeFitParent:
						factor += al.second->width_factor;
						break;
					case SizeGreedy:
						factor += al.second->width_factor;
						w += al.second->min_size.x();
						break;
					}
					switch (al.second ? al.second->height_policy : SizeFixed)
					{
					case SizeFixed:
						h = max(al.first->size.y(), h);
						break;
					case SizeFitParent:
						break;
					case SizeGreedy:
						h = max(al.second->min_size.y(), h);
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

					if (al.second)
					{
						if (al.second->width_policy == SizeFitParent)
							al.first->size.x() = w * al.second->width_factor;
						else if (al.second->width_policy == SizeGreedy)
							al.first->size.x() = al.second->min_size.x() + w * al.second->width_factor;
					}
					assert(!al.second || al.second->x_align == AlignxFree);
					al.first->pos.x() = scroll_offset.x() + x;
					x += al.first->size.x() + item_padding;
				}
				for (auto i = n; i < als.size(); i++)
					apply_h_free_layout(als[i], false);
				for (auto i = 0; i < n; i++)
					apply_v_free_layout(als[i], true);
				for (auto i = n; i < als.size(); i++)
					apply_v_free_layout(als[i], false);
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

					switch (al.second ? al.second->width_policy : SizeFixed)
					{
					case SizeFixed:
						w = max(al.first->size.x(), w);
						break;
					case SizeFitParent:
						break;
					case SizeGreedy:
						w = max(al.second->min_size.x(), w);
						break;
					}
					switch (al.second ? al.second->height_policy : SizeFixed)
					{
					case SizeFixed:
						h += al.first->size.y();
						break;
					case SizeFitParent:
						factor += al.second->height_factor;
						break;
					case SizeGreedy:
						factor += al.second->height_factor;
						h += al.second->min_size.y();
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
					apply_h_free_layout(als[i], true);
				for (auto i = n; i < als.size(); i++)
					apply_h_free_layout(als[i], false);
				h = element->size.y() - h;
				if (h > 0.f && factor > 0)
					h /= factor;
				else
					h = 0.f;
				auto y = element->inner_padding[1];
				for (auto i = 0; i < n; i++)
				{
					auto& al = als[i];

					if (al.second)
					{
						if (al.second->height_policy == SizeFitParent)
							al.first->size.y() = h * al.second->height_factor;
						else if (al.second->height_policy == SizeGreedy)
							al.first->size.y() = al.second->min_size.y() + h * al.second->height_factor;
					}
					assert(!al.second || al.second->y_align == AlignyFree);
					al.first->pos.y() = scroll_offset.y() + y;
					y += al.first->size.y() + item_padding;
				}
				for (auto i = n; i < als.size(); i++)
					apply_v_free_layout(als[i], false);
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

						assert(!al.second || (al.second->x_align == AlignxFree && al.second->y_align == AlignyFree));

						al.first->pos.x() = scroll_offset.x() + element->inner_padding[0];
						al.first->pos.y() = scroll_offset.y() + element->inner_padding[1];
					}
					for (auto i = n; i < als.size(); i++)
					{
						apply_h_free_layout(als[i], false);
						apply_v_free_layout(als[i], false);
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

						assert(!al.second || (al.second->width_policy == SizeFixed && al.second->height_policy == SizeFixed));

						lw += al.first->size.x() + item_padding;
						lh = max(al.first->size.y(), lh);

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

						assert(!al.second || (al.second->x_align == AlignxFree && al.second->y_align == AlignyFree));

						al.first->pos.x() = scroll_offset.x() + x;
						al.first->pos.y() = scroll_offset.y() + y;

						x += al.first->size.x() + item_padding;
						lh = max(al.first->size.y(), lh);

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
						apply_h_free_layout(als[i], false);
						apply_v_free_layout(als[i], false);
					}
				}
			}
				break;
			}
		}

		virtual Component* copy() override
		{
			auto copy = new cLayoutPrivate(type);

			copy->item_padding = item_padding;
			copy->width_fit_children = width_fit_children;
			copy->height_fit_children = height_fit_children;

			return copy;
		}
	};

	cLayout* cLayout::create(LayoutType type)
	{
		return new cLayoutPrivate(type);
	}
}
