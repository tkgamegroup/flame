#include "../entity_private.h"
#include "element_private.h"
#include "layout_private.h"

namespace flame
{
	void cLayoutPrivate::set_type(ElementLayoutType _type)
	{
		if (type == _type)
			return;
		type = _type;
		element->mark_transform_dirty();
		data_changed("type"_h);
	}

	void cLayoutPrivate::set_padding(const vec4& padding)
	{
		if (this->padding == padding)
			return;
		this->padding = padding;
		element->mark_transform_dirty();
		data_changed("padding"_h);
	}

	void cLayoutPrivate::set_item_spacing(float spacing)
	{
		if (item_spacing == spacing)
			return;
		item_spacing = spacing;
		element->mark_transform_dirty();
		data_changed("item_spacing"_h);
	}

	void cLayoutPrivate::set_columns(uint _columns)
	{
		_columns = max(1U, _columns);
		if (columns == _columns)
			return;
		columns = _columns;
		element->mark_transform_dirty();
		data_changed("columns"_h);
	}

	void cLayoutPrivate::set_auto_width(bool _auto_width)
	{
		if (auto_width == _auto_width)
			return;
		auto_width = _auto_width;
		element->mark_transform_dirty();
		data_changed("auto_width"_h);
	}

	void cLayoutPrivate::set_auto_height(bool _auto_height)
	{
		if (auto_height == _auto_height)
			return;
		auto_height = _auto_height;
		element->mark_transform_dirty();
		data_changed("auto_height"_h);
	}

	void cLayoutPrivate::on_active()
	{
		element->mark_transform_dirty();
	}

	void cLayoutPrivate::update_layout()
	{
		float x = padding.x;				float y = padding.y;
		float w = padding.x + padding.z;	float h = padding.y + padding.w;

		switch (type)
		{
			case ElementLayoutVertical:
				for (auto& c : entity->children)
				{
					if (!c->global_enable)
						continue;
					if (auto element = c->element(); element)
					{
						if (element->horizontal_alignment == ElementAlignNone)
						{
							element->set_x(x);
							w = max(w, element->ext.x + padding.x + padding.z);
						}
						if (element->vertical_alignment == ElementAlignNone)
						{
							element->set_y(y);
							y += item_spacing;
							y += element->ext.y;
							h = max(h, y + padding.w);
						}
					}
				}
				break;
			case ElementLayoutHorizontal:
				for (auto& c : entity->children)
				{
					if (!c->global_enable)
						continue;
					if (auto element = c->element(); element)
					{
						if (element->horizontal_alignment == ElementAlignNone)
						{
							element->set_x(x);
							x += item_spacing;
							x += element->ext.x;
							w = max(w, x + padding.z);
						}
						if (element->vertical_alignment == ElementAlignNone)
						{
							element->set_y(y);
							h = max(h, element->ext.y + padding.y + padding.w);
						}
					}
				}
				break;
			case ElementLayoutGrid:
			{
				std::vector<float> column_widths(columns);
				std::vector<float> column_offsets(columns);
				auto idx = 0;
				for (auto& c : entity->children)
				{
					if (!c->global_enable)
						continue;
					if (auto element = c->element(); element)
					{
						if (element->horizontal_alignment == ElementAlignNone && element->vertical_alignment == ElementAlignNone)
						{
							// we first log the max width of each column
							auto& column_width = column_widths[idx % columns];
							column_width = max(column_width, element->ext.x);
						}
						idx++;
					}
				}

				// we then calculate the offset of each column and the width of the layout
				column_offsets[0] = padding.x;
				for (auto i = 1; i < columns; i++)
					column_offsets[i] = column_offsets[i - 1] + column_widths[i - 1] + item_spacing;
				w = column_offsets.back() + column_widths.back() + padding.z;

				idx = 0;
				auto row_height = 0.f;
				for (auto& c : entity->children)
				{
					if (!c->global_enable)
						continue;
					if (auto element = c->element(); element)
					{
						if (element->horizontal_alignment == ElementAlignNone && element->vertical_alignment == ElementAlignNone)
						{
							auto column = idx % columns;
							if (idx > 0 && column == 0) // each row except the first one
							{
								y += item_spacing + row_height;
								row_height = 0.f;
							}
							element->set_pos(vec2(column_offsets[column], y));
							row_height = max(row_height, element->ext.y);
						}
						idx++;
					}
				}

				h = y + row_height + padding.w;
			}
				break;
			case ElementLayoutCircle:
			{
				auto max_ext = vec2(0.f);
				for (auto& c : entity->children)
				{
					if (!c->global_enable)
						continue;
					if (auto element = c->element(); element)
					{
						if (element->horizontal_alignment == ElementAlignNone && element->vertical_alignment == ElementAlignNone)
							max_ext = max(max_ext, element->ext);
					}
				}

				auto hf_columns = columns >> 1;
				auto idx = 0;
				for (auto& c : entity->children)
				{
					if (!c->global_enable)
						continue;
					if (auto element = c->element(); element)
					{
						if (element->horizontal_alignment == ElementAlignNone && element->vertical_alignment == ElementAlignNone)
						{
							int x, y;
							bool ok = true;
							while (true)
							{
								x = (idx % columns) - hf_columns;
								y = (idx / columns) - hf_columns;
								if (x * x + y * y > hf_columns * hf_columns)
									idx++;
								else
									break;
								if (idx >= columns * columns)
								{
									ok = false;
									break;
								}
							}
							if (!ok)
								break;

							x += hf_columns;
							y += hf_columns;
							element->set_pos(vec2(x * max_ext.x + x * item_spacing, y * max_ext.y + y * item_spacing));
							idx++;
						}
					}
				}

				w = columns * max_ext.x + (columns - 1) * item_spacing;
				h = columns * max_ext.y + (columns - 1) * item_spacing;
			}
				break;
		}

		if (auto_width)
			element->set_w(w);
		if (auto_height)
			element->set_h(h);
	}

	struct cLayoutCreate : cLayout::Create
	{
		cLayoutPtr operator()(EntityPtr) override
		{
			return new cLayoutPrivate();
		}
	}cLayout_create;
	cLayout::Create& cLayout::create = cLayout_create;
}
