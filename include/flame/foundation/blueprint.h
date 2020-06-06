#pragma once

#include <flame/foundation/typeinfo.h>

namespace flame
{
	enum bpUnitType
	{
		bpUnitNode,
		bpUnitGroup
	};

	enum bpNodeType
	{
		bpNodeReal,
		bpNodeRefRead,
		bpNodeRefWrite
	};

	enum bpSlotIO
	{
		bpSlotIn,
		bpSlotOut
	};

	struct BP
	{
		struct Slot;
		struct Unit;
		struct Node;
		struct Group;

		struct Slot
		{
			Node* node;
			Group* group;
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

			Array<Slot*> links;

			FLAME_FOUNDATION_EXPORTS bool link_to(Slot* target);

			StringA get_address() const
			{
				std::string str;
				if (node)
					str = node->id.str();
				if (group)
					str = group->id.str();
				return StringA(str + "." + name.v);
			}

			void* user_data;
		};

		struct Unit
		{
			bpUnitType unit_type;
			Guid guid;
			StringA id;
			Vec2f pos;
			void* user_data;

			inline bool set_id(const std::string& _id);
		};

		struct Node : Unit
		{
			Group* group;
			bpNodeType node_type;
			StringA type;
			UdtInfo* udt;

			Array<Slot*> inputs;
			Array<Slot*> outputs;

			Slot* find_input(const std::string& name) const
			{
				for (auto in : inputs)
				{
					if (in->name == name)
						return in;
				}
				return nullptr;
			}

			Slot* find_output(const std::string& name) const
			{
				for (auto out : outputs)
				{
					if (out->name == name)
						return out;
				}
				return nullptr;
			}
		};

		struct Group : Unit
		{
			BP* scene;

			Slot* signal;

			Array<Node*> nodes;

			FLAME_FOUNDATION_EXPORTS Node* add_node(const char* id, const char* type, bpNodeType node_type = bpNodeReal);
			FLAME_FOUNDATION_EXPORTS void remove_node(Node* n);

			Node* find_node(const Guid& guid) const
			{
				for (auto n : nodes)
				{
					if (memcmp(&n->guid, &guid, sizeof(Guid)) == 0)
						return n;
				}
				return nullptr;
			}

			Node* find_node(const std::string& id) const
			{
				for (auto n : nodes)
				{
					if (n->id == id)
						return n;
				}
				return nullptr;
			}

			Slot* find_input(const std::string& address/* node.var */) const
			{
				auto sp = SUS::split_lastone(address, '.');
				auto n = find_node(sp[0]);
				if (!n)
					return nullptr;
				return n->find_input(sp[1]);
			}

			Slot* find_output(const std::string& address/* node.var */) const
			{
				auto sp = SUS::split_lastone(address, '.');
				auto n = find_node(sp[0]);
				if (!n)
					return nullptr;
				return n->find_output(sp[1]);
			}

			FLAME_FOUNDATION_EXPORTS void update();
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

		StringW filename;

		Array<Group*> groups;

		FLAME_FOUNDATION_EXPORTS Group* add_group(const char* id);
		FLAME_FOUNDATION_EXPORTS void remove_group(Group* g);

		Group* find_group(const Guid& guid) const
		{
			for (auto g : groups)
			{
				if (memcmp(&g->guid, &guid, sizeof(Guid)) == 0)
					return g;
			}
			return nullptr;
		}

		Group* find_group(const std::string& id) const
		{
			for (auto g : groups)
			{
				if (g->id == id)
					return g;
			}
			return nullptr;
		}

		Node* find_node(const Guid& guid) const
		{
			for (auto g : groups)
			{
				auto n = g->find_node(guid);
				if (n)
					return n;
			}
			return nullptr;
		}

		FLAME_FOUNDATION_EXPORTS void update();

		FLAME_FOUNDATION_EXPORTS static BP* create_from_file(const wchar_t* filename);
		FLAME_FOUNDATION_EXPORTS static void save_to_file(BP* bp, const wchar_t* filename);
		FLAME_FOUNDATION_EXPORTS static void destroy(BP* bp);
	};

	bool BP::Unit::set_id(const std::string& _id)
	{
		if (_id.empty())
			return false;
		if (id == _id)
			return true;
		if (unit_type == bpUnitNode)
		{
			for (auto n : ((BP::Node*)this)->group->nodes)
			{
				if (n->id == _id)
					return false;
			}
		}
		else
		{
			for (auto g : ((Group*)this)->scene->groups)
			{
				if (g->id == _id)
					return false;
			}
		}
		id = _id;
		return true;
	}
}

