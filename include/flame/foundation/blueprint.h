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

	inline char break_bp_node_type(const std::string& name, std::string* parameters = nullptr)
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

	struct bpSlot
	{
		bpNode* node;
		bpSlotIO io;
		uint index;
		TypeInfo* type;
		StringA name;
		uint offset;
		uint size;
		void* default_value;
		void* data;

		FLAME_FOUNDATION_EXPORTS void set_data(const void* data);

		void* data_p()
		{
			return *(void**)data;
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
			auto in_base_hash = in_type->base_hash;
			auto out_tag = out_out->tag;
			if (in_type->tag == TypePointer && (out_tag == TypeData || out_tag == TypePointer) &&
				(in_base_hash == out_out->base_hash || in_base_hash == FLAME_CHASH("void")))
				return true;

			return false;
		}

		Array<bpSlot*> links;

		FLAME_FOUNDATION_EXPORTS bool link_to(bpSlot* target);

		StringA get_address() const;

		void* user_data;
	};

	struct bpNode
	{
		bpScene* scene;
		bpNode* parent;

		Guid guid;
		StringA id;
		Vec2f pos;

		bpNodeType node_type;
		StringA type;
		UdtInfo* udt;

		Array<bpSlot*> inputs;
		Array<bpSlot*> outputs;

		Array<bpNode*> children;

		void* user_data;

		inline bool set_id(const std::string& _id);

		bpSlot* find_input(const std::string& name) const
		{
			for (auto in : inputs)
			{
				if (in->name == name)
					return in;
			}
			return nullptr;
		}

		bpSlot* find_output(const std::string& name) const
		{
			for (auto out : outputs)
			{
				if (out->name == name)
					return out;
			}
			return nullptr;
		}

		// TODO
		//bpSlot* find_input(const std::string& address/* node.var */) const
		//{
		//	auto sp = SUS::split_lastone(address, '.');
		//	auto n = find_node(sp[0]);
		//	if (!n)
		//		return nullptr;
		//	return n->find_input(sp[1]);
		//}

		// TODO
		//bpSlot* find_output(const std::string& address/* node.var */) const
		//{
		//	auto sp = SUS::split_lastone(address, '.');
		//	auto n = find_node(sp[0]);
		//	if (!n)
		//		return nullptr;
		//	return n->find_output(sp[1]);
		//}

		FLAME_FOUNDATION_EXPORTS bpNode* add_node(const char* id, const char* type, bpNodeType node_type = bpNodeReal);
		FLAME_FOUNDATION_EXPORTS void remove_node(bpNode* n);

		bpNode* find_node(const Guid& guid) const
		{
			for (auto n : children)
			{
				if (memcmp(&n->guid, &guid, sizeof(Guid)) == 0)
					return n;
				auto res = n->find_node(guid);
				if (res)
					return res;
			}
			return nullptr;
		}

		bpNode* find_node(const std::string& id) const
		{
			for (auto n : children)
			{
				if (n->id == id)
					return n;
				auto res = n->find_node(id);
				if (res)
					return res;
			}
			return nullptr;
		}

		FLAME_FOUNDATION_EXPORTS void update();
	};

	struct bpScene
	{
		float time;

		StringW filename;

		bpNode* root;

		FLAME_FOUNDATION_EXPORTS void update();

		FLAME_FOUNDATION_EXPORTS static bpScene* create_from_file(const wchar_t* filename);
		FLAME_FOUNDATION_EXPORTS static void save_to_file(bpScene* bp, const wchar_t* filename);
		FLAME_FOUNDATION_EXPORTS static void destroy(bpScene* bp);
	};

	StringA bpSlot::get_address() const
	{
		return StringA(node->id.str() + "." + name.v);
	}

	bool bpNode::set_id(const std::string& _id)
	{
		if (_id.empty())
			return false;
		if (id == _id)
			return true;
		if (scene->root->find_node(_id))
			return false;
		id = _id;
		return true;
	}
}

