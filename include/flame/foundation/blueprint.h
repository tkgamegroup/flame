#pragma once

#include <flame/foundation/foundation.h>

namespace flame
{
	/*
		- A blueprint(BP) is a scene that represents relations between objects.
		- An object is called node and associated with an udt or a package that contains another scene.
		- At least one input or output must exist in the reflected udts.
		- The udt must have a update function, the function return nothing and takes no parameters
		- Address: [node_id].[varible_name]
	*/

	struct TypeInfo;
	struct VariableInfo;
	struct UdtInfo;
	struct TypeinfoDatabase;

	struct BP
	{
		struct Node;

		struct Library
		{
			FLAME_FOUNDATION_EXPORTS const wchar_t* filename() const;
			FLAME_FOUNDATION_EXPORTS TypeinfoDatabase* db() const;
		};

		struct Slot
		{
			enum IO
			{
				In,
				Out
			};

			FLAME_FOUNDATION_EXPORTS Node* node() const;
			FLAME_FOUNDATION_EXPORTS IO io() const;
			FLAME_FOUNDATION_EXPORTS const TypeInfo* type() const;
			FLAME_FOUNDATION_EXPORTS const char* name() const;
			FLAME_FOUNDATION_EXPORTS uint offset() const;
			FLAME_FOUNDATION_EXPORTS uint size() const;
			FLAME_FOUNDATION_EXPORTS const char* default_value() const;

			FLAME_FOUNDATION_EXPORTS int frame() const;
			FLAME_FOUNDATION_EXPORTS void set_frame(int frame);
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

			FLAME_FOUNDATION_EXPORTS StringA get_address() const;

			void* user_data;
		};

		struct Node
		{
			FLAME_FOUNDATION_EXPORTS BP* scene() const;
			FLAME_FOUNDATION_EXPORTS const char* id() const;
			FLAME_FOUNDATION_EXPORTS void set_id(const char* id);
			FLAME_FOUNDATION_EXPORTS const char* type_name() const;
			FLAME_FOUNDATION_EXPORTS UdtInfo* udt() const;
			Vec2f pos;

			FLAME_FOUNDATION_EXPORTS uint input_count() const;
			FLAME_FOUNDATION_EXPORTS Slot* input(uint idx) const;
			FLAME_FOUNDATION_EXPORTS uint output_count() const;
			FLAME_FOUNDATION_EXPORTS Slot* output(uint idx) const;

			FLAME_FOUNDATION_EXPORTS Slot* find_input(const char* name) const;
			FLAME_FOUNDATION_EXPORTS Slot* find_output(const char* name) const;

			bool used_by_editor;
			void* user_data;
		};

		uint frame;
		float time;

		FLAME_FOUNDATION_EXPORTS const wchar_t* filename() const;

		FLAME_FOUNDATION_EXPORTS uint library_count() const;
		FLAME_FOUNDATION_EXPORTS Library* library(uint idx) const;
		FLAME_FOUNDATION_EXPORTS Library* add_library(const wchar_t* filename);
		FLAME_FOUNDATION_EXPORTS void remove_library(Library* m);
		FLAME_FOUNDATION_EXPORTS Library* find_library(const wchar_t* filename) const;

		FLAME_FOUNDATION_EXPORTS uint node_count() const;
		FLAME_FOUNDATION_EXPORTS Node* node(uint idx) const;
		FLAME_FOUNDATION_EXPORTS Node* add_node(const char* type, const char* id);
		FLAME_FOUNDATION_EXPORTS void remove_node(Node* n);
		FLAME_FOUNDATION_EXPORTS Node* find_node(const char* id) const;
		FLAME_FOUNDATION_EXPORTS Slot* find_input(const char* address) const;
		FLAME_FOUNDATION_EXPORTS Slot* find_output(const char* address) const;

		FLAME_FOUNDATION_EXPORTS void clear();

		FLAME_FOUNDATION_EXPORTS void update();

		FLAME_FOUNDATION_EXPORTS void report_used_resource(const wchar_t* filename);

		FLAME_FOUNDATION_EXPORTS static BP* create();
		FLAME_FOUNDATION_EXPORTS static BP* create_from_file(const wchar_t* filename);
		FLAME_FOUNDATION_EXPORTS static void save_to_file(BP* bp, const wchar_t* filename);
		FLAME_FOUNDATION_EXPORTS static void destroy(BP* bp);
	};
}

