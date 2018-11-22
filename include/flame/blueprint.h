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

#ifdef FLAME_WINDOWS
#ifdef FLAME_BLUEPRINT_MODULE
#define FLAME_BLUEPRINT_EXPORTS __declspec(dllexport)
#else
#define FLAME_BLUEPRINT_EXPORTS __declspec(dllimport)
#endif
#else
#define FLAME_BLUEPRINT_EXPORTS
#endif

#include <flame/math.h>

namespace flame
{
	namespace typeinfo
	{
		namespace cpp
		{
			struct EnumType;
			struct UDT;
		}
	}

	namespace blueprint
	{
		struct Node;

		struct InSlotItem
		{
			CommonData d;
			Node *n;

			inline InSlotItem()
			{
				*(Ivec4*)d.i = Ivec4(0);
				n = nullptr;
			}
		};

		struct InSlot
		{
			FLAME_BLUEPRINT_EXPORTS int item_count() const;
			FLAME_BLUEPRINT_EXPORTS InSlotItem *item(int idx) const;
		};

		struct OutSlot
		{
			CommonData d;

			inline OutSlot()
			{
				*(Ivec4*)d.i = Ivec4(0);
			}
		};

		struct Node
		{
			enum Type
			{
				TypeInputEnumSingle,
				TypeUDT
			};

			FLAME_BLUEPRINT_EXPORTS Type type() const;
			FLAME_BLUEPRINT_EXPORTS const char *id() const;
		};

		struct NodeInputEnumSingle : Node
		{
			FLAME_BLUEPRINT_EXPORTS typeinfo::cpp::EnumType *enumeration() const;
			FLAME_BLUEPRINT_EXPORTS int idx() const;
		};

		struct NodeUDT : Node
		{
			FLAME_BLUEPRINT_EXPORTS typeinfo::cpp::UDT *udt() const;
			FLAME_BLUEPRINT_EXPORTS int insl_count() const;
			FLAME_BLUEPRINT_EXPORTS InSlot *insl(int idx) const;
			FLAME_BLUEPRINT_EXPORTS int outsl_count() const;
			FLAME_BLUEPRINT_EXPORTS OutSlot *outsl(int idx) const;
		};

		struct Scene
		{
			FLAME_BLUEPRINT_EXPORTS int node_count() const;
			FLAME_BLUEPRINT_EXPORTS Node *node(int idx) const;

			FLAME_BLUEPRINT_EXPORTS Node *add_node_input_enum_single(uint enum_hash, int value);
			FLAME_BLUEPRINT_EXPORTS Node *add_node_udt(uint hash);
			FLAME_BLUEPRINT_EXPORTS void remove_node(Node *n);

			FLAME_BLUEPRINT_EXPORTS void clear();
			FLAME_BLUEPRINT_EXPORTS void save(const wchar_t *filename);

			FLAME_BLUEPRINT_EXPORTS static Scene *create();
			FLAME_BLUEPRINT_EXPORTS static Scene *create_from_file(const wchar_t *filename);
			FLAME_BLUEPRINT_EXPORTS static void destroy(Scene *s);
		};
	}
}

