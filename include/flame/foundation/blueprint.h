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
			have a bool function that takes one float parameter (the delta_time, delta_time < 0 means the end of the node) and its name is 'update' (it returns if it needs update in next frame)
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

			FLAME_FOUNDATION_EXPORTS Mail<std::string> get_address() const;
		};

		struct Node
		{
			FLAME_FOUNDATION_EXPORTS BP* bp() const;
			FLAME_FOUNDATION_EXPORTS const std::string& id() const;
			FLAME_FOUNDATION_EXPORTS UdtInfo* udt() const;
			FLAME_FOUNDATION_EXPORTS Vec2f position() const;
			FLAME_FOUNDATION_EXPORTS void set_position(const Vec2f& p);

			FLAME_FOUNDATION_EXPORTS int input_count() const;
			FLAME_FOUNDATION_EXPORTS Slot* input(int idx) const;
			FLAME_FOUNDATION_EXPORTS int output_count() const;
			FLAME_FOUNDATION_EXPORTS Slot* output(int idx) const;

			FLAME_FOUNDATION_EXPORTS Slot* find_input(const std::string& name) const;
			FLAME_FOUNDATION_EXPORTS Slot* find_output(const std::string& name) const;
		};

		FLAME_FOUNDATION_EXPORTS int node_count() const;
		FLAME_FOUNDATION_EXPORTS Node* node(int idx) const;
		FLAME_FOUNDATION_EXPORTS Node* add_node(const std::string& type_name, const std::string& id);
		FLAME_FOUNDATION_EXPORTS void remove_node(Node* n);

		FLAME_FOUNDATION_EXPORTS Node* find_node(const std::string& id) const;
		FLAME_FOUNDATION_EXPORTS Slot* find_input(const std::string& address) const;
		FLAME_FOUNDATION_EXPORTS Slot* find_output(const std::string& address) const;

		FLAME_FOUNDATION_EXPORTS void clear();

		FLAME_FOUNDATION_EXPORTS void update(float delta_time);

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

