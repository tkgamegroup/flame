#pragma once

#include <flame/foundation/foundation.h>

namespace flame
{
	/*
		- A blueprint(BP) is a scene that represents relations between objects.
		- An object is called node and associated with an udt or a package that contains another scene.
		- The reflected members with attribute 'i' or 'o' will be as inputs or outpus.
		- All inputs and outputs must be Attribute[*]<T> type.
		- The udt must have a update function, the function return nothing and takes no parameters
		- Address: [package_id].[node_id].[varible_name]
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

		struct Module
		{
			FLAME_FOUNDATION_EXPORTS const std::wstring& filename() const;
			FLAME_FOUNDATION_EXPORTS void* module() const;
			FLAME_FOUNDATION_EXPORTS TypeinfoDatabase* db() const;
			Vec2f pos;

			bool external;
			void* user_data;
		};

		struct Package
		{
			FLAME_FOUNDATION_EXPORTS BP* scene() const;
			FLAME_FOUNDATION_EXPORTS const std::string& id() const;
			FLAME_FOUNDATION_EXPORTS void set_id(const std::string& id);
			Vec2f pos;

			FLAME_FOUNDATION_EXPORTS BP* bp() const;

			bool external;
			void* user_data;
		};

		struct Slot
		{
			enum Type
			{
				Input,
				Output
			};

			FLAME_FOUNDATION_EXPORTS Node* node() const;
			FLAME_FOUNDATION_EXPORTS Type type() const;
			FLAME_FOUNDATION_EXPORTS VariableInfo* vi() const;

			FLAME_FOUNDATION_EXPORTS int frame() const;
			FLAME_FOUNDATION_EXPORTS void set_frame(int frame);
			FLAME_FOUNDATION_EXPORTS void* raw_data() const;
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

			FLAME_FOUNDATION_EXPORTS uint link_count() const;
			FLAME_FOUNDATION_EXPORTS Slot* link(int idx = 0) const;
			FLAME_FOUNDATION_EXPORTS bool link_to(Slot* target);

			FLAME_FOUNDATION_EXPORTS Mail<std::string> get_address() const;

			void* user_data;
		};

		struct Node
		{
			FLAME_FOUNDATION_EXPORTS BP* scene() const;
			FLAME_FOUNDATION_EXPORTS const std::string& id() const;
			FLAME_FOUNDATION_EXPORTS void set_id(const std::string& id);
			FLAME_FOUNDATION_EXPORTS UdtInfo* udt() const;
			Vec2f pos;

			FLAME_FOUNDATION_EXPORTS uint input_count() const;
			FLAME_FOUNDATION_EXPORTS Slot* input(uint idx) const;
			FLAME_FOUNDATION_EXPORTS uint output_count() const;
			FLAME_FOUNDATION_EXPORTS Slot* output(uint idx) const;

			FLAME_FOUNDATION_EXPORTS Slot* find_input(const std::string& name) const;
			FLAME_FOUNDATION_EXPORTS Slot* find_output(const std::string& name) const;

			bool external;
			void* user_data;
		};

		struct Group
		{

		};

		struct Environment
		{
			std::wstring path;
			std::vector<TypeinfoDatabase*> dbs;
			float time;
		};

		float time;

		FLAME_FOUNDATION_EXPORTS Package* package() const;

		FLAME_FOUNDATION_EXPORTS uint module_count() const;
		FLAME_FOUNDATION_EXPORTS Module* module(uint idx) const;
		FLAME_FOUNDATION_EXPORTS Module* self_module() const;
		FLAME_FOUNDATION_EXPORTS Module* add_module(const std::wstring& filename);
		FLAME_FOUNDATION_EXPORTS void remove_module(Module* m);
		FLAME_FOUNDATION_EXPORTS Module* find_module(const std::wstring& filename) const;

		FLAME_FOUNDATION_EXPORTS uint package_count() const;
		FLAME_FOUNDATION_EXPORTS Package* package(uint idx) const;
		FLAME_FOUNDATION_EXPORTS Package* add_package(const std::wstring& filename, const std::string& id);
		FLAME_FOUNDATION_EXPORTS void remove_package(Package* e);
		FLAME_FOUNDATION_EXPORTS Package* find_package(const std::string& id) const;

		FLAME_FOUNDATION_EXPORTS const std::vector<TypeinfoDatabase*> dbs() const;

		FLAME_FOUNDATION_EXPORTS uint node_count() const;
		FLAME_FOUNDATION_EXPORTS Node* node(uint idx) const;
		FLAME_FOUNDATION_EXPORTS Node* add_node(uint type_hash, const std::string& id);
		FLAME_FOUNDATION_EXPORTS void remove_node(Node* n);
		FLAME_FOUNDATION_EXPORTS Node* find_node(const std::string& address) const;
		FLAME_FOUNDATION_EXPORTS Slot* find_input(const std::string& address) const;
		FLAME_FOUNDATION_EXPORTS Slot* find_output(const std::string& address) const;
		
		FLAME_FOUNDATION_EXPORTS uint input_export_count() const;
		FLAME_FOUNDATION_EXPORTS Slot* input_export(uint idx) const;
		FLAME_FOUNDATION_EXPORTS void add_input_export(Slot* s);
		FLAME_FOUNDATION_EXPORTS void remove_input_export(Slot* s);
		FLAME_FOUNDATION_EXPORTS int find_input_export(Slot* s) const;

		FLAME_FOUNDATION_EXPORTS uint output_export_count() const;
		FLAME_FOUNDATION_EXPORTS Slot* output_export(uint idx) const;
		FLAME_FOUNDATION_EXPORTS void add_output_export(Slot* s);
		FLAME_FOUNDATION_EXPORTS void remove_output_export(Slot* s);
		FLAME_FOUNDATION_EXPORTS int find_output_export(Slot* s) const;

		FLAME_FOUNDATION_EXPORTS void clear();

		FLAME_FOUNDATION_EXPORTS void update();

		FLAME_FOUNDATION_EXPORTS static BP* create();
		FLAME_FOUNDATION_EXPORTS static BP* create_from_file(const std::wstring& filename, bool no_compile = false);
		FLAME_FOUNDATION_EXPORTS static void save_to_file(BP* bp, const std::wstring& filename);
		FLAME_FOUNDATION_EXPORTS static void destroy(BP* bp);
	};

	FLAME_FOUNDATION_EXPORTS const BP::Environment& bp_env();
}

