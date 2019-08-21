#pragma once

#include <flame/foundation/foundation.h>

namespace flame
{
	/*
		- A blueprint(BP) is a scene that represents relations between objects.
		- An object is called node and associated with an udt.
		- The reflected members with attribute 'i' or 'o' will be as inputs or outpus.
		- All inputs and outputs must be Attribute[*]<T> type.
		- The udt must have a update function, the function return nothing and takes no parameters
		- Address in BP: [node_id].[varible_name]
		  you can use address to find an object in BP, e.g.
		  'a'     for node
		  'a.b'   for node input or output
		- A BP file is basically a XML file
	*/

	struct SerializableNode;
	struct VariableInfo;
	struct UdtInfo;
	struct TypeinfoDatabase;

	namespace graphics
	{
		struct Device;
	}

	struct BP
	{
		struct Node;

		struct Environment
		{
			std::wstring path;
			std::vector<TypeinfoDatabase*> dbs;
			graphics::Device* graphics_device;
		};

		struct Slot
		{
			enum Type
			{
				Input,
				Output
			};

			Type type;
			FLAME_FOUNDATION_EXPORTS Node* node() const;
			VariableInfo* variable_info;

			FLAME_FOUNDATION_EXPORTS int frame() const;
			FLAME_FOUNDATION_EXPORTS void* data() const;
			FLAME_FOUNDATION_EXPORTS void set_data(const void* data);

			void* data_p()
			{
				return *(void**)data();
			}

			void set_data_i(int i)
			{
				set_data(&i);
			}

			void set_data_p(const void* p)
			{
				set_data(&p);
			}

			FLAME_FOUNDATION_EXPORTS int link_count() const;
			FLAME_FOUNDATION_EXPORTS Slot* link(int idx = 0) const;
			FLAME_FOUNDATION_EXPORTS bool link_to(Slot* target);

			FLAME_FOUNDATION_EXPORTS Mail<std::string> get_address() const;

			void* user_data;
		};

		struct Node
		{
			FLAME_FOUNDATION_EXPORTS BP* bp() const;
			FLAME_FOUNDATION_EXPORTS const std::string& id() const;
			UdtInfo* udt;
			Vec2f pos;

			FLAME_FOUNDATION_EXPORTS int input_count() const;
			FLAME_FOUNDATION_EXPORTS Slot* input(int idx) const;
			FLAME_FOUNDATION_EXPORTS int output_count() const;
			FLAME_FOUNDATION_EXPORTS Slot* output(int idx) const;

			FLAME_FOUNDATION_EXPORTS Slot* find_input(const std::string& name) const;
			FLAME_FOUNDATION_EXPORTS Slot* find_output(const std::string& name) const;
		};

		graphics::Device* graphics_device;

		FLAME_FOUNDATION_EXPORTS uint dependency_count() const;
		FLAME_FOUNDATION_EXPORTS Mail<std::wstring> dependency_filename(int idx) const;
		FLAME_FOUNDATION_EXPORTS TypeinfoDatabase* dependency_typeinfodatabase(int idx) const;
		FLAME_FOUNDATION_EXPORTS void add_dependency(const std::wstring& filename);
		FLAME_FOUNDATION_EXPORTS void remove_dependency(const std::wstring& filename);

		TypeinfoDatabase* typeinfodatabase;

		FLAME_FOUNDATION_EXPORTS uint node_count() const;
		FLAME_FOUNDATION_EXPORTS Node* node(int idx) const;
		FLAME_FOUNDATION_EXPORTS Node* add_node(const std::string& type_name, const std::string& id);
		FLAME_FOUNDATION_EXPORTS void remove_node(Node* n);

		FLAME_FOUNDATION_EXPORTS Node* find_node(const std::string& id) const;
		FLAME_FOUNDATION_EXPORTS Slot* find_input(const std::string& address) const;
		FLAME_FOUNDATION_EXPORTS Slot* find_output(const std::string& address) const;

		FLAME_FOUNDATION_EXPORTS void clear(); // all nodes and links

		FLAME_FOUNDATION_EXPORTS void update();

		FLAME_FOUNDATION_EXPORTS static BP* create_from_file(const std::wstring& filename, bool no_compile = false);
		FLAME_FOUNDATION_EXPORTS static void save_to_file(BP* bp, const std::wstring& filename);
		FLAME_FOUNDATION_EXPORTS static void destroy(BP* bp);
	};

	FLAME_FOUNDATION_EXPORTS const BP::Environment& bp_env();

	// basic nodes are available after calling typeinfo_init_basic_bp_nodes or loading from file
	// they are:
	//  Vec<[1-4], [float, uint, int, uchar, bool]
	//  Mat<[2-4], [2-4], [float, double]>
	//  Array<[1-16], [float, uint, int, uchar, bool, voidptr, Vec, Mat]>
}

