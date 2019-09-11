#include <flame/universe/components/element.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>

namespace flame
{
	struct cLayoutPrivate : cLayout
	{
		cLayoutPrivate()
		{
			element = nullptr;

			type = LayoutFree;
			item_padding = 0.f;
			width_fit_children = true;
			height_fit_children = true;
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
					auto upifl = al.second ? al.second->using_padding_in_free_layout : false;
					auto w = element->width - (upifl ? element->inner_padding_horizontal() : 0.f);
					auto h = element->height - (upifl ? element->inner_padding_vertical() : 0.f);
					if (upifl)
						int cut = 1;
					switch (al.second ? al.second->width_policy : SizeFixed)
					{
					case SizeFitParent:
						al.first->width = w;
						break;
					case SizeGreedy:
						if (w > al.second->min_width)
							al.first->width = w;
						break;
					}
					switch (al.second ? al.second->height_policy : SizeFixed)
					{
					case SizeFitParent:
						al.first->height = h;
						break;
					case SizeGreedy:
						if (h > al.second->min_height)
							al.first->height = h;
						break;
					}
					switch (al.second ? al.second->x_align : AlignxFree)
					{
					case AlignxLeft:
						al.first->x = scroll_offset.x() + (upifl ? element->inner_padding[0] : 0.f);
						break;
					case AlignxMiddle:
						al.first->x = scroll_offset.x() + (w - al.first->width) * 0.5f;
						break;
					case AlignxRight:
						al.first->x = scroll_offset.x() + element->width - (upifl ? element->inner_padding[2] : 0.f) - al.first->width;
						break;
					}
					switch (al.second ? al.second->y_align : AlignyFree)
					{
					case AlignyTop:
						al.first->y = scroll_offset.y() + (upifl ? element->inner_padding[1] : 0.f);
						break;
					case AlignyMiddle:
						al.first->y = scroll_offset.y() + (h - al.first->height) * 0.5f;
						break;
					case AlignyBottom:
						al.first->y = scroll_offset.y() + element->height - (upifl ? element->inner_padding[3] : 0.f) - al.first->height;
						break;
					}
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
						w += al.first->width;
						break;
					case SizeFitParent:
						factor += al.second->width_factor;
						break;
					case SizeGreedy:
						factor += al.second->width_factor;
						w += al.second->min_width;
						break;
					}
					switch (al.second ? al.second->height_policy : SizeFixed)
					{
					case SizeFixed:
						h = max(al.first->height, h);
						break;
					case SizeFitParent:
						break;
					case SizeGreedy:
						h = max(al.second->min_height, h);
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
						aligner->min_width = w;
						element->width = max(element->width, w);
					}
					else
						element->width = w;
				}
				w = element->width - w;
				if (w > 0.f && factor > 0)
					w /= factor;
				else
					w = 0.f;
				for (auto al : als)
				{
					if (al.second)
					{
						if (al.second->width_policy == SizeFitParent)
							al.first->width = w * al.second->width_factor;
						else if (al.second->width_policy == SizeGreedy)
							al.first->width = al.second->min_width + w * al.second->width_factor;
					}
				}
				if (height_fit_children)
				{
					if (aligner && aligner->height_policy == SizeGreedy)
					{
						aligner->min_height = h;
						element->height = max(element->height, h);
					}
					else
						element->height = h;
				}
				for (auto al : als)
				{
					if (al.second)
					{
						if (al.second->height_policy == SizeFitParent)
							al.first->height = element->height - element->inner_padding_vertical();
						else if (al.second->height_policy == SizeGreedy)
							al.first->height = max(al.second->min_height, element->height - element->inner_padding_vertical());
					}
				}

				auto x = element->inner_padding[0];
				for (auto al : als)
				{
					assert(!al.second || al.second->x_align == AlignxFree);
					al.first->x = scroll_offset.x() + x;
					x += al.first->width + item_padding;
					switch (al.second ? al.second->y_align : AlignyFree)
					{
					case AlignyFree: case AlignyTop:
						al.first->y = scroll_offset.y() + element->inner_padding[1];
						break;
					case AlignyMiddle:
						al.first->y = scroll_offset.y() + (element->height - element->inner_padding_vertical() - al.first->height) * 0.5f;
						break;
					case AlignyBottom:
						al.first->y = scroll_offset.y() + element->height - element->inner_padding[3] - al.first->height;
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
						w = max(al.first->width, w);
						break;
					case SizeFitParent:
						break;
					case SizeGreedy:
						w = max(al.second->min_width, w);
						break;
					}
					switch (al.second ? al.second->height_policy : SizeFixed)
					{
					case SizeFixed:
						h += al.first->height;
						break;
					case SizeFitParent:
						factor += al.second->height_factor;
						break;
					case SizeGreedy:
						factor += al.second->height_factor;
						h += al.second->min_height;
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
						aligner->min_width = w;
						element->width = max(element->width, w);
					}
					else
						element->width = w;
				}
				for (auto al : als)
				{
					if (al.second)
					{
						if (al.second->width_policy == SizeFitParent)
							al.first->width = element->width - element->inner_padding_horizontal();
						else if (al.second->width_policy == SizeGreedy)
							al.first->width = max(al.second->min_width, element->width - element->inner_padding_horizontal());
					}
				}
				if (height_fit_children)
				{
					if (aligner && aligner->height_policy == SizeGreedy)
					{
						aligner->min_height = h;
						element->height = max(element->height, h);
					}
					else
						element->height = h;
				}
				h = element->height - h;
				if (h > 0.f && factor > 0)
					h /= factor;
				else
					h = 0.f;
				for (auto al : als)
				{
					if (al.second)
					{
						if (al.second->height_policy == SizeFitParent)
							al.first->height = h * al.second->height_factor;
						else if (al.second->height_policy == SizeGreedy)
							al.first->height = al.second->min_height + h * al.second->height_factor;
					}
				}
				auto y = element->inner_padding[1];
				for (auto al : als)
				{
					switch (al.second ? al.second->x_align : AlignxFree)
					{
					case AlignxFree: case AlignxLeft:
						al.first->x = scroll_offset.x() + element->inner_padding[0];
						break;
					case AlignxMiddle:
						al.first->x = scroll_offset.x() + (element->width - element->inner_padding_horizontal() - al.first->width) * 0.5f;
						break;
					case AlignxRight:
						al.first->x = scroll_offset.x() + element->width - element->inner_padding[2] - al.first->width;
						break;
					}
					assert(!al.second || al.second->y_align == AlignyFree);
					al.first->y = scroll_offset.y() + y;
					y += al.first->height + item_padding;
				}
			}
				break;
			}
		}

		Component* copy()
		{
			auto copy = new cLayoutPrivate();

			copy->type = type;
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

	cLayout* cLayout::create()
	{
		return new cLayoutPrivate();
	}
}
