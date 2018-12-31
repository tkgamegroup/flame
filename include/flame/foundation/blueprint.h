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
	struct UDT;

	struct BP
	{
		struct Node;

		struct InSlotItem
		{
			CommonData d;
			Node *n;

			inline InSlotItem()
			{
				d.v.i = Ivec4(0);
				n = nullptr;
			}
		};

		struct InSlot
		{
			FLAME_FOUNDATION_EXPORTS int item_count() const;
			FLAME_FOUNDATION_EXPORTS InSlotItem *item(int idx) const;
		};

		struct OutSlot
		{
			CommonData d;

			inline OutSlot()
			{
				d.v.i = Ivec4(0);
			}
		};

		struct Node
		{
			enum Type
			{
				TypeInputEnumSingle,
				TypeUDT
			};

			FLAME_FOUNDATION_EXPORTS Type type() const;
			FLAME_FOUNDATION_EXPORTS const char *id() const;
		};

		struct NodeInputEnumSingle : Node
		{
			FLAME_FOUNDATION_EXPORTS EnumInfo *get_enum() const;
			FLAME_FOUNDATION_EXPORTS int idx() const;
		};

		struct NodeUDT : Node
		{
			FLAME_FOUNDATION_EXPORTS UDT *get_udt() const;
			FLAME_FOUNDATION_EXPORTS int insl_count() const;
			FLAME_FOUNDATION_EXPORTS InSlot *insl(int idx) const;
			FLAME_FOUNDATION_EXPORTS int outsl_count() const;
			FLAME_FOUNDATION_EXPORTS OutSlot *outsl(int idx) const;
		};

		FLAME_FOUNDATION_EXPORTS int node_count() const;
		FLAME_FOUNDATION_EXPORTS Node *node(int idx) const;

		FLAME_FOUNDATION_EXPORTS Node *add_node_input_enum_single(uint enum_hash, int value);
		FLAME_FOUNDATION_EXPORTS Node *add_node_udt(uint hash);
		FLAME_FOUNDATION_EXPORTS void remove_node(Node *n);

		FLAME_FOUNDATION_EXPORTS void clear();
		FLAME_FOUNDATION_EXPORTS void save(const wchar_t *filename);

		FLAME_FOUNDATION_EXPORTS static BP *create();
		FLAME_FOUNDATION_EXPORTS static BP *create_from_file(const wchar_t *filename);
		FLAME_FOUNDATION_EXPORTS static void destroy(BP *s);
	};
}

