#include <flame/foundation/window.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>

namespace flame
{
	struct cLayoutPrivate : cLayout
	{
		Window* w;

		cLayoutPrivate(Entity* e, Window* w) :
			cLayout(e),
			w(w)
		{
			element = (cElement*)(e->find_component(cH("Element")));
			assert(element);

			type = LayoutFree;
			item_padding = 0.f;
			clip = false;
			fit_children = false;
		}

		void update()
		{
			if (w)
			{
				element->width = w->size.x();
				element->height = w->size.y();
			}

			/*
		switch (layout_type$)
		{
		case LayoutVertical:
		{
			if (size_policy_hori$ == SizeFitChildren || size_policy_hori$ == SizeGreedy)
			{
				auto width = 0;
				for (auto i_c = 0; i_c < children_1$.size; i_c++)
				{
					auto c = children_1$[i_c];

					if (!c->visible$)
						continue;

					width = max(width, c->size$.x);
				}

				width += inner_padding$[0] + inner_padding$[2];
				if (size_policy_hori$ == SizeFitChildren)
					set_width(width, this);
				else if (size_policy_hori$ == SizeGreedy)
				{
					if (width > size$.x)
						set_width(width, this);
				}
			}
			if (size_policy_vert$ == SizeFitChildren || size_policy_vert$ == SizeGreedy)
			{
				auto height = 0;
				for (auto i_c = 0; i_c < children_1$.size; i_c++)
				{
					auto c = children_1$[i_c];

					if (!c->visible$)
						continue;

					height += c->size$.y + item_padding$;
				}
				height -= item_padding$;
				content_size = height;

				height += inner_padding$[1] + inner_padding$[3];
				if (size_policy_vert$ == SizeFitChildren)
					set_height(height, this);
				else if (size_policy_vert$ == SizeGreedy)
				{
					if (height > size$.y)
						set_height(height, this);
				}
			}
			else if (size_policy_vert$ == SizeFitLayout)
			{
				auto cnt = 0;
				auto height = 0;
				for (auto i_c = 0; i_c < children_1$.size; i_c++)
				{
					auto c = children_1$[i_c];

					if (!c->visible$)
						continue;

					if (c->size_policy_vert$ == SizeFitLayout)
						cnt++;
					else
						height += c->size$.y;
					height += item_padding$;
				}
				height -= item_padding$;
				content_size = height;

				height = max(0, (size$.y - inner_padding$[1] - inner_padding$[3] - height) / cnt);

				for (auto i_c = 0; i_c < children_1$.size; i_c++)
				{
					auto c = children_1$[i_c];

					if (!c->visible$)
						continue;

					if (c->size_policy_vert$ == SizeFitLayout)
						c->set_height(height, this);
				}
			}

			auto width = size$.x - inner_padding$[0] - inner_padding$[2];
			auto height = size$.y - inner_padding$[1] - inner_padding$[3];

			auto content_size = get_content_size();
			scroll_offset$ = content_size > height ? clamp((float)scroll_offset$, height - content_size, 0.f) : 0.f;

			auto y = inner_padding$[1] + scroll_offset$;

			for (auto i_c = 0; i_c < children_1$.size; i_c++)
			{
				auto c = children_1$[i_c];

				if (!c->visible$)
					continue;

				if (c->size_policy_hori$ == SizeFitLayout)
					c->set_width(width, this);
				else if (c->size_policy_hori$ == SizeGreedy)
				{
					if (width > c->size$.x)
						c->set_width(width, this);
				}

				switch (c->align$)
				{
				case AlignLittleEnd:
					c->pos$ = Vec2(inner_padding$[0] + c->layout_padding$, y);
					break;
				case AlignLargeEnd:
					c->pos$ = Vec2(size$.x - inner_padding$[2] - c->size$.x - c->layout_padding$, y);
					break;
				case AlignMiddle:
					c->pos$ = Vec2((size$.x - inner_padding$[0] - inner_padding$[2] - c->size$.x) * 0.5f + inner_padding$[0], y);
					break;
				}

				y += c->size$.y + item_padding$;
			}
		}
			break;
		case LayoutHorizontal:
		{
			if (size_policy_hori$ == SizeFitChildren || size_policy_hori$ == SizeGreedy)
			{
				auto width = 0;
				for (auto i_c = 0; i_c < children_1$.size; i_c++)
				{
					auto c = children_1$[i_c];

					if (!c->visible$)
						continue;

					width += c->size$.x + item_padding$;
				}
				width -= item_padding$;
				content_size = width;

				width += inner_padding$[0] + inner_padding$[2];
				if (size_policy_hori$ == SizeFitChildren)
					set_width(width, this);
				else if (size_policy_hori$ == SizeGreedy)
				{
					if (width > size$.x)
						set_width(width, this);
				}
			}
			else if (size_policy_hori$ == SizeFitLayout)
			{
				auto cnt = 0;
				auto width = 0;
				for (auto i_c = 0; i_c < children_1$.size; i_c++)
				{
					auto c = children_1$[i_c];

					if (!c->visible$)
						continue;

					if (c->size_policy_hori$ == SizeFitLayout)
						cnt++;
					else
						width += c->size$.x;
					width += item_padding$;
				}
				width -= item_padding$;
				content_size = width;

				width = max(0, (size$.x - inner_padding$[0] - inner_padding$[2] - width) / cnt);

				for (auto i_c = 0; i_c < children_1$.size; i_c++)
				{
					auto c = children_1$[i_c];

					if (!c->visible$)
						continue;

					if (c->size_policy_hori$ == SizeFitLayout)
						c->set_width(width, this);
				}
			}
			if (size_policy_vert$ == SizeFitChildren || size_policy_vert$ == SizeGreedy)
			{
				auto height = 0;
				for (auto i_c = 0; i_c < children_1$.size; i_c++)
				{
					auto c = children_1$[i_c];

					if (!c->visible$)
						continue;

					height = max(height, c->size$.y);
				}

				height += inner_padding$[1] + inner_padding$[3];
				if (size_policy_vert$ == SizeFitChildren)
					set_height(height, this);
				else if (size_policy_vert$ == SizeGreedy)
				{
					if (height > size$.y)
						set_height(height, this);
				}
			}

			auto height = size$.y - inner_padding$[1] - inner_padding$[3];
			auto x = inner_padding$[0];
			for (auto i_c = 0; i_c < children_1$.size; i_c++)
			{
				auto c = children_1$[i_c];

				if (!c->visible$)
					continue;

				if (c->size_policy_vert$ == SizeFitLayout)
					c->set_height(height, this);
				else if (c->size_policy_vert$ == SizeGreedy)
				{
					if (height > c->size$.y)
						c->set_height(height, this);
				}

				switch (c->align$)
				{
				case AlignLittleEnd:
					c->pos$ = Vec2(x, inner_padding$[1] + c->layout_padding$);
					break;
				case AlignLargeEnd:
					c->pos$ = Vec2(x, size$.y - inner_padding$[3] - c->size$.y - c->layout_padding$);
					break;
				case AlignMiddle:
					c->pos$ = Vec2(x, (size$.y - inner_padding$[1] - inner_padding$[3] - c->size$.y) * 0.5f + inner_padding$[2]);
					break;
				}

				x += c->size$.x + item_padding$;
			}
		}
			break;
		case LayoutGrid:
		{
			auto pos = Vec2(inner_padding$[0], inner_padding$[1]);

			auto cnt = 0;
			auto line_height = 0.f;
			auto max_width = 0.f;
			for (auto i_c = 0; i_c < children_1$.size; i_c++)
			{
				auto c = children_1$[i_c];

				c->pos$ = pos;
				line_height = max(line_height, c->size$.y);

				pos.x += c->size$.x + item_padding$;
				max_width = max(max_width, pos.x);
				cnt++;
				if (cnt >= grid_hori_count$)
				{
					pos.x = inner_padding$[0];
					pos.y += line_height + item_padding$;
					cnt = 0;
					line_height = 0.f;
				}
			}

			if (size_policy_hori$ == SizeFitChildren)
				set_width(max(max_width - item_padding$, 0.f), this);
			if (size_policy_vert$ == SizeFitChildren)
				set_height(max(pos.y - item_padding$, 0.f), this);
		}
			break;
		}
			*/

			switch (type)
			{
			case LayoutFree:
				for (auto i = 0; i < entity->child_count(); i++)
				{
					auto e = entity->child(i);
					if (!e->global_visible)
						continue;
					auto al = (cAligner*)e->find_component(cH("Aligner"));
					auto ale = al->element;
					if (al->width_greedy)
						ale->width = element->width;
					if (al->height_greedy)
						ale->height = element->height;
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
				for (auto i = 0; i < entity->child_count(); i++)
				{
					auto al = (cAligner*)entity->child(i)->find_component(cH("Aligner"));
					auto ale = al->element;

				}
				break;
			case LayoutVertical:

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

	cLayout* cLayout::create(Entity* e, Window* w)
	{
		return new cLayoutPrivate(e, w);
	}
}
