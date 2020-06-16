#pragma once

#include <flame/foundation/typeinfo.h>

namespace flame
{
	enum bpSlotIO
	{
		bpSlotIn,
		bpSlotOut
	};

	enum bpNodeType
	{
		bpNodeReal,
		bpNodeRefRead,
		bpNodeRefWrite
	};

	struct bpNode;
	struct bpScene;

	inline char bp_break_node_type(const std::string& name, std::string* parameters = nullptr)
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
		{
			static FLAME_SAL(prefix, "Group");
			if (name.compare(0, prefix.l, prefix.s) == 0)
			{
				if (parameters)
					*parameters = "";
				return 'G';
			}
		}
		return 0;
	}

	inline bool bp_can_link(const TypeInfo* in_type, const TypeInfo* out_out)
	{
		if (in_type == out_out)
			return true;
		auto in_base_hash = in_type->get_base_hash();
		auto out_tag = out_out->get_tag();
		if (in_type->get_tag() == TypePointer && (out_tag == TypeData || out_tag == TypePointer) &&
			(in_base_hash == out_out->get_base_hash() || in_base_hash == FLAME_CHASH("void")))
			return true;

		return false;
	}

	struct bpSlot
	{
		void* user_data;

		virtual bpNode* get_node() const = 0;
		virtual bpSlotIO get_io() const = 0;
		virtual uint get_index() const = 0;
		virtual TypeInfo* get_type() const = 0;
		virtual const char* get_name() const = 0;
		virtual uint get_offset() const = 0;
		virtual uint get_size() const = 0;
		virtual const void* get_data() const const = 0;
		inline const void* get_data_p() const { return *(void**)get_data(); }
		virtual void set_data(const void* data) = 0;
		inline void set_data_i(int i) { set_data(&i); }
		inline void set_data_f(float f) { set_data(&f); }
		inline void set_data_p(const void* p) { set_data(&p); }
		virtual const void* get_default_value() const = 0;

		virtual uint get_links_count() const = 0;
		virtual bpSlot* get_link(uint idx) const = 0;
		virtual bool link_to(bpSlot* target) = 0;
	};

	struct bpNode
	{
		void* user_data;

		virtual bpScene* get_scene() const = 0;
		virtual bpNode* get_parent() const = 0;

		virtual Guid get_guid() const = 0;
		virtual void set_guid(const Guid& guid) = 0;
		virtual const char* get_id() const = 0;
		virtual bool set_id(const char* id) = 0;
		virtual Vec2f get_pos() const = 0;
		virtual void set_pos(const Vec2f& pos) = 0;

		virtual bpNodeType get_node_type() const = 0;
		virtual const char* get_type() const = 0;
		virtual UdtInfo* get_udt() const = 0;

		virtual uint get_inputs_count() const = 0;
		virtual bpSlot* get_input(uint idx) const = 0;
		virtual bpSlot* find_input(const char* name) const = 0;
		virtual uint get_outputs_count() const = 0;
		virtual bpSlot* get_output(uint idx) const = 0;
		virtual bpSlot* find_output(const char* name) const = 0;

		virtual uint get_children_count() const = 0;
		virtual bpNode* get_child(uint idx) const = 0;
		virtual bpNode* add_child(const char* id, const char* type, bpNodeType node_type = bpNodeReal) = 0;
		virtual void remove_child(bpNode* n) = 0;
		virtual bpNode* find_child(const char* name) const = 0;
		virtual bpNode* find_child(const Guid& guid) const = 0;

		virtual void update() = 0;
	};

	struct bpScene
	{
		virtual void release() = 0;

		virtual const wchar_t* get_filename() const = 0;
		virtual float get_time() const = 0;
		virtual bpNode* get_root() const = 0;

		virtual void update() = 0;
		virtual void save() = 0;

		FLAME_FOUNDATION_EXPORTS static bpScene* create_from_file(const wchar_t* filename);
	};
}

