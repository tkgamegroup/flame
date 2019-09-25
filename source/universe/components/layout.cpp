#include <flame/universe/components/element.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>

namespace flame
{
	struct cLayoutPrivate : cLayout
	{
		cLayoutPrivate(LayoutType$ _type)
		{
			element = nullptr;

			type = _type;
			item_padding = 0.f;
			width_fit_children = type != LayoutFree;
			height_fit_children = type != LayoutFree;
			fence = -1;
			scroll_offset = Vec2f(0.f);

			content_size = Vec2f(0.f);
		}

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			aligner = (cAligner*)(entity->find_component(cH("Aligner")));
		}

		void update()
		{
			std::vector<std::pair<cElement*, cAligner*>> als;
			for (auto i = 0; i < entity->child_count(); i++)
			{
				auto e = entity->child(i);
				if (e->global_visible)
				{
					auto al = (cAligner*)e->find_component(cH("Aligner"));
					als.emplace_back(al ? al->element : (cElement*)entity->child(i)->find_component(cH("Element")), al);
				}
			}

			switch (type)
			{
			case LayoutFree:
			{
				for (auto al : als)
				{
					auto using_padding = al.second ? al.second->using_padding : false;
					auto w = element->size.x() - (using_padding ? element->inner_padding_horizontal() : 0.f);
					auto h = element->size.y() - (using_padding ? element->inner_padding_vertical() : 0.f);
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
					switch (al.second ? al.second->x_align : AlignxFree)
					{
					case AlignxLeft:
						al.first->pos.x() = scroll_offset.x() + (using_padding ? element->inner_padding[0] : 0.f);
						break;
					case AlignxMiddle:
						al.first->pos.x() = scroll_offset.x() + (w - al.first->size.x()) * 0.5f;
						break;
					case AlignxRight:
						al.first->pos.x() = scroll_offset.x() + element->size.x() - (using_padding ? element->inner_padding[2] : 0.f) - al.first->size.x();
						break;
					}
					switch (al.second ? al.second->y_align : AlignyFree)
					{
					case AlignyTop:
						al.first->pos.y() = scroll_offset.y() + (using_padding ? element->inner_padding[1] : 0.f);
						break;
					case AlignyMiddle:
						al.first->pos.y() = scroll_offset.y() + (h - al.first->size.y()) * 0.5f;
						break;
					case AlignyBottom:
						al.first->pos.y() = scroll_offset.y() + element->size.y() - (using_padding ? element->inner_padding[3] : 0.f) - al.first->size.y();
						break;
					}
				}
				if (width_fit_children && !als.empty())
				{
					auto& al = als[0];
					element->size.x() = al.first->size.x() + ((al.second ? al.second->using_padding : false) ? element->inner_padding_horizontal() : 0.f);
				}
				if (height_fit_children && !als.empty())
				{
					auto& al = als[0];
					element->size.y() = al.first->size.y() + ((al.second ? al.second->using_padding : false) ? element->inner_padding_vertical() : 0.f);
				}
			}
				break;
			case LayoutHorizontal:
			{
				auto w = 0.f;
				auto h = 0.f;
				auto factor = 0.f;
				for (auto al : als)
				{
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
				if (!als.empty())
					w -= item_padding;
				content_size = Vec2f(w, h);
				w += element->inner_padding_horizontal();
				h += element->inner_padding_vertical();

				if (width_fit_children)
				{
					if (aligner && aligner->width_policy == SizeGreedy)
					{
						aligner->min_size.x() = w;
						element->size.x() = max(element->size.x(), w);
					}
					else
						element->size.x() = w;
				}
				w = element->size.x() - w;
				if (w > 0.f && factor > 0)
					w /= factor;
				else
					w = 0.f;
				for (auto al : als)
				{
					if (al.second)
					{
						if (al.second->width_policy == SizeFitParent)
							al.first->size.x() = w * al.second->width_factor;
						else if (al.second->width_policy == SizeGreedy)
							al.first->size.x() = al.second->min_size.x() + w * al.second->width_factor;
					}
				}
				if (height_fit_children)
				{
					if (aligner && aligner->height_policy == SizeGreedy)
					{
						aligner->min_size.y() = h;
						element->size.y() = max(element->size.y(), h);
					}
					else
						element->size.y() = h;
				}
				for (auto al : als)
				{
					if (al.second)
					{
						if (al.second->height_policy == SizeFitParent)
							al.first->size.y() = element->size.y() - element->inner_padding_vertical();
						else if (al.second->height_policy == SizeGreedy)
							al.first->size.y() = max(al.second->min_size.y(), element->size.y() - element->inner_padding_vertical());
					}
				}

				auto x = element->inner_padding[0];
				for (auto al : als)
				{
					assert(!al.second || al.second->x_align == AlignxFree);
					al.first->pos.x() = scroll_offset.x() + x;
					x += al.first->size.x() + item_padding;
					switch (al.second ? al.second->y_align : AlignyFree)
					{
					case AlignyFree: case AlignyTop:
						al.first->pos.y() = scroll_offset.y() + element->inner_padding[1];
						break;
					case AlignyMiddle:
						al.first->pos.y() = scroll_offset.y() + (element->size.y() - element->inner_padding_vertical() - al.first->size.y()) * 0.5f;
						break;
					case AlignyBottom:
						al.first->pos.y() = scroll_offset.y() + element->size.y() - element->inner_padding[3] - al.first->size.y();
						break;
					}
				}
			}
				break;
			case LayoutVertical:
			{
				auto w = 0.f;
				auto h = 0.f;
				auto factor = 0.f;
				for (auto al : als)
				{
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
				if (!als.empty())
					h -= item_padding;
				content_size = Vec2f(w, h);
				w += element->inner_padding_horizontal();
				h += element->inner_padding_vertical();

				if (width_fit_children)
				{
					if (aligner && aligner->width_policy == SizeGreedy)
					{
						aligner->min_size.x() = w;
						element->size.x() = max(element->size.x(), w);
					}
					else
						element->size.x() = w;
				}
				for (auto al : als)
				{
					if (al.second)
					{
						if (al.second->width_policy == SizeFitParent)
							al.first->size.x() = element->size.x() - element->inner_padding_horizontal();
						else if (al.second->width_policy == SizeGreedy)
							al.first->size.x() = max(al.second->min_size.x(), element->size.x() - element->inner_padding_horizontal());
					}
				}
				if (height_fit_children)
				{
					if (aligner && aligner->height_policy == SizeGreedy)
					{
						aligner->min_size.y() = h;
						element->size.y() = max(element->size.y(), h);
					}
					else
						element->size.y() = h;
				}
				h = element->size.y() - h;
				if (h > 0.f && factor > 0)
					h /= factor;
				else
					h = 0.f;
				for (auto al : als)
				{
					if (al.second)
					{
						if (al.second->height_policy == SizeFitParent)
							al.first->size.y() = h * al.second->height_factor;
						else if (al.second->height_policy == SizeGreedy)
							al.first->size.y() = al.second->min_size.y() + h * al.second->height_factor;
					}
				}
				auto y = element->inner_padding[1];
				for (auto al : als)
				{
					switch (al.second ? al.second->x_align : AlignxFree)
					{
					case AlignxFree: case AlignxLeft:
						al.first->pos.x() = scroll_offset.x() + element->inner_padding[0];
						break;
					case AlignxMiddle:
						al.first->pos.x() = scroll_offset.x() + (element->size.x() - element->inner_padding_horizontal() - al.first->size.x()) * 0.5f;
						break;
					case AlignxRight:
						al.first->pos.x() = scroll_offset.x() + element->size.x() - element->inner_padding[2] - al.first->size.x();
						break;
					}
					assert(!al.second || al.second->y_align == AlignyFree);
					al.first->pos.y() = scroll_offset.y() + y;
					y += al.first->size.y() + item_padding;
				}
			}
				break;
			}
		}

		Component* copy()
		{
			auto copy = new cLayoutPrivate(type);

			copy->item_padding = item_padding;
			copy->width_fit_children = width_fit_children;
			copy->height_fit_children = height_fit_children;

			return copy;
		}
	};

	void cLayout::start()
	{
		((cLayoutPrivate*)this)->start();
	}

	void cLayout::update()
	{
		((cLayoutPrivate*)this)->update();
	}

	Component* cLayout::copy()
	{
		return ((cLayoutPrivate*)this)->copy();
	}

	cLayout* cLayout::create(LayoutType$ type)
	{
		return new cLayoutPrivate(type);
	}
}
