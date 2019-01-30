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
	/*
		- A blueprint(BP) is a scene that represents relations between objects.
		- An object is called node in a BP.
		- A node is bound to an udt which its name started with 'BP_'.
		- The reflected members of the udt will be separated into inputs and outpus.
		- An input has an attribute 'i', and an output has an attribute 'o'.
		- Both inputs and outputs must be compatible with CommonData.
		- Only for inputs, if it is an Array<>, there will be multiple items of it,
		  else, there will be only one item of it
		- An available udt must have at least one input and one output
		- A BP file is basically a XML file, you can use both .xml or .bp.
	*/

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
			FLAME_FOUNDATION_EXPORTS void set_data(const CommonData &d); // setting datas for output's item is ok, but the data will be rushed when the node update

			FLAME_FOUNDATION_EXPORTS Item *link() const;
			FLAME_FOUNDATION_EXPORTS bool set_link(Item *target); // it is vaild for input's item only

			FLAME_FOUNDATION_EXPORTS String get_address() const; // node_id.varible_name.item_index (item_index is default for 0, vaild on input's item)
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
			FLAME_FOUNDATION_EXPORTS Item *item() const;
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

			FLAME_FOUNDATION_EXPORTS void update() const;
		};

		FLAME_FOUNDATION_EXPORTS int node_count() const;
		FLAME_FOUNDATION_EXPORTS Node *node(int idx) const;
		FLAME_FOUNDATION_EXPORTS Node *add_node(const char *id, UDT *udt);
		FLAME_FOUNDATION_EXPORTS void remove_node(Node *n);
		FLAME_FOUNDATION_EXPORTS Node *find_node(const char *id) const;

		FLAME_FOUNDATION_EXPORTS Item *find_item(const char *address);

		FLAME_FOUNDATION_EXPORTS void clear();

		// before you update the BP, you should call install_dummys, basically, 
		// it let all notes create a piece of memory to represent the 'true' object
		FLAME_FOUNDATION_EXPORTS void install_dummys();
		// when you're done all of your updatings, call uninstall_dummys
		FLAME_FOUNDATION_EXPORTS void uninstall_dummys();
		FLAME_FOUNDATION_EXPORTS void update();

		FLAME_FOUNDATION_EXPORTS void save(const wchar_t *filename);

		FLAME_FOUNDATION_EXPORTS static BP *create();
		FLAME_FOUNDATION_EXPORTS static BP *create_from_file(const wchar_t *filename);
		FLAME_FOUNDATION_EXPORTS static void destroy(BP *bp);
	};

	// here, we define some basic blueprint nodes

	struct BP_Int : R
	{
		int v$i;

		int v$o;

		FLAME_FOUNDATION_EXPORTS BP_Int();
		FLAME_FOUNDATION_EXPORTS void update();
	};

	struct BP_Float : R
	{
		float v$i;

		float v$o;

		FLAME_FOUNDATION_EXPORTS BP_Float();
		FLAME_FOUNDATION_EXPORTS void update();
	};

	struct BP_Bool : R
	{
		bool v$i;

		bool v$o;

		FLAME_FOUNDATION_EXPORTS BP_Bool();
		FLAME_FOUNDATION_EXPORTS void update();
	};

	struct BP_Vec2 : R
	{
		float x$i;
		float y$i;

		Vec2 v$o;

		FLAME_FOUNDATION_EXPORTS BP_Vec2();
		FLAME_FOUNDATION_EXPORTS void update();
	};

	struct BP_Vec3 : R
	{
		float x$i;
		float y$i;
		float z$i;

		Vec3 v$o;

		FLAME_FOUNDATION_EXPORTS BP_Vec3();
		FLAME_FOUNDATION_EXPORTS void update();
	};

	struct BP_Vec4 : R
	{
		float x$i;
		float y$i;
		float z$i;
		float w$i;

		Vec4 v$o;

		FLAME_FOUNDATION_EXPORTS BP_Vec4();
		FLAME_FOUNDATION_EXPORTS void update();
	};

	struct BP_Ivec2 : R
	{
		int x$i;
		int y$i;

		Ivec2 v$o;

		FLAME_FOUNDATION_EXPORTS BP_Ivec2();
		FLAME_FOUNDATION_EXPORTS void update();
	};

	struct BP_Ivec3 : R
	{
		int x$i;
		int y$i;
		int z$i;

		Ivec3 v$o;

		FLAME_FOUNDATION_EXPORTS BP_Ivec3();
		FLAME_FOUNDATION_EXPORTS void update();
	};

	struct BP_Ivec4 : R
	{
		int x$i;
		int y$i;
		int z$i;
		int w$i;

		Ivec4 v$o;

		FLAME_FOUNDATION_EXPORTS BP_Ivec4();
		FLAME_FOUNDATION_EXPORTS void update();
	};
}

