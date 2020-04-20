#pragma once

#include <flame/foundation/typeinfo.h>

namespace flame
{
	struct BP
	{
		enum ObjectType
		{
			ObjectReal,
			ObjectRefRead,
			ObjectRefWrite
		};

		struct Node;

		struct Slot
		{
			enum IO
			{
				In,
				Out
			};

			FLAME_FOUNDATION_EXPORTS Node* node() const;
			FLAME_FOUNDATION_EXPORTS IO io() const;
			FLAME_FOUNDATION_EXPORTS uint index() const;
			FLAME_FOUNDATION_EXPORTS const TypeInfo* type() const;
			FLAME_FOUNDATION_EXPORTS const char* name() const;
			FLAME_FOUNDATION_EXPORTS uint offset() const;
			FLAME_FOUNDATION_EXPORTS uint size() const;
			FLAME_FOUNDATION_EXPORTS const void* default_value() const;

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

			void set_data_f(float f)
			{
				set_data(&f);
			}

			void set_data_p(const void* p)
			{
				set_data(&p);
			}

			static bool can_link(const TypeInfo* in_type, const TypeInfo* out_out)
			{
				if (in_type == out_out)
					return true;
				auto in_base_hash = in_type->base_hash();
				auto out_tag = out_out->tag();
				if (in_type->tag() == TypePointer && (out_tag == TypeData || out_tag == TypePointer) &&
					(in_base_hash == out_out->base_hash() || in_base_hash == FLAME_CHASH("void")))
					return true;

				return false;
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
			FLAME_FOUNDATION_EXPORTS ObjectType object_type() const;
			FLAME_FOUNDATION_EXPORTS const char* id() const;
			FLAME_FOUNDATION_EXPORTS bool set_id(const char* id);
			FLAME_FOUNDATION_EXPORTS const char* type() const;
			FLAME_FOUNDATION_EXPORTS UdtInfo* udt() const;
			Vec2f pos;

			FLAME_FOUNDATION_EXPORTS uint input_count() const;
			FLAME_FOUNDATION_EXPORTS Slot* input(uint idx) const;
			FLAME_FOUNDATION_EXPORTS uint output_count() const;
			FLAME_FOUNDATION_EXPORTS Slot* output(uint idx) const;

			FLAME_FOUNDATION_EXPORTS Slot* find_input(const char* name) const;
			FLAME_FOUNDATION_EXPORTS Slot* find_output(const char* name) const;

			FLAME_FOUNDATION_EXPORTS static Node* current();

			void* user_data;
		};

		inline static char break_node_type(const std::string& name, std::string* parameters = nullptr)
		{
			{
				static FLAME_SAL(prefix, "EnumSingle");
				if (name.compare(0, prefix.l, prefix.s) == 0)
				{
					if (parameters)
						*parameters = std::string(name.begin() + prefix.l + 1, name.end() - 1);
					return 'S';
				}
			}
			{
				static FLAME_SAL(prefix, "EnumMulti");
				if (name.compare(0, prefix.l, prefix.s) == 0)
				{
					if (parameters)
						*parameters = std::string(name.begin() + prefix.l + 1, name.end() - 1);
					return 'M';
				}
			}
			{
				static FLAME_SAL(prefix, "Variable");
				if (name.compare(0, prefix.l, prefix.s) == 0)
				{

					if (parameters)
						*parameters = std::string(name.begin() + prefix.l + 1, name.end() - 1);
					return 'V';
				}
			}
			{
				static FLAME_SAL(prefix, "Array");
				if (name.compare(0, prefix.l, prefix.s) == 0)
				{
					if (parameters)
						*parameters = std::string(name.begin() + prefix.l + 1, name.end() - 1);
					return 'A';
				}
			}
			return 0;
		}

		float time;

		FLAME_FOUNDATION_EXPORTS const wchar_t* filename() const;

		FLAME_FOUNDATION_EXPORTS uint node_count() const;
		FLAME_FOUNDATION_EXPORTS Node* node(uint idx) const;
		FLAME_FOUNDATION_EXPORTS Node* add_node(const char* id, const char* type, ObjectType object_type = ObjectReal);
		FLAME_FOUNDATION_EXPORTS void remove_node(Node* n);
		FLAME_FOUNDATION_EXPORTS Node* find_node(const char* id) const;
		FLAME_FOUNDATION_EXPORTS Slot* find_input(const char* address/* node.var*/) const;
		FLAME_FOUNDATION_EXPORTS Slot* find_output(const char* address/* node.var*/) const;

		FLAME_FOUNDATION_EXPORTS void clear();

		FLAME_FOUNDATION_EXPORTS void update();

		FLAME_FOUNDATION_EXPORTS static BP* create_from_file(const wchar_t* filename);
		FLAME_FOUNDATION_EXPORTS static void save_to_file(BP* bp, const wchar_t* filename);
		FLAME_FOUNDATION_EXPORTS static void destroy(BP* bp);
	};
}

