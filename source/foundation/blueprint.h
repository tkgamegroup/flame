#pragma once

#include "foundation.h"

namespace flame
{
	struct BlueprintSlot
	{
		BlueprintNodePtr node;
		std::string name;
		uint name_hash = 0;
		std::vector<TypeInfo*> allowed_types;
		int type_idx = -1;
		void* data = nullptr;

		inline int find_type(TypeInfo* type) const
		{
			for (auto i = 0; i < allowed_types.size(); i++)
			{
				if (allowed_types[i] == type)
					return i;
			}
			return -1;
		}

		inline TypeInfo* get_type(int idx) const
		{
			return idx != -1 ? allowed_types[idx] : nullptr;
		}

		inline TypeInfo* get_type() const
		{
			return get_type(type_idx);
		}
	};

	typedef BlueprintSlot* BlueprintSlotPtr;

	struct BlueprintArgument
	{
		int type_idx;
		void* data;
	};

	typedef void(*BlueprintNodeFunction)(BlueprintArgument* inputs, BlueprintArgument* outputs);
	typedef void(*BlueprintNodeConstructor)(BlueprintArgument* inputs, BlueprintArgument* outputs);
	typedef void(*BlueprintNodeDestructor)(BlueprintArgument* inputs, BlueprintArgument* outputs);
	typedef void(*BlueprintNodeInputSlotChangedCallback)(TypeInfo** input_types, TypeInfo** output_types);
	typedef void(*BlueprintNodePreviewer)(BlueprintArgument* inputs, BlueprintArgument* outputs, void* wtf);

	struct BlueprintNode
	{
		BlueprintGroupPtr group;
		std::string name;
		uint name_hash = 0;
		std::vector<BlueprintSlot> inputs;
		std::vector<BlueprintSlot> outputs;
		BlueprintNodeFunction function = nullptr;
		BlueprintNodeConstructor constructor = nullptr;
		BlueprintNodeDestructor destructor = nullptr;
		BlueprintNodeInputSlotChangedCallback input_slot_changed_callback = nullptr;
		BlueprintNodePreviewer previewer = nullptr;

		vec2 position;
		bool collapsed = false;

		virtual ~BlueprintNode() {}

		inline BlueprintSlotPtr find_input(uint name) const
		{
			for (auto& i : inputs)
			{
				if (i.name_hash == name)
					return (BlueprintSlotPtr)&i;
			}
			return nullptr;
		}

		inline BlueprintSlotPtr find_output(uint name) const
		{
			for (auto& o : outputs)
			{
				if (o.name_hash == name)
					return (BlueprintSlotPtr)&o;
			}
			return nullptr;
		}
	};

	struct BlueprintLink
	{
		BlueprintNodePtr	from_node;
		BlueprintSlotPtr	from_slot;
		BlueprintNodePtr	to_node;
		BlueprintSlotPtr	to_slot;

		virtual ~BlueprintLink() {}
	};

	struct BlueprintGroup
	{
		BlueprintPtr blueprint;
		std::string name;
		uint name_hash = 0;
		std::vector<std::unique_ptr<BlueprintNodeT>> nodes;
		std::vector<std::unique_ptr<BlueprintLinkT>> links;
		std::vector<BlueprintSlot> inputs;
		std::vector<BlueprintSlot> outputs;

		vec2 offset;
		float scale = 1.f;

		virtual ~BlueprintGroup() {}
	};

	// Reflect ctor
	struct Blueprint
	{
		// Reflect
		std::vector<std::unique_ptr<BlueprintGroupT>> groups;

		uint dirty_frame;

		std::filesystem::path filename;
		uint ref = 0;

		virtual ~Blueprint() {}
		virtual BlueprintNodePtr	add_node(BlueprintGroupPtr group /*null means the main group*/, const std::string& name, 
			const std::vector<BlueprintSlot>& inputs = {}, const std::vector<BlueprintSlot>& outputs = {}, 
			BlueprintNodeFunction function = nullptr, BlueprintNodeConstructor constructor = nullptr, BlueprintNodeDestructor destructor = nullptr,
			BlueprintNodeInputSlotChangedCallback input_slot_changed_callback = nullptr, BlueprintNodePreviewer previewer = nullptr) = 0;
		virtual void				remove_node(BlueprintNodePtr node) = 0;
		virtual BlueprintLinkPtr	add_link(BlueprintNodePtr from_node, uint from_slot, BlueprintNodePtr to_node, uint to_slot) = 0;
		virtual void				remove_link(BlueprintLinkPtr link) = 0;
		virtual BlueprintGroupPtr	add_group(const std::string& name) = 0;
		virtual void				remove_group(BlueprintGroupPtr group) = 0;
		virtual void				move_to_group(const std::vector<BlueprintNodePtr>& nodes) = 0;

