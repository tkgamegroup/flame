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
		- The reflected members of the udt will be separated into inputs and outpus.
		- An input has an attribute 'i', and an output has an attribute 'o'.
		- Address in BP: [node_id].[varible_name]
		  you can use address to find an object in BP, e.g.
		  'a'     for node
		  'a.b'   for node input or output
		- An available udt should:
			all of its data types must be pods
			have an nonparametric void function called 'update'
			have an nonparametric void function called 'initialize' (optional)
			have an nonparametric void function called 'finish' (optional)
		- A BP file is basically a XML file
	*/

	struct VariableInfo;
	struct UdtInfo;
	struct SerializableNode;

	struct BP
	{
		struct Node;

		struct Slot
		{
			enum Type
			{
				Input,
				Output
			};

			FLAME_FOUNDATION_EXPORTS Type type() const;
			FLAME_FOUNDATION_EXPORTS Node* node() const;
			FLAME_FOUNDATION_EXPORTS VariableInfo* variable_info() const;

			FLAME_FOUNDATION_EXPORTS void* data();
			FLAME_FOUNDATION_EXPORTS void set_data(const void* data);

			FLAME_FOUNDATION_EXPORTS int link_count() const;
			FLAME_FOUNDATION_EXPORTS Slot* link(int idx = 0) const;
			FLAME_FOUNDATION_EXPORTS bool link_to(Slot* target);

			FLAME_FOUNDATION_EXPORTS String get_address() const;
		};

		struct Node
		{
			FLAME_FOUNDATION_EXPORTS BP* bp() const;
			FLAME_FOUNDATION_EXPORTS const char* id() const;
			FLAME_FOUNDATION_EXPORTS UdtInfo* udt() const;
			FLAME_FOUNDATION_EXPORTS Vec2f position() const;
			FLAME_FOUNDATION_EXPORTS void set_position(const Vec2f& p);

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
		FLAME_FOUNDATION_EXPORTS Node* add_node(const char* type_name, const char* id);
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

		FLAME_FOUNDATION_EXPORTS void load(SerializableNode* src);
		FLAME_FOUNDATION_EXPORTS void load(const std::wstring& filename);
		FLAME_FOUNDATION_EXPORTS void save(SerializableNode* dst);
		FLAME_FOUNDATION_EXPORTS void save(const std::wstring& filename);

		FLAME_FOUNDATION_EXPORTS static BP *create();
		FLAME_FOUNDATION_EXPORTS static BP *create_from_file(const std::wstring& filename);
		FLAME_FOUNDATION_EXPORTS static void destroy(BP *bp);
	};

	// basic nodes are available after calling typeinfo_init_basic_bp_nodes or loading from file
	// they are:
	//  Vec<[1-4], [float, uint, int, uchar, bool] (as Vec<*>)
	//  Mat<[2-4], [2-4], [float, double]> (as Mat<*>)
	//  LNA<[1-16], [float, uint, int, uchar, bool, voidptr, Vec<*>, Mat<*>]>
}

