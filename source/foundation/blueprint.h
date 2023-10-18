#pragma once

#include "foundation.h"
#include "typeinfo.h"

namespace flame
{
	enum BlueprintSlotFlags
	{
		BlueprintSlotFlagNone = 0,
		BlueprintSlotFlagInput = 1 << 0,
		BlueprintSlotFlagOutput = 1 << 1,
		BlueprintSlotFlagHideInUI = 1 << 2,
	};

	inline BlueprintSlotFlags operator|(BlueprintSlotFlags a, BlueprintSlotFlags b)
	{
		return (BlueprintSlotFlags)((uint)a | (uint)b);
	}

	inline BlueprintSlotFlags operator&(BlueprintSlotFlags a, BlueprintSlotFlags b)
	{
		return (BlueprintSlotFlags)((uint)a & (uint)b);
	}

	enum BlueprintBreakpointOption
	{
		BlueprintBreakpointNormal,
		BlueprintBreakpointTriggerOnce,
		BlueprintBreakpointBreakInCode
	};

	struct BlueprintSignal
	{
		uint v;
	};

	struct BlueprintAttribute
	{
		TypeInfo* type;
		void* data;
	};

	struct BlueprintSlotDesc
	{
		std::string				name;
		uint					name_hash;
		BlueprintSlotFlags		flags = BlueprintSlotFlagNone;
		std::vector<TypeInfo*>	allowed_types;
		std::string				default_value;
	};

