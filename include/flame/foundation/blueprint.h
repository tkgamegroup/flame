// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <flame/foundation/foundation.h>

namespace flame
{
	struct EnumInfo;
	struct VaribleInfo;
	struct UDT;

	struct BP
	{
		struct Node;
		struct Input;
		struct Output;

		struct Item
		{
			FLAME_FOUNDATION_EXPORTS Input *parent_i() const; // null or its parent is input
			FLAME_FOUNDATION_EXPORTS Output *parent_o() const; // null or its parent is output
			FLAME_FOUNDATION_EXPORTS CommonData &data();

			FLAME_FOUNDATION_EXPORTS Item *link() const;
			FLAME_FOUNDATION_EXPORTS bool set_link(Item *target); // well, it is vaild for input's item only

			FLAME_FOUNDATION_EXPORTS String get_address() const; // node_id.varible_name.item_index (item_index is default for 0)
		};

		struct Input
		{
			FLAME_FOUNDATION_EXPORTS Node *node() const;
			FLAME_FOUNDATION_EXPORTS VaribleInfo *varible_info() const;

			FLAME_FOUNDATION_EXPORTS int array_item_count() const;
			FLAME_FOUNDATION_EXPORTS Item *array_item(int idx) const;
			FLAME_FOUNDATION_EXPORTS Item *array_insert_item(int idx);
			FLAME_FOUNDATION_EXPORTS void array_remove_item(int idx);
			FLAME_FOUNDATION_EXPORTS void array_clear() const;
		};

		struct Output
		{
			FLAME_FOUNDATION_EXPORTS Node *node() const;
			FLAME_FOUNDATION_EXPORTS VaribleInfo *varible_info() const;
			FLAME_FOUNDATION_EXPORTS Item *item() const; /* output has only one item */
		};

		struct Node
		{
			FLAME_FOUNDATION_EXPORTS BP *bp() const;
			FLAME_FOUNDATION_EXPORTS const char *id() const;
			FLAME_FOUNDATION_EXPORTS UDT *udt() const;
			FLAME_FOUNDATION_EXPORTS int input_count() const;
			FLAME_FOUNDATION_EXPORTS Input *input(int idx) const;
			FLAME_FOUNDATION_EXPORTS int output_count() const;
			FLAME_FOUNDATION_EXPORTS Output *output(int idx) const;
			FLAME_FOUNDATION_EXPORTS bool enable() const;
			FLAME_FOUNDATION_EXPORTS void set_enable(bool enable) const;
		};

		FLAME_FOUNDATION_EXPORTS int node_count() const;
		FLAME_FOUNDATION_EXPORTS Node *node(int idx) const;
		FLAME_FOUNDATION_EXPORTS Node *add_node(const char *id, UDT *udt);
		FLAME_FOUNDATION_EXPORTS void remove_node(Node *n);
		FLAME_FOUNDATION_EXPORTS Node *find_node(const char *id) const;

		FLAME_FOUNDATION_EXPORTS Item *find_item(const char *address);

		FLAME_FOUNDATION_EXPORTS void clear();
		FLAME_FOUNDATION_EXPORTS void save(const wchar_t *filename);

		FLAME_FOUNDATION_EXPORTS static BP *create();
		FLAME_FOUNDATION_EXPORTS static BP *create_from_file(const wchar_t *filename);
		FLAME_FOUNDATION_EXPORTS static void destroy(BP *bp);
	};

	struct BP_Vec2 : R
	{
		float x$i;
		float y$i;

		Vec2 out$o;

		inline BP_Vec2() :
			x$i(0.f),
			y$i(0.f),
			out$o(0.f, 0.f)
		{
		}
	};
}

