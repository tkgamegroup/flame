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
			clip = false;
			width_fit_children = true;
			height_fit_children = true;
		}

		void on_add_to_parent()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			aligner = (cAligner*)(entity->find_component(cH("Aligner")));
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
				auto h = 0.f;
				auto div_num = 0U;
				for (auto al : als)
				{
					auto ale = al->element;
					switch (al->width_policy)
					{
					case SizeFixed:
						w += ale->width;
						break;
					case SizeFitLayout:
						div_num++;
						break;
					case SizeGreedy:
						div_num++;
						w += al->min_width;
						break;
					}
					switch (al->height_policy)
					{
					case SizeFixed:
						h = max(ale->height, h);
						break;
					case SizeFitLayout:
						break;
					case SizeGreedy:
						h = max(al->min_height, h);
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
					auto ale = al->element;
					if (al->width_policy == SizeFitLayout)
						ale->width = w;
					else if (al->width_policy == SizeGreedy)
						ale->width = al->min_width + w;
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
					auto ale = al->element;
					if (al->height_policy == SizeFitLayout)
						ale->height = element->height - element->inner_padding[1] - element->inner_padding[3];
					else if (al->height_policy == SizeGreedy)
						ale->height = max(al->min_height, element->height - element->inner_padding[1] - element->inner_padding[3]);
				}

				auto x = element->inner_padding[0];
				for (auto al : als)
				{
					auto ale = al->element;
					assert(al->x_align == AlignxFree);
					ale->x = x;
					x += ale->width + item_padding;
					switch (al->y_align)
					{
					case AlignyFree: case AlignyTop:
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
				auto w = 0.f;
				auto h = element->inner_padding[1];
				auto div_num = 0U;
				for (auto al : als)
				{
					auto ale = al->element;
					switch (al->width_policy)
					{
					case SizeFixed:
						w = max(ale->width, w);
						break;
					case SizeFitLayout:
						break;
					case SizeGreedy:
						w = max(al->min_width, w);
						break;
					}
					switch (al->height_policy)
					{
					case SizeFixed:
						h += ale->height;
						break;
					case SizeFitLayout:
						div_num++;
						break;
					case SizeGreedy:
						div_num++;
						h += al->min_height;
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
					auto ale = al->element;
					if (al->width_policy == SizeFitLayout)
						ale->width = element->width - element->inner_padding[0] - element->inner_padding[2];
					else if (al->width_policy == SizeGreedy)
						ale->width = max(al->min_width, element->width - element->inner_padding[0] - element->inner_padding[2]);
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
					auto ale = al->element;
					if (al->height_policy == SizeFitLayout)
						ale->height = h;
					else if (al->height_policy == SizeGreedy)
						ale->height = al->min_height + h;
				}

				auto y = element->inner_padding[1];
				for (auto al : als)
				{
					auto ale = al->element;
					switch (al->x_align)
					{
					case AlignxFree: case AlignxLeft:
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
					y += ale->height + item_padding;
				}
			}
				break;
			}
		}
	};

	cLayout::~cLayout()
	{
	}

	void cLayout::on_add_to_parent()
	{
		((cLayoutPrivate*)this)->on_add_to_parent();
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