	inline bool blueprint_allow_type(const std::vector<TypeInfo*>& allowed_types, TypeInfo* type)
	{
		for (auto t : allowed_types)
		{
			if (t == type)
				return true;
			if (t == TypeInfo::get<voidptr>())
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

	struct BlueprintSlot
	{
		uint					object_id;
		BlueprintNodePtr		node;
		std::string				name;
		uint					name_hash = 0;
		BlueprintSlotFlags		flags = BlueprintSlotFlagNone;
		std::vector<TypeInfo*>	allowed_types;
		TypeInfo*				type = nullptr;
		void*					data = nullptr;
		std::string				default_value;
		uint					data_changed_frame = 0;

		virtual bool is_linked() const = 0;
		virtual BlueprintSlotPtr get_linked(uint idx) const = 0;
	};

	struct BlueprintNodePreview
	{
		uint type;
		uvec2 extent;
		void* data;
	};

	typedef void(*BlueprintNodeFunction)(BlueprintAttribute* inputs, BlueprintAttribute* outputs);
	typedef void(*BlueprintNodeBeginBlockFunction)(BlueprintAttribute* inputs, BlueprintAttribute* outputs, uint* max_execute_times);
	typedef void(*BlueprintNodeEndBlockFunction)(BlueprintAttribute* inputs, BlueprintAttribute* outputs);
	typedef void(*BlueprintNodeConstructor)(BlueprintAttribute* inputs, BlueprintAttribute* outputs);
	typedef void(*BlueprintNodeDestructor)(BlueprintAttribute* inputs, BlueprintAttribute* outputs);
	typedef void(*BlueprintNodeInputSlotChangedCallback)(TypeInfo** input_types, TypeInfo** output_types);
	typedef void(*BlueprintNodePreviewProvider)(BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintNodePreview* preview);

	struct BlueprintNode
	{
		uint											object_id;
		BlueprintGroupPtr								group;

		std::string										name;
		uint											name_hash = 0;
		std::string										display_name;
		std::vector<std::unique_ptr<BlueprintSlotT>>	inputs;
		std::vector<std::unique_ptr<BlueprintSlotT>>	outputs;
		BlueprintNodeFunction							function = nullptr;
		BlueprintNodeConstructor						constructor = nullptr;
		BlueprintNodeDestructor							destructor = nullptr;
		BlueprintNodeInputSlotChangedCallback			input_slot_changed_callback = nullptr;
		BlueprintNodePreviewProvider					preview_provider = nullptr;
		bool											is_block = false;
		BlueprintNodeBeginBlockFunction					begin_block_function = nullptr;
		BlueprintNodeEndBlockFunction					end_block_function = nullptr;

		BlueprintNodePtr								parent;
		uint											depth = 0;
		std::vector<BlueprintNodePtr>					children;

		vec2 position;
		Rect rect;
		bool collapsed = false;

		virtual ~BlueprintNode() {}

		inline BlueprintSlotPtr find_input(uint name) const
		{
			for (auto& i : inputs)
			{
				if (((BlueprintSlot*)i.get())->name_hash == name)
					return (BlueprintSlotPtr)i.get();
			}
			return nullptr;
		}

		inline BlueprintSlotPtr find_output(uint name) const
		{
			for (auto& o : outputs)
			{
				if (((BlueprintSlot*)o.get())->name_hash == name)
					return (BlueprintSlotPtr)o.get();
			}
			return nullptr;
		}

		inline bool contains(BlueprintNodePtr oth) const
		{
			if (!is_block)
				return false;
			while (oth)
			{
				if (this == (BlueprintNode*)oth)
					return true;
				oth = (BlueprintNodePtr)((BlueprintNode*)oth)->parent;
			}
			return false;
		}
	};

	struct BlueprintVariable
	{
		std::string name;
		uint		name_hash = 0;
		TypeInfo*	type = nullptr;
		void*		data = nullptr;
	};

	struct BlueprintGroup
	{
		uint													object_id;
		BlueprintPtr											blueprint;

		std::string												name;
		uint													name_hash = 0;
		std::vector<BlueprintVariable>							variables;
		std::vector<BlueprintVariable>							inputs;
		std::vector<BlueprintVariable>							outputs;
		std::vector<std::unique_ptr<BlueprintNodeT>>			nodes;
		std::vector<std::unique_ptr<BlueprintLinkT>>			links;

		inline BlueprintNodePtr find_node(uint name) const
		{
			for (auto& n : nodes)
			{
				if (((BlueprintNode*)n.get())->name_hash == name)
					return (BlueprintNodePtr)n.get();
			}
			return nullptr;
		}

		vec2	offset;
		float	scale = 1.f;

		uint	variable_changed_frame = 1;
		uint	structure_changed_frame = 1;
		uint	data_changed_frame = 1;

		virtual ~BlueprintGroup() {}
	};

	struct BlueprintLink
	{
		uint				object_id;

		BlueprintSlotPtr	from_slot;
		BlueprintSlotPtr	to_slot;

		virtual ~BlueprintLink() {}
	};

	// Reflect ctor
	struct Blueprint
	{
		std::vector<BlueprintVariable>					variables;

		// Reflect
		std::vector<std::unique_ptr<BlueprintGroupT>>	groups;

		uint											variable_changed_frame = 1;
		uint											dirty_frame = 1;

		std::filesystem::path							filename;
		std::string										name;
		uint											name_hash;
		uint											ref = 0;

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

		virtual void*					add_variable(BlueprintGroupPtr group /* or null for blueprint variable */, const std::string& name, TypeInfo* type) = 0; // return: the data of the variable
		virtual void					remove_variable(BlueprintGroupPtr group /* or null for blueprint variable */, uint name) = 0;
		virtual void					alter_variable(BlueprintGroupPtr group /* or null for blueprint variable */, uint old_name, const std::string& new_name = "", TypeInfo* new_type = nullptr) = 0;
		virtual BlueprintNodePtr		add_node(BlueprintGroupPtr group, BlueprintNodePtr parent, const std::string& name, const std::string& display_name,
			const std::vector<BlueprintSlotDesc>& inputs = {}, const std::vector<BlueprintSlotDesc>& outputs = {},
			BlueprintNodeFunction function = nullptr, BlueprintNodeConstructor constructor = nullptr, BlueprintNodeDestructor destructor = nullptr,
			BlueprintNodeInputSlotChangedCallback input_slot_changed_callback = nullptr, BlueprintNodePreviewProvider preview_provider = nullptr, 
			bool is_block = false, BlueprintNodeBeginBlockFunction begin_block_function = nullptr, BlueprintNodeEndBlockFunction end_block_function = nullptr) = 0;
		virtual BlueprintNodePtr		add_block(BlueprintGroupPtr group, BlueprintNodePtr parent) = 0;
		virtual BlueprintNodePtr		add_variable_node(BlueprintGroupPtr group, BlueprintNodePtr parent, uint variable_name, uint type = "get"_h, uint location_name = 0 /* from a sheet or a static bp */ ) = 0;
		virtual BlueprintNodePtr		add_call_node(BlueprintGroupPtr group, BlueprintNodePtr parent, uint group_name, uint location_name = 0 /* from other static bp */ ) = 0; // add a node that will call another group
		virtual void					remove_node(BlueprintNodePtr node, bool recursively = true) = 0;
		virtual void					set_nodes_parent(const std::vector<BlueprintNodePtr> nodes, BlueprintNodePtr new_parent) = 0;
		virtual void					set_input_type(BlueprintSlotPtr slot, TypeInfo* type) = 0;
		virtual BlueprintLinkPtr		add_link(BlueprintSlotPtr from_slot, BlueprintSlotPtr to_slot) = 0;
		virtual void					remove_link(BlueprintLinkPtr link) = 0;
		virtual BlueprintGroupPtr		add_group(const std::string& name) = 0;
		virtual void					remove_group(BlueprintGroupPtr group) = 0;
		virtual void					add_group_input(BlueprintGroupPtr group, const std::string& name, TypeInfo* type) = 0;
		virtual void					remove_group_input(BlueprintGroupPtr group, uint name) = 0;
		virtual void					alter_group_input(BlueprintGroupPtr group, uint old_name, const std::string& new_name = "", TypeInfo* new_type = nullptr) = 0;
		virtual void					add_group_output(BlueprintGroupPtr group, const std::string& name, TypeInfo* type) = 0;
		virtual void					remove_group_output(BlueprintGroupPtr group, uint name) = 0;
		virtual void					alter_group_output(BlueprintGroupPtr group, uint old_name, const std::string& new_name = "", TypeInfo* new_type = nullptr) = 0;

		virtual void					save(const std::filesystem::path& path = L"") = 0;

		struct Get
		{
			virtual BlueprintPtr operator()(const std::filesystem::path& filename, bool is_static = false) = 0;
			virtual BlueprintPtr operator()(uint name) = 0;
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
			std::string								name;
			uint									name_hash = 0;
			std::string display_name;
			std::vector<BlueprintSlotDesc>			inputs;
			std::vector<BlueprintSlotDesc>			outputs;
			BlueprintNodeFunction					function = nullptr;
			BlueprintNodeConstructor				constructor = nullptr;
			BlueprintNodeDestructor					destructor = nullptr;
			BlueprintNodeInputSlotChangedCallback	input_slot_changed_callback = nullptr;
			BlueprintNodePreviewProvider			preview_provider = nullptr;
			bool									is_block = false;
			BlueprintNodeBeginBlockFunction			begin_block_function = nullptr;
			BlueprintNodeEndBlockFunction			end_block_function = nullptr;
		};

		std::vector<NodeTemplate>	node_templates;

		std::filesystem::path		filename;
		uint						ref = 0;

		virtual ~BlueprintNodeLibrary() {}
		virtual void add_template(const std::string& name, const std::string& display_name,
			const std::vector<BlueprintSlotDesc>& inputs = {}, const std::vector<BlueprintSlotDesc>& outputs = {},
			BlueprintNodeFunction function = nullptr, BlueprintNodeConstructor constructor = nullptr, BlueprintNodeDestructor destructor = nullptr,
			BlueprintNodeInputSlotChangedCallback input_slot_changed_callback = nullptr, BlueprintNodePreviewProvider preview_provider = nullptr, 
			bool is_block = false, BlueprintNodeBeginBlockFunction begin_block_function = nullptr, BlueprintNodeEndBlockFunction end_block_function = nullptr) = 0;

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
			uint							object_id;
			BlueprintNodePtr				original;
			std::vector<BlueprintAttribute> inputs;
			std::vector<BlueprintAttribute> outputs;
			std::vector<Node>				children;
			uint							order;
			uint							updated_frame;
		};

		struct ExecutingBlock
		{
			Node*	node;
			int		child_index;
			uint	executed_times;
			uint	max_execute_times;
		};

		struct Group
		{
			struct Data
			{
				BlueprintAttribute	attribute;
				bool				own_data = true;
				uint				changed_frame = 0;
			};

			BlueprintInstancePtr	instance;
			BlueprintGroupPtr		original;
			uint					name;

			std::map<uint, Data>							slot_datas; // key: slot id
			Node											root_node;
			std::map<uint, Node*>							node_map;
			Node*											input_node = nullptr;
			Node*											output_node = nullptr;
			std::unordered_map<uint, BlueprintAttribute>	variables; // key: variable name hash

			std::vector<ExecutingBlock>						executing_stack;

			uint											variable_updated_frame = 0;
			uint											structure_updated_frame = 0;
			uint											data_updated_frame = 0;

			template<class T>
			inline T get_variable(uint name)
			{
				if (auto it = variables.find(name); it != variables.end())
					return *(T*)it->second.data;
				return T(0);
			}

			template<class T>
			inline void set_variable(uint name, T v)
			{
				if (auto it = variables.find(name); it != variables.end())
					*(T*)it->second.data = v;
			}

			inline Node* executing_node() const
			{
				if (executing_stack.empty())
					return nullptr;
				auto& current_block = executing_stack.back();
				return &current_block.node->children[current_block.child_index];
			}
		};

		BlueprintPtr blueprint;
		bool is_static = false;

		std::unordered_map<uint, BlueprintAttribute>	variables; // key: variable name hash
		std::unordered_map<uint, Group>					groups; // key: group name hash

		uint variable_updated_frame = 0;
		uint built_frame = 0;

		template<class T>
		inline T get_variable(uint name)
		{
			if (auto it = variables.find(name); it != variables.end())
				return *(T*)it->second.data;
			return T(0);
		}

		template<class T>
		inline void set_variable(uint name, T v)
		{
			if (auto it = variables.find(name); it != variables.end())
				*(T*)it->second.data = v;
		}

		virtual ~BlueprintInstance() {}


		inline Group* get_group(uint name) const
		{
			auto it = groups.find(name);
			if (it == groups.end())
				return nullptr;
			return (Group*)&it->second;
		}

		virtual void build() = 0;
		virtual void prepare_executing(Group* group) = 0;
		virtual void run(Group* group) = 0;
		virtual Node* step(Group* group) = 0; // return: next node
		virtual void stop(Group* group) = 0;
		virtual void call(uint group_name, void** inputs, void** outputs) = 0;

		struct Create
		{
			virtual BlueprintInstancePtr operator()(BlueprintPtr blueprint) = 0;
		};
		// Reflect static
		FLAME_FOUNDATION_API static Create& create;

		struct Get
		{
			virtual BlueprintInstancePtr operator()(uint name) = 0;
		};
		// Reflect static
		FLAME_FOUNDATION_API static Get& get;
	};

	struct BlueprintDebugger
	{
		std::vector<std::pair<BlueprintNodePtr, BlueprintBreakpointOption>> break_nodes;
		Listeners<void(uint, void*, void*)> callbacks;

		BlueprintInstance::Group* debugging = nullptr;

		virtual ~BlueprintDebugger() {}

		inline bool has_break_node(BlueprintNodePtr node, BlueprintBreakpointOption* out_option = nullptr) const
		{
			for (auto& i : break_nodes)
			{
				if (i.first == node)
				{
					if (out_option)
						*out_option = i.second;
					return true;
				}
			}
			return false;
		}

		virtual void add_break_node(BlueprintNodePtr node, BlueprintBreakpointOption option = BlueprintBreakpointNormal) = 0;
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
