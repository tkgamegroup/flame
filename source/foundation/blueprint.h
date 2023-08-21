#pragma once

#include "foundation.h"
#include "typeinfo.h"

namespace flame
{
	struct BlueprintSlot
	{
		BlueprintNodePtr node;
		uint object_id;
		std::string name;
		uint name_hash = 0;
		std::vector<TypeInfo*> allowed_types;
		TypeInfo* type = nullptr;
		void* data = nullptr;
		std::string default_value;
		uint data_changed_frame = 0;

		inline bool allow_type(TypeInfo* type) const
		{
			for (auto t : allowed_types)
			{
				if (t == type)
					return true;
				if (t->tag == TagPU && type->tag == TagU)
				{
					auto ui = t->retrive_ui();
					if (ui && ui == type->retrive_ui())
						return true;
				}
			}
			return false;
		}
	};

	typedef BlueprintSlot* BlueprintSlotPtr;

	struct BlueprintArgument
	{
		TypeInfo* type;
		void* data;
	};

	struct BlueprintNodePreview
	{
		uint type;
		uvec2 extent;
		void* data;
	};

	typedef void(*BlueprintNodeFunction)(BlueprintArgument* inputs, BlueprintArgument* outputs);
	typedef void(*BlueprintNodeConstructor)(BlueprintArgument* inputs, BlueprintArgument* outputs);
	typedef void(*BlueprintNodeDestructor)(BlueprintArgument* inputs, BlueprintArgument* outputs);
	typedef void(*BlueprintNodeInputSlotChangedCallback)(TypeInfo** input_types, TypeInfo** output_types);
	typedef void(*BlueprintNodePreviewProvider)(BlueprintArgument* inputs, BlueprintArgument* outputs, BlueprintNodePreview* preview);

	struct BlueprintNode
	{
		BlueprintGroupPtr group;
		uint object_id;
		std::string name;
		uint name_hash = 0;
		std::vector<BlueprintSlot> inputs;
		std::vector<BlueprintSlot> outputs;
		BlueprintNodeFunction function = nullptr;
		BlueprintNodeConstructor constructor = nullptr;
		BlueprintNodeDestructor destructor = nullptr;
		BlueprintNodeInputSlotChangedCallback input_slot_changed_callback = nullptr;
		BlueprintNodePreviewProvider preview_provider = nullptr;

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
		uint object_id;
		BlueprintNodePtr	from_node;
		BlueprintSlotPtr	from_slot;
		BlueprintNodePtr	to_node;
		BlueprintSlotPtr	to_slot;

		virtual ~BlueprintLink() {}
	};

	struct BlueprintGroupSlot
	{
		BlueprintGroupPtr group;
		uint object_id;
		std::string name;
		uint name_hash = 0;
		TypeInfo* type = nullptr;
		void* data = nullptr;
		uint data_changed_frame = 0;
	};

	typedef BlueprintGroupSlot* BlueprintGroupSlotPtr;

	struct BlueprintGroup
	{
		BlueprintPtr blueprint;
		uint object_id;
		std::string name;
		uint name_hash = 0;
		std::vector<std::unique_ptr<BlueprintNodeT>> nodes;
		std::vector<std::unique_ptr<BlueprintLinkT>> links;
		std::vector<BlueprintGroupSlot> inputs;
		std::vector<BlueprintGroupSlot> outputs;

		vec2 offset;
		float scale = 1.f;

		uint structure_changed_frame = 0;
		uint data_changed_frame = 0;

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

		inline BlueprintGroupPtr find_group(uint name) const
		{
			for (auto& g : groups)
			{
				if (((BlueprintGroup*)g.get())->name_hash == name)
					return (BlueprintGroupPtr)g.get();
			}
			return nullptr;
		}

		virtual ~Blueprint() {}
		virtual BlueprintNodePtr		add_node(BlueprintGroupPtr group /*null means the first group*/, const std::string& name,
			const std::vector<BlueprintSlot>& inputs = {}, const std::vector<BlueprintSlot>& outputs = {},
			BlueprintNodeFunction function = nullptr, BlueprintNodeConstructor constructor = nullptr, BlueprintNodeDestructor destructor = nullptr,
			BlueprintNodeInputSlotChangedCallback input_slot_changed_callback = nullptr, BlueprintNodePreviewProvider preview_provider = nullptr) = 0;
		virtual BlueprintNodePtr		add_input_node(BlueprintGroupPtr group, uint name) = 0;
		virtual BlueprintNodePtr		add_variable_node(BlueprintGroupPtr group, uint variable_group_name) = 0;
		virtual void					remove_node(BlueprintNodePtr node) = 0;
		virtual BlueprintLinkPtr		add_link(BlueprintNodePtr from_node, uint from_slot, BlueprintNodePtr to_node, uint to_slot) = 0;
		virtual void					remove_link(BlueprintLinkPtr link) = 0;
		virtual BlueprintGroupPtr		add_group(const std::string& name) = 0;
		virtual void					remove_group(BlueprintGroupPtr group) = 0;
		virtual BlueprintGroupSlotPtr	add_group_input(BlueprintGroupPtr group, const std::string& name, TypeInfo* type) = 0;
		virtual void					remove_group_input(BlueprintGroupPtr group, BlueprintGroupSlotPtr slot) = 0;
		virtual BlueprintGroupSlotPtr	add_group_output(BlueprintGroupPtr group, const std::string& name, TypeInfo* type) = 0;
		virtual void					remove_group_output(BlueprintGroupPtr group, BlueprintGroupSlotPtr slot) = 0;

		virtual void					save(const std::filesystem::path& path = L"") = 0;

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
			BlueprintNodePreviewProvider preview_provider = nullptr;
		};

		std::vector<NodeTemplate> node_templates;

		std::filesystem::path filename;
		uint ref = 0;

		virtual ~BlueprintNodeLibrary() {}
		virtual void add_template(const std::string& name, const std::vector<BlueprintSlot>& inputs = {}, const std::vector<BlueprintSlot>& outputs = {},
			BlueprintNodeFunction function = nullptr, BlueprintNodeConstructor constructor = nullptr, BlueprintNodeDestructor destructor = nullptr,
			BlueprintNodeInputSlotChangedCallback input_slot_changed_callback = nullptr, BlueprintNodePreviewProvider preview_provider = nullptr) = 0;

		struct Get
		{
			virtual BlueprintNodeLibraryPtr operator()(const std::filesystem::path& filename /* L"standard" for standard */) = 0;
		};
		// Reflect static
		FLAME_FOUNDATION_API static Get& get;
	};

	struct BlueprintInstance
	{
		struct Node
		{
			BlueprintNodePtr original;
			uint object_id;
			std::vector<BlueprintArgument> inputs;
			std::vector<BlueprintArgument> outputs;
			uint updated_frame;
		};

		struct Group
		{
			struct Data
			{
				BlueprintArgument arg;
				uint changed_frame = 0;
			};

			std::map<uint, Data> datas; // key: group input output/slot id
			std::vector<Node> nodes;

			uint structure_updated_frame = 0;
			uint data_updated_frame = 0;

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
		virtual bool prepare_executing(uint group_name) = 0;
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