		virtual void save() = 0;

		struct Get
		{
			virtual BlueprintPtr operator()(const std::filesystem::path& filename) = 0;
		};
		// Reflect static
		FLAME_FOUNDATION_API static Get& get;

		struct Release
		{
			virtual void operator()(BlueprintPtr blueprint) = 0;
		};
		// Reflect static
		FLAME_FOUNDATION_API static Release& release;
	};

	struct BlueprintNodeLibrary
	{
		struct NodeTemplate
		{
			std::string name;
			uint name_hash = 0;
			std::vector<BlueprintSlot> inputs;
			std::vector<BlueprintSlot> outputs;
			BlueprintNodeFunction function = nullptr;
			BlueprintNodeConstructor constructor = nullptr;
			BlueprintNodeDestructor destructor = nullptr;
			BlueprintNodeInputSlotChangedCallback input_slot_changed_callback = nullptr;
			BlueprintNodePreviewer previewer = nullptr;
		};

		std::vector<NodeTemplate> node_templates;

		std::filesystem::path filename;
		uint ref = 0;

		virtual ~BlueprintNodeLibrary() {}
		virtual void add_template(const std::string& name, const std::vector<BlueprintSlot>& inputs = {}, const std::vector<BlueprintSlot>& outputs = {},
			BlueprintNodeFunction function = nullptr, BlueprintNodeConstructor constructor = nullptr, BlueprintNodeDestructor destructor = nullptr,
			BlueprintNodeInputSlotChangedCallback input_slot_changed_callback = nullptr, BlueprintNodePreviewer previewer = nullptr) = 0;

		struct Get
		{
			virtual BlueprintNodeLibraryPtr operator()(const std::filesystem::path& filename /* L"standard" for standard */ ) = 0;
		};
		// Reflect static
		FLAME_FOUNDATION_API static Get& get;
	};

	struct BlueprintInstance
	{
		struct Node
		{
			BlueprintNodePtr original;
			std::vector<BlueprintArgument> inputs;
			std::vector<BlueprintArgument> outputs;
		};

		struct Group
		{
			std::map<BlueprintSlotPtr, std::pair<TypeInfo*, void*>> datas;
			std::vector<Node> nodes;

			~Group();

			inline const Node* find(BlueprintNodePtr original) const
			{
				for (auto& n : nodes)
				{
					if (n.original == original)
						return &n;
				}
				return nullptr;
			}
		};

		BlueprintPtr blueprint;

		std::map<uint, Group> groups;

		uint executing_group = 0;
		Group* current_group = nullptr;
		int current_node = -1;

		uint built_frame;

		virtual ~BlueprintInstance() {}

		inline Node* current_node_ptr() const
		{
			if (!current_group)
				return nullptr;
			if (current_node < 0 || current_node >= current_group->nodes.size())
				return nullptr;
			return &current_group->nodes[current_node];
		}

		virtual void build() = 0;
		virtual void prepare_executing(uint group_name) = 0;
		virtual void run() = 0;
		virtual void step() = 0;
		virtual void stop() = 0;

		struct Create
		{
			virtual BlueprintInstancePtr operator()(BlueprintPtr blueprint) = 0;
		};
		// Reflect static
		FLAME_FOUNDATION_API static Create& create;
	};

	struct BlueprintDebugger
	{
		std::vector<std::pair<BlueprintNodePtr, bool>> break_nodes;

		BlueprintInstancePtr debugging = nullptr;

		virtual ~BlueprintDebugger() {}

		inline bool has_break_node(BlueprintNodePtr node) const
		{
			for (auto& i : break_nodes)
			{
				if (i.first == node)
					return true;
			}
			return false;
		}

		virtual void add_break_node(BlueprintNodePtr node) = 0;
		virtual void remove_break_node(BlueprintNodePtr node) = 0;

		struct Create
		{
			virtual BlueprintDebuggerPtr operator()() = 0;
		};
		// Reflect static
		FLAME_FOUNDATION_API static Create& create;

		struct Current
		{
			virtual BlueprintDebuggerPtr operator()() = 0;
		};
		// Reflect static
		FLAME_FOUNDATION_API static Current& current;

		struct SetCurrent
		{
			virtual BlueprintDebuggerPtr operator()(BlueprintDebuggerPtr debugger) = 0;
		};
		// Reflect static
		FLAME_FOUNDATION_API static SetCurrent& set_current;
	};
}
