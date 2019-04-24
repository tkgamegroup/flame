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
		- Address in BP: [node_id].[varible_name].[item_index]
		  you can use address to find an object in BP, e.g.
		  'a'     for node
		  'a.b'   for node input or output
		- An available udt should:
			have all data types being one of these:
			 CommonData's fmt
			 String or StringW
			have an nonparametric void function called 'update'
			have an nonparametric void function called 'initialize' (optional)
			have an nonparametric void function called 'finish' (optional)
			have a member indicates the module name (optional), such as: 'static const int flame_foundation$m;'
			 (we use the module name to run 'update', 'initialize' and 'finish' functions)
		- A BP file is basically a XML file, you can use both .xml or .bp.
	*/

	struct EnumInfo;
	struct VariableInfo;
	struct UdtInfo;

	struct BP
	{
		struct Node;

		struct Slot
		{
			FLAME_FOUNDATION_EXPORTS Node *node() const;
			FLAME_FOUNDATION_EXPORTS VariableInfo*variable_info() const;

			FLAME_FOUNDATION_EXPORTS CommonData& data();
			FLAME_FOUNDATION_EXPORTS void set_data(const CommonData& d); // setting datas for output's item is ok, but the data will be rushed when the node update

			FLAME_FOUNDATION_EXPORTS Slot* link() const; // link is only storaged in input's item
			FLAME_FOUNDATION_EXPORTS bool set_link(Slot* target); // it is vaild for input's item only

			FLAME_FOUNDATION_EXPORTS String get_address() const;
		};

		struct Node
		{
			FLAME_FOUNDATION_EXPORTS BP* bp() const;
			FLAME_FOUNDATION_EXPORTS const char* id() const;
			FLAME_FOUNDATION_EXPORTS UdtInfo* udt() const;
			FLAME_FOUNDATION_EXPORTS Vec2 position() const;
			FLAME_FOUNDATION_EXPORTS void set_position(const Vec2& p);

			FLAME_FOUNDATION_EXPORTS int input_count() const;
			FLAME_FOUNDATION_EXPORTS Slot* input(int idx) const;
			FLAME_FOUNDATION_EXPORTS int output_count() const;
			FLAME_FOUNDATION_EXPORTS Slot* output(int idx) const;

			FLAME_FOUNDATION_EXPORTS Slot* find_input(const char* name) const;
			FLAME_FOUNDATION_EXPORTS Slot* find_output(const char* name) const;

			FLAME_FOUNDATION_EXPORTS bool enable() const;
			FLAME_FOUNDATION_EXPORTS void set_enable(bool enable) const;
		};

		FLAME_FOUNDATION_EXPORTS int node_count() const;
		FLAME_FOUNDATION_EXPORTS Node* node(int idx) const;
		FLAME_FOUNDATION_EXPORTS Node* add_node(const char* id, UdtInfo* udt);
		FLAME_FOUNDATION_EXPORTS void remove_node(Node* n);

		FLAME_FOUNDATION_EXPORTS Node* find_node(const char* id) const;
		FLAME_FOUNDATION_EXPORTS Slot* find_input(const char* address) const;
		FLAME_FOUNDATION_EXPORTS Slot* find_output(const char* address) const;

		FLAME_FOUNDATION_EXPORTS void clear();

		// build data for 'update'
		// let all notes create a piece of memory to represent the 'true' object and
		// determines update order
		FLAME_FOUNDATION_EXPORTS void initialize();

		// release the data that built by 'initialize'
		FLAME_FOUNDATION_EXPORTS void finish();

		// update the 'bp' using nodes
		FLAME_FOUNDATION_EXPORTS void update();

		FLAME_FOUNDATION_EXPORTS void save(const wchar_t *filename);

		FLAME_FOUNDATION_EXPORTS static BP *create();
		FLAME_FOUNDATION_EXPORTS static BP *create_from_file(const wchar_t *filename);
		FLAME_FOUNDATION_EXPORTS static void destroy(BP *bp);
	};

	// here, we define some basic udt for blueprint nodes

	struct BP_Socket4$
	{
		static const int flame_foundation$m;

		CommonData v1$i;
		CommonData v2$i;
		CommonData v3$i;
		CommonData v4$i;

		CommonData* v$o;

		FLAME_FOUNDATION_EXPORTS void initialize$c();
		FLAME_FOUNDATION_EXPORTS void finish$c();
		FLAME_FOUNDATION_EXPORTS void update$c();
	};

	struct BP_Int$
	{
		static const int flame_foundation$m;

		int v$i;

		int v$o;

		FLAME_FOUNDATION_EXPORTS void update$c();
	};

	struct BP_Float$
	{
		static const int flame_foundation$m;

		float v$i;

		float v$o;

		FLAME_FOUNDATION_EXPORTS void update$c();
	};

	struct BP_Bool$
	{
		static const int flame_foundation$m;

		bool v$i;

		bool v$o;

		FLAME_FOUNDATION_EXPORTS void update$c();
	};

	struct BP_Vec2$
	{
		static const int flame_foundation$m;

		float x$i;
		float y$i;

		Vec2 v$o;

		FLAME_FOUNDATION_EXPORTS void update$c();
	};

	struct BP_Vec3$
	{
		static const int flame_foundation$m;

		float x$i;
		float y$i;
		float z$i;

		Vec3 v$o;

		FLAME_FOUNDATION_EXPORTS void update$c();
	};

	struct BP_Vec4$
	{
		static const int flame_foundation$m;

		float x$i;
		float y$i;
		float z$i;
		float w$i;

		Vec4 v$o;

		FLAME_FOUNDATION_EXPORTS void update$c();
	};

	struct BP_Ivec2$
	{
		static const int flame_foundation$m;

		int x$i;
		int y$i;

		Ivec2 v$o;

		FLAME_FOUNDATION_EXPORTS void update$c();
	};

	struct BP_Ivec3$
	{
		static const int flame_foundation$m;

		int x$i;
		int y$i;
		int z$i;

		Ivec3 v$o;

		FLAME_FOUNDATION_EXPORTS void update$c();
	};

	struct BP_Ivec4$
	{
		static const int flame_foundation$m;

		int x$i;
		int y$i;
		int z$i;
		int w$i;

		Ivec4 v$o;

		FLAME_FOUNDATION_EXPORTS void update$c();
	};

	struct BP_Bvec2$
	{
		static const int flame_foundation$m;

		uchar x$i;
		uchar y$i;

		Bvec2 v$o;

		FLAME_FOUNDATION_EXPORTS void update$c();
	};

	struct BP_Bvec3$
	{
		static const int flame_foundation$m;

		uchar x$i;
		uchar y$i;
		uchar z$i;

		Bvec3 v$o;

		FLAME_FOUNDATION_EXPORTS void update$c();
	};

	struct BP_Bvec4$
	{
		static const int flame_foundation$m;

		uchar x$i;
		uchar y$i;
		uchar z$i;
		uchar w$i;

		Bvec4 v$o;

		FLAME_FOUNDATION_EXPORTS void update$c();
	};
}

