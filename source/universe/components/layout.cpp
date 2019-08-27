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
				for (auto al : als)
				{
					switch (al.second ? al.second->width_policy : SizeFixed)
					{
					case SizeFitLayout:
						al.first->width = element->width;
						break;
					case SizeGreedy:
						if (element->width > al.second->min_width)
							al.first->width = element->width;
						break;
					}
					switch (al.second ? al.second->height_policy : SizeFixed)
					{
					case SizeFitLayout:
						al.first->height = element->height;
						break;
					case SizeGreedy:
						if (element->height > al.second->min_height)
							al.first->height = element->height;
						break;
					}
					switch (al.second ? al.second->x_align : AlignxFree)
					{
					case AlignxLeft:
						al.first->x = 0;
						break;
					case AlignxMiddle:
						al.first->x = (element->width - al.first->width) * 0.5f;
						break;
					case AlignxRight:
						al.first->x = element->width - al.first->width;
						break;
					}
					switch (al.second ? al.second->y_align : AlignyFree)
					{
					case AlignyTop:
						al.first->y = 0;
						break;
					case AlignyMiddle:
						al.first->y = (element->height - al.first->height) * 0.5f;
						break;
					case AlignyBottom:
						al.first->y = element->height - al.first->height;
						break;
					}
				}
				break;
			case LayoutHorizontal:
			{
				auto w = element->inner_padding[0];
				auto h = 0.f;
				auto div_num = 0U;
				for (auto al : als)
				{
					switch (al.second ? al.second->width_policy : SizeFixed)
					{
					case SizeFixed:
						w += al.first->width;
						break;
					case SizeFitLayout:
						div_num++;
						break;
					case SizeGreedy:
						div_num++;
						w += al.second->min_width;
						break;
					}
					switch (al.second ? al.second->height_policy : SizeFixed)
					{
					case SizeFixed:
						h = max(al.first->height, h);
						break;
					case SizeFitLayout:
						break;
					case SizeGreedy:
						h = max(al.second->min_height, h);
						break;
					}
					w += item_padding;
				}
				if (!als.empty())
					w -= item_padding;
				w += element->inner_padding[2];
				h += element->inner_padding[1] + element->inner_padding[3];

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
				if (w > 0.f && div_num > 0)
					w /= div_num;
				else
					w = 0.f;
				for (auto al : als)
				{
					if (al.second)
					{
						if (al.second->width_policy == SizeFitLayout)
							al.first->width = w;
						else if (al.second->width_policy == SizeGreedy)
							al.first->width = al.second->min_width + w;
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
						if (al.second->height_policy == SizeFitLayout)
							al.first->height = element->height - element->inner_padding[1] - element->inner_padding[3];
						else if (al.second->height_policy == SizeGreedy)
							al.first->height = max(al.second->min_height, element->height - element->inner_padding[1] - element->inner_padding[3]);
					}
				}

				auto x = element->inner_padding[0];
				for (auto al : als)
				{
					assert(!al.second || al.second->x_align == AlignxFree);
					al.first->x = x;
					x += al.first->width + item_padding;
					switch (al.second ? al.second->y_align : AlignyFree)
					{
					case AlignyFree: case AlignyTop:
						al.first->y = element->inner_padding[1];
						break;
					case AlignyMiddle:
						al.first->y = (element->height - element->inner_padding[1] - element->inner_padding[3] - al.first->height) * 0.5f;
						break;
					case AlignyBottom:
						al.first->y = element->height - element->inner_padding[3] - al.first->height;
						break;
					}
				}
			}
				break;
			case LayoutVertical:
			{
				auto w = 0.f;
				auto h = element->inner_padding[1];
				auto div_num = 0U;
				for (auto al : als)
				{
					switch (al.second ? al.second->width_policy : SizeFixed)
					{
					case SizeFixed:
						w = max(al.first->width, w);
						break;
					case SizeFitLayout:
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
					case SizeFitLayout:
						div_num++;
						break;
					case SizeGreedy:
						div_num++;
						h += al.second->min_height;
						break;
					}
					h += item_padding;
				}
				if (!als.empty())
					h -= item_padding;
				w += element->inner_padding[0] + element->inner_padding[2];
				h += element->inner_padding[3];

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
						if (al.second->width_policy == SizeFitLayout)
							al.first->width = element->width - element->inner_padding[0] - element->inner_padding[2];
						else if (al.second->width_policy == SizeGreedy)
							al.first->width = max(al.second->min_width, element->width - element->inner_padding[0] - element->inner_padding[2]);
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
				if (h > 0.f && div_num > 0)
					h /= div_num;
				else
					h = 0.f;
				for (auto al : als)
				{
					if (al.second)
					{
						if (al.second->height_policy == SizeFitLayout)
							al.first->height = h;
						else if (al.second->height_policy == SizeGreedy)
							al.first->height = al.second->min_height + h;
					}
				}

				auto y = element->inner_padding[1];
				for (auto al : als)
				{
					switch (al.second ? al.second->x_align : AlignxFree)
					{
					case AlignxFree: case AlignxLeft:
						al.first->x = element->inner_padding[0];
						break;
					case AlignxMiddle:
						al.first->x = (element->width - element->inner_padding[0] - element->inner_padding[2] - al.first->width) * 0.5f;
						break;
					case AlignxRight:
						al.first->x = element->width - element->inner_padding[2] - al.first->width;
						break;
					}
					assert(!al.second || al.second->y_align == AlignyFree);
					al.first->y = y;
					y += al.first->height + item_padding;
				}
			}
				break;
			}
		}
	};

	cLayout::~cLayout()
	{
	}

	void cLayout::start()
	{
		((cLayoutPrivate*)this)->start();
	}

	void cLayout::update()
	{
		((cLayoutPrivate*)this)->update();
	}

	cLayout* cLayout::create()
	{
		return new cLayoutPrivate();
	}
}
