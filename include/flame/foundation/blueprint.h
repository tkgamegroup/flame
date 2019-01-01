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

		struct Slot
		{
			CommonData d;
			Node *link;
		};

		struct Item
		{
			Array<Slot*> slots;
		};

		struct Node
		{
			StringAndHash id;
			UDT *udt;
			Array<Item*> items;
		};

		Array<Node*> nodes;

		FLAME_FOUNDATION_EXPORTS Node *add_node_udt(uint hash);
		FLAME_FOUNDATION_EXPORTS void remove_node(Node *n);

		FLAME_FOUNDATION_EXPORTS void clear();
		FLAME_FOUNDATION_EXPORTS void save(const wchar_t *filename);

		FLAME_FOUNDATION_EXPORTS static BP *create();
		FLAME_FOUNDATION_EXPORTS static BP *create_from_file(const wchar_t *filename);
		FLAME_FOUNDATION_EXPORTS static void destroy(BP *s);
	};
}

