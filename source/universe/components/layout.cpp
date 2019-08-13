#include <flame/universe/components/element.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>

namespace flame
{
	struct cLayoutPrivate : cLayout
	{
		cLayoutPrivate(Entity* e) :
			cLayout(e)
		{
			element = (cElement*)(e->find_component(cH("Element")));
			assert(element);

			type = LayoutFree;
			item_padding = 0.f;
			clip = false;
			fit_children = true;
		}

		void update()
		{
			std::vector<cAligner*> als;
			als.resize(entity->child_count());
			for (auto i = 0; i < entity->child_count(); i++)
			{
				auto al = (cAligner*)entity->child(i)->find_component(cH("Aligner"));
				assert(al);
				als[i] = al;
			}

			switch (type)
			{
			case LayoutFree:
				for (auto al : als)
				{
					auto ale = al->element;
					switch (al->width_policy)
					{
					case SizeFitLayout:
						ale->width = element->width;
						break;
					case SizeGreedy:
						if (element->width > al->min_width)
							ale->width = element->width;
						break;
					}
					switch (al->height_policy)
					{
					case SizeFitLayout:
						ale->height = element->height;
						break;
					case SizeGreedy:
						if (element->height > al->min_height)
							ale->height = element->height;
						break;
					}
					switch (al->x_align)
					{
					case AlignxLeft:
						ale->x = 0;
						break;
					case AlignxMiddle:
						ale->x = (element->width - ale->width) * 0.5f;
						break;
					case AlignxRight:
						ale->x = element->width - ale->width;
						break;
					}
					switch (al->y_align)
					{
					case AlignyTop:
						ale->y = 0;
						break;
					case AlignyMiddle:
						ale->y = (element->height - ale->height) * 0.5f;
						break;
					case AlignyBottom:
						ale->y = element->height - ale->height;
						break;
					}
				}
				break;
			case LayoutHorizontal:
			{
				auto w = element->inner_padding[0];
				auto h = element->inner_padding[1];
				auto div_num = 0U;
				for (auto al : als)
				{
					auto ale = al->element;
					if (al->width_policy == SizeFitLayout || al->width_policy == SizeGreedy)
					{
						assert(!fit_children);
						div_num++;
						if (al->width_policy == SizeGreedy)
							w += al->min_width;
					}
					else
						w += ale->width;
					if (al->height_policy == SizeFitLayout || al->height_policy == SizeGreedy)
					{
						assert(!fit_children);
						if (al->height_policy == SizeGreedy)
							h += al->min_height;
					}
					else
						h += ale->height;
					w += item_padding;
				}
				w += element->inner_padding[2];
				h += element->inner_padding[3];

				if (fit_children)
				{
					element->width = w;
					element->height = h;
				}
				else
				{
					w -= element->width;
					if (w > 0.f && div_num > 0)
						w /= div_num;
					else
						w = 0.f;
					h -= element->height;
					if (h < 0.f)
						h = 0.f;
					for (auto al : als)
					{
						auto ale = al->element;
						if (al->width_policy == SizeFitLayout)
							ale->width = w;
						else if (al->width_policy == SizeGreedy)
							ale->width = al->min_width + w;
						if (al->height_policy == SizeFitLayout)
							ale->height = h;
						else if (al->height_policy == SizeGreedy)
							ale->height = al->min_height + h;
					}
				}

				auto x = element->inner_padding[0];
				for (auto al : als)
				{
					auto ale = al->element;
					assert(al->x_align == AlignxFree);
					ale->x = x;
					x += ale->width;
					switch (al->y_align)
					{
					case AlignyTop:
						ale->y = element->inner_padding[1];
						break;
					case AlignyMiddle:
						ale->y = (element->height - element->inner_padding[1] - element->inner_padding[3] - ale->height) * 0.5f;
						break;
					case AlignyBottom:
						ale->y = element->height - element->inner_padding[3] - ale->height;
						break;
					}
				}
			}
				break;
			case LayoutVertical:
			{
				auto w = element->inner_padding[0];
				auto h = element->inner_padding[1];
				auto div_num = 0U;
				for (auto al : als)
				{
					auto ale = al->element;
					if (al->width_policy == SizeFitLayout || al->width_policy == SizeGreedy)
					{
						assert(!fit_children);
						if (al->width_policy == SizeGreedy)
							w += al->min_width;
					}
					else
						w += ale->width;
					if (al->height_policy == SizeFitLayout || al->height_policy == SizeGreedy)
					{
						assert(!fit_children);
						div_num++;
						if (al->height_policy == SizeGreedy)
							h += al->min_height;
					}
					else
						h += ale->height;
					h += item_padding;
				}
				w += element->inner_padding[2];
				h += element->inner_padding[3];

				if (fit_children)
				{
					element->width = w;
					element->height = h;
				}
				else
				{
					w -= element->width;
					if (w < 0.f)
						w = 0.f;
					h -= element->height;
					if (h > 0.f && div_num > 0)
						h /= div_num;
					else
						h = 0.f;
					for (auto al : als)
					{
						auto ale = al->element;
						if (al->width_policy == SizeFitLayout)
							ale->width = w;
						else if (al->width_policy == SizeGreedy)
							ale->width = al->min_width + w;
						if (al->height_policy == SizeFitLayout)
							ale->height = h;
						else if (al->height_policy == SizeGreedy)
							ale->height = al->min_height + h;
					}
				}

				auto y = element->inner_padding[1];
				for (auto al : als)
				{
					auto ale = al->element;
					switch (al->x_align)
					{
					case AlignxLeft:
						ale->x = element->inner_padding[0];
						break;
					case AlignxMiddle:
						ale->x = (element->width - element->inner_padding[0] - element->inner_padding[2] - ale->width) * 0.5f;
						break;
					case AlignxRight:
						ale->x = element->width - element->inner_padding[2] - ale->width;
						break;
					}
					assert(al->y_align == AlignyFree);
					ale->y = y;
					y += ale->height;
				}
			}
				break;
			}
		}
	};

	cLayout::cLayout(Entity* e) :
		Component("Layout", e)
	{
	}

	cLayout::~cLayout()
	{
	}

	void cLayout::update()
	{
		((cLayoutPrivate*)this)->update();
	}

	cLayout* cLayout::create(Entity* e)
	{
		return new cLayoutPrivate(e);
	}
}
