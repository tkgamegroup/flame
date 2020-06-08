#pragma once

#include <flame/foundation/blueprint.h>

namespace flame
{
	struct bpSlotPrivate;
	struct bpNodePrivate;
	struct bpScenePrivate;

	struct bpSlotPrivate : bpSlot
	{
		Setter* setter;
		void* listener;

		bpSlotPrivate(bpNode* _node, bpSlotIO _io, uint _index, TypeInfo* _type, const std::string& _name, uint _offset, uint _size, const void* _default_value);
		bpSlotPrivate(bpNode* node, bpSlotIO io, uint index, VariableInfo* vi);
		~bpSlotPrivate();
		void set_data(const void* data);
		bool link_to(bpSlotPrivate* target);
	};

	struct bpNodePrivate : bpNode
	{
		void* object;
		void* library;

		void* dtor_addr;
		void* update_addr;

		uint order;

		std::vector<bpNodePrivate*> update_list;
		bool need_rebuild_update_list;

		bpNodePrivate(bpNodeType _node_type, bpNodePrivate* parent, const std::string& id, const std::string& type);
		~bpNodePrivate();
		bpNodePrivate* add_node(const std::string& id, const std::string& type, bpNodeType node_type);
		void remove_node(bpNodePrivate* n);
		void update();
	};

	struct bpScenePrivate : bpScene
	{
		bpScenePrivate();
		~bpScenePrivate();
		void update();
	};
}
