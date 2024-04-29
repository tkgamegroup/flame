#pragma once

#include "foundation.h"
#include "typeinfo.h"

namespace flame
{
	enum BlueprintNodeFlags
	{
		BlueprintNodeFlagNone = 0,
		BlueprintNodeFlagEnableTemplate = 1 << 0,
		BlueprintNodeFlagBreakTarget = 1 << 1,
		BlueprintNodeFlagReturnTarget = 1 << 2,
		BlueprintNodeFlagWidget = 1 << 3,
		BlueprintNodeFlagHorizontalInputs = 1 << 4,
		BlueprintNodeFlagHorizontalOutputs = 1 << 5
	};

	inline BlueprintNodeFlags operator|(BlueprintNodeFlags a, BlueprintNodeFlags b)
	{
		return (BlueprintNodeFlags)((uint)a | (uint)b);
	}

	inline BlueprintNodeFlags operator&(BlueprintNodeFlags a, BlueprintNodeFlags b)
	{
		return (BlueprintNodeFlags)((uint)a & (uint)b);
	}

	enum BlueprintSlotFlags
	{
		BlueprintSlotFlagNone = 0,
		BlueprintSlotFlagInput = 1 << 0,
		BlueprintSlotFlagOutput = 1 << 1,
		BlueprintSlotFlagBeginWidget = 1 << 2,
		BlueprintSlotFlagEndWidget = 1 << 3,
		BlueprintSlotFlagHideInUI = 1 << 4
	};

	inline BlueprintSlotFlags operator|(BlueprintSlotFlags a, BlueprintSlotFlags b)
	{
		return (BlueprintSlotFlags)((uint)a | (uint)b);
	}

	inline BlueprintSlotFlags operator&(BlueprintSlotFlags a, BlueprintSlotFlags b)
	{
		return (BlueprintSlotFlags)((uint)a & (uint)b);
	}

	enum BlueprintVariableFlags
	{
		BlueprintVariableFlagNone = 0,
		BlueprintVariableFlagAttribute = 1 << 0,
	};

	enum BlueprintNodeStructureChangeReason
	{
		BlueprintNodeInputTypesChanged,
		BlueprintNodeTemplateChanged
	};

	enum BlueprintExecutionType
	{
		BlueprintExecutionFunction,
		BlueprintExecutionCoroutine
	};

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
		TypeInfo*	type;
		void*		data;
	};

	struct BlueprintSlotDesc
	{
		std::string				name;
		uint					name_hash;
		BlueprintSlotFlags		flags = BlueprintSlotFlagNone;
		std::vector<TypeInfo*>	allowed_types;
		std::string				default_value;
	};

	struct BlueprintEnumItem
	{
		std::string	name;
		uint		name_hash;
		int			value;
	};

	struct BlueprintEnum
	{
		std::string						name;
		uint							name_hash;
		std::vector<BlueprintEnumItem>	items;
	};

	struct BlueprintStructVariable
	{
		std::string name;
		uint		name_hash;
		TypeInfo*	type;
		std::string default_value;
	};

	struct BlueprintStruct
	{
		std::string								name;
		uint									name_hash;
		std::vector<BlueprintStructVariable>	variables;
	};

	struct BlueprintExecutionData;
	struct BlueprintInstance;
	struct BlueprintExecutingBlock;

	struct BlueprintSystem
	{
		FLAME_FOUNDATION_API static std::vector<std::pair<std::string, TypeInfo*>> template_types;
	};

	inline bool blueprint_allow_type(const std::vector<TypeInfo*>& allowed_types, TypeInfo* type)
	{
		if (type == TypeInfo::get<voidptr>())
			return true;
		for (auto t : allowed_types)
		{
			if (t == type)
				return true;
			if (t == TypeInfo::get<voidptr>())
				return true;
			if (t == TypeInfo::get<uint>() && type == TypeInfo::get<int>() ||
				t == TypeInfo::get<int>() && type == TypeInfo::get<uint>())
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

	inline bool blueprint_allow_any_type(const std::vector<TypeInfo*>& allowed_types, const std::vector<TypeInfo*> type_list)
	{
		for (auto t : type_list)
		{
			if (blueprint_allow_type(allowed_types, t))
				return true;
		}
		return false;
	}

	inline bool blueprint_is_variable_node(uint name)
	{
		return name == "Variable"_h ||
			name == "Set Variable"_h ||
			name == "Add Assign"_h ||
			name == "Subtrac Assign"_h ||
			name == "Multiply Assign"_h ||
			name == "Divide Assign"_h ||
			name == "Or Assign"_h ||
			name == "And Assign"_h ||
			name == "Get Property"_h ||
			name == "Get Properties"_h ||
			name == "Set Property"_h ||
			name == "Array Size"_h ||
			name == "Array Clear"_h ||
			name == "Array Get Item"_h ||
			name == "Array Set Item"_h ||
			name == "Array Get Item Property"_h ||
			name == "Array Get Item Properties"_h ||
			name == "Array Set Item Property"_h ||
			name == "Array Add Item"_h ||
			name == "Array Emplace Item"_h;
	}

	inline TypeInfo* blueprint_type_from_template_str(std::string_view str)
	{
		TypeInfo* type = nullptr;
		for (auto& t : BlueprintSystem::template_types)
		{
			if (str == t.first)
			{
				type = t.second;
				break;
			}
		}
		return type;
	}

	struct BlueprintSlot
	{
		uint					object_id;
		BlueprintNodePtr		node;
		std::string				name;
		uint					name_hash = 0;
		BlueprintSlotFlags		flags = BlueprintSlotFlagNone;
		std::vector<TypeInfo*>	allowed_types;
		TypeInfo* type = nullptr;
		void* data = nullptr;
		std::string				default_value;
		uint					data_changed_frame = 0;

		virtual uint get_linked_count() const = 0;
		virtual BlueprintSlotPtr get_linked(uint idx) const = 0;
	};

	struct BlueprintNodePreview
	{
		uint type;
		uvec2 extent;
		void* data;
	};

	struct BlueprintNodeStructureChangeInfo;

	typedef void(*BlueprintNodeFunction)(uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs);
	typedef void(*BlueprintNodeLoopFunction)(uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution);
	typedef void(*BlueprintNodeBeginBlockFunction)(uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintExecutionData& execution);
	typedef void(*BlueprintNodeEndBlockFunction)(uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs);
	typedef void(*BlueprintNodeConstructor)(uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs);
	typedef void(*BlueprintNodeDestructor)(uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs);
	typedef bool(*BlueprintNodeChangeStructureCallback)(BlueprintNodeStructureChangeInfo& info);
	typedef void(*BlueprintNodePreviewProvider)(uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs, BlueprintNodePreview* preview);

	struct BlueprintNodeStructureChangeInfo
	{
		BlueprintNodeStructureChangeReason reason;
		std::string template_string;
		std::vector<TypeInfo*> input_types;
		std::vector<TypeInfo*> output_types;
		std::vector<BlueprintSlotDesc> new_inputs;
		std::vector<BlueprintSlotDesc> new_outputs;

		BlueprintNodeFunction new_function;
		BlueprintNodeLoopFunction new_loop_function;
		BlueprintNodeBeginBlockFunction new_begin_block_function;
		BlueprintNodeEndBlockFunction new_end_block_function;
	};

	struct BlueprintNode
	{
		uint											object_id;
		BlueprintGroupPtr								group;

		std::string										name;
		uint											name_hash = 0;
		BlueprintNodeFlags								flags = BlueprintNodeFlagNone;
		std::string										display_name;
		std::string										template_string;
		std::vector<std::unique_ptr<BlueprintSlotT>>	inputs;
		std::vector<std::unique_ptr<BlueprintSlotT>>	outputs;
		BlueprintNodeFunction							function = nullptr;
		BlueprintNodeLoopFunction						loop_function = nullptr;
		BlueprintNodeConstructor						constructor = nullptr;
		BlueprintNodeDestructor							destructor = nullptr;
		BlueprintNodeChangeStructureCallback			change_structure_callback = nullptr;
		BlueprintNodePreviewProvider					preview_provider = nullptr;
		bool											is_block = false;
		BlueprintNodeBeginBlockFunction					begin_block_function = nullptr;
		BlueprintNodeEndBlockFunction					end_block_function = nullptr;

		BlueprintNodePtr								parent;
		uint											depth = 0;
		std::vector<BlueprintNodePtr>					children;
		uint											degree = 0;
		bool											hide_defaults = false;

		vec2 position;
		bool collapsed = false;

		virtual ~BlueprintNode() {}

		inline int find_input_i(std::string_view name) const
		{
			for (auto i = 0; i < inputs.size(); i++)
			{
				if (((BlueprintSlot*)inputs[i].get())->name == name)
					return i;
			}
			return -1;
		}

		inline int find_input_i(uint name_hash) const
		{
			for (auto i = 0; i < inputs.size(); i++)
			{
				if (((BlueprintSlot*)inputs[i].get())->name_hash == name_hash)
					return i;
			}
			return -1;
		}

		inline BlueprintSlotPtr find_input(std::string_view name) const
		{
			auto idx = find_input_i(name);
			return idx != -1 ? (BlueprintSlotPtr)inputs[idx].get() : nullptr;
		}

		inline BlueprintSlotPtr find_input(uint name_hash) const
		{
			auto idx = find_input_i(name_hash);
			return idx != -1 ? (BlueprintSlotPtr)inputs[idx].get() : nullptr;
		}

		inline int find_output_i(std::string_view name) const
		{
			for (auto i = 0; i < outputs.size(); i++)
			{
				if (((BlueprintSlot*)outputs[i].get())->name == name)
					return i;
			}
			return -1;
		}

		inline int find_output_i(uint name_hash) const
		{
			for (auto i = 0; i < outputs.size(); i++)
			{
				if (((BlueprintSlot*)outputs[i].get())->name_hash == name_hash)
					return i;
			}
			return -1;
		}

		inline BlueprintSlotPtr find_output(std::string_view name) const
		{
			auto idx = find_output_i(name);
			return idx != -1 ? (BlueprintSlotPtr)outputs[idx].get() : nullptr;
		}

		inline BlueprintSlotPtr find_output(uint name_hash) const
		{
			auto idx = find_output_i(name_hash);
			return idx != -1 ? (BlueprintSlotPtr)outputs[idx].get() : nullptr;
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

	inline void blueprint_form_top_list(std::vector<BlueprintNodePtr>& list, BlueprintNodePtr node)
	{
		auto _n = (BlueprintNode*)node;
		for (auto it = list.begin(); it != list.end();)
		{
			auto n = (BlueprintNode*)*it;
			if (_n->depth < n->depth && _n->contains((BlueprintNodePtr)n))
				it = list.erase(it);
			else
				it++;
		}
		for (auto i = 0; i < list.size(); i++)
		{
			auto n = (BlueprintNode*)list[i];
			if (_n->depth > n->depth && n->contains((BlueprintNodePtr)_n))
			{
				_n = nullptr;
				break;
			}
		}
		if (_n)
			list.push_back((BlueprintNodePtr)_n);
	}

	struct BlueprintLink
	{
		uint				object_id;

		BlueprintSlotPtr	from_slot;
		BlueprintSlotPtr	to_slot;

		virtual ~BlueprintLink() {}
	};

	struct BlueprintVariable
	{
		std::string				name;
		uint					name_hash = 0;
		BlueprintVariableFlags	flags = BlueprintVariableFlagNone;
		TypeInfo*				type = nullptr;
		void*					data = nullptr;
	};

	struct BlueprintInvalidNode
	{
		std::string name;
		std::vector<std::string> inputs;
		std::vector<std::string> outputs;
		vec2 position;
	};

	enum BlueprintInvalidReason
	{
		BlueprintInvalidNone = 0,
		BlueprintInvalidName = 1 << 0,
		BlueprintInvalidType = 1 << 1,
		BlueprintInvalidFromNode = 1 << 2,
		BlueprintInvalidFromSlot = 1 << 3,
		BlueprintInvalidToNode = 1 << 4,
		BlueprintInvalidToSlot = 1 << 5
	};

	inline BlueprintInvalidReason operator|(BlueprintInvalidReason a, BlueprintInvalidReason b)
	{
		return (BlueprintInvalidReason)((uint)a | (uint)b);
	}

	inline BlueprintInvalidReason operator&(BlueprintInvalidReason a, BlueprintInvalidReason b)
	{
		return (BlueprintInvalidReason)((uint)a & (uint)b);
	}

	struct BlueprintInvalidInput
	{
		BlueprintInvalidReason reason;

		uint node;
		std::string name;
		std::string type;
		std::string value;
	};

	struct BlueprintInvalidLink
	{
		BlueprintInvalidReason reason;

		uint from_node;
		uint from_slot;
		std::string from_slot_name;
		uint to_node;
		uint to_slot;
		std::string to_slot_name;
	};

	FLAME_FOUNDATION_API void set_blueprint_refactoring_environment(bool is_preview, std::string* out_log);

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
		std::vector<float>										splits;

		std::vector<BlueprintInvalidNode> 						invalid_nodes;
		std::vector<BlueprintInvalidInput> 						invalid_inputs;
		std::vector<BlueprintInvalidLink>						invalid_links;

		bool 													responsive = false;

		inline BlueprintVariable* find_variable(uint name) const
		{
			for (auto& v : variables)
			{
				if (v.name_hash == name)
					return (BlueprintVariable*)&v;
			}
			return nullptr;
		}

		inline BlueprintNodePtr find_node(uint name) const
		{
			for (auto& n : nodes)
			{
				if (((BlueprintNode*)n.get())->name_hash == name)
					return (BlueprintNodePtr)n.get();
			}
			return nullptr;
		}

		inline BlueprintNodePtr find_node_by_id(uint object_id) const
		{
			for (auto& n : nodes)
			{
				if (((BlueprintNode*)n.get())->object_id == object_id)
					return (BlueprintNodePtr)n.get();
			}
			return nullptr;
		}

		inline BlueprintLinkPtr find_link(BlueprintSlotPtr from_slot, BlueprintSlotPtr to_slot) const
		{
			for (auto& l : links)
			{
				if (((BlueprintLink*)l.get())->from_slot == from_slot && ((BlueprintLink*)l.get())->to_slot == to_slot)
					return (BlueprintLinkPtr)l.get();
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

	struct Blueprint
	{
		std::filesystem::path							super_filename;
		BlueprintPtr									super = nullptr;

		std::vector<BlueprintEnum>						enums;
		std::vector<BlueprintStruct>					structs;

		std::vector<BlueprintVariable>					variables;
		std::vector<std::unique_ptr<BlueprintGroupT>>	groups;

		uint											variable_changed_frame = 1;
		uint											dirty_frame = 1;

		std::filesystem::path							filename;
		std::string										name;
		uint											name_hash;
		bool											is_static = false;
		uint											ref = 0;

		inline BlueprintVariable* find_variable(uint name) const
		{
			for (auto& v : variables)
			{
				if (v.name_hash == name)
					return (BlueprintVariable*)&v;
			}
			return nullptr;
		}

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

		virtual void					set_super(const std::filesystem::path& filename) = 0;

		virtual BlueprintEnum*			add_enum(const std::string& name, const std::vector<BlueprintEnumItem>& items) = 0;
		virtual void					remove_enum(uint name) = 0;
		virtual void					alter_enum(uint old_name, const std::string& new_name, const std::vector<BlueprintEnumItem>& new_items) = 0;
		virtual BlueprintStruct*		add_struct(const std::string& name, const std::vector<BlueprintStructVariable>& variables) = 0;
		virtual void					remove_struct(uint name) = 0;
		virtual void					alter_struct(uint old_name, const std::string& new_name, const std::vector<BlueprintStructVariable>& new_variables) = 0;

		virtual BlueprintVariable*		add_variable(BlueprintGroupPtr group /* or null for blueprint variable */, const std::string& name, TypeInfo* type) = 0; // return: the data of the variable
		virtual void					remove_variable(BlueprintGroupPtr group /* or null for blueprint variable */, uint name) = 0;
		virtual void					alter_variable(BlueprintGroupPtr group /* or null for blueprint variable */, uint old_name, const std::string& new_name = "", TypeInfo* new_type = nullptr) = 0;
		virtual BlueprintNodePtr		add_node(BlueprintGroupPtr group, BlueprintNodePtr parent, const std::string& name, BlueprintNodeFlags flags, const std::string& display_name,
			const std::vector<BlueprintSlotDesc>& inputs = {}, const std::vector<BlueprintSlotDesc>& outputs = {},
			BlueprintNodeFunction function = nullptr, BlueprintNodeLoopFunction loop_function = nullptr,
			BlueprintNodeConstructor constructor = nullptr, BlueprintNodeDestructor destructor = nullptr,
			BlueprintNodeChangeStructureCallback change_structure_callback = nullptr, BlueprintNodePreviewProvider preview_provider = nullptr,
			bool is_block = false, BlueprintNodeBeginBlockFunction begin_block_function = nullptr, BlueprintNodeEndBlockFunction end_block_function = nullptr) = 0;
		virtual BlueprintNodePtr		add_node(BlueprintGroupPtr group, BlueprintNodePtr parent, uint name_hash) = 0;
		virtual BlueprintNodePtr		add_block(BlueprintGroupPtr group, BlueprintNodePtr parent) = 0;
		virtual BlueprintNodePtr		add_variable_node(BlueprintGroupPtr group, BlueprintNodePtr parent, uint variable_name, uint type = "Variable"_h, uint location_name = 0 /* a static bp or enum */, uint property_name = 0) = 0;
		virtual BlueprintNodePtr		add_call_node(BlueprintGroupPtr group, BlueprintNodePtr parent, uint group_name, uint location_name = 0 /* from other static bp */) = 0; // add a node that will call another group
		virtual void					remove_node(BlueprintNodePtr node, bool recursively = true) = 0;
		virtual void					set_nodes_parent(const std::vector<BlueprintNodePtr> nodes, BlueprintNodePtr new_parent) = 0;
		virtual bool					change_node_structure(BlueprintNodePtr node, const std::string& new_template_string, const std::vector<TypeInfo*>& new_input_types) = 0;
		virtual bool					change_references(BlueprintGroupPtr group, uint old_name, uint old_location, uint old_property, uint new_name, uint new_location, uint new_property) = 0;
		virtual BlueprintLinkPtr		add_link(BlueprintSlotPtr from_slot, BlueprintSlotPtr to_slot) = 0;
		virtual void					remove_link(BlueprintLinkPtr link) = 0;
		virtual BlueprintGroupPtr		add_group(const std::string& name) = 0;
		virtual void					remove_group(BlueprintGroupPtr group) = 0;
		virtual void					alter_group(uint old_name, const std::string& new_name) = 0;
		virtual void					add_group_input(BlueprintGroupPtr group, const std::string& name, TypeInfo* type) = 0;
		virtual void					remove_group_input(BlueprintGroupPtr group, uint name) = 0;
		virtual void					alter_group_input(BlueprintGroupPtr group, uint old_name, const std::string& new_name = "", TypeInfo* new_type = nullptr) = 0;
		virtual void					add_group_output(BlueprintGroupPtr group, const std::string& name, TypeInfo* type) = 0;
		virtual void					remove_group_output(BlueprintGroupPtr group, uint name) = 0;
		virtual void					alter_group_output(BlueprintGroupPtr group, uint old_name, const std::string& new_name = "", TypeInfo* new_type = nullptr) = 0;

		virtual void					load(const std::filesystem::path& path, bool load_typeinfos = false) = 0;
		virtual void					save(const std::filesystem::path& path = L"") = 0;

		struct Create
		{
			virtual BlueprintPtr operator()(bool empty = false) = 0;
		};
		FLAME_FOUNDATION_API static Create& create;

		struct Destroy
		{
			virtual void operator()(BlueprintPtr bp) = 0;
		};
		FLAME_FOUNDATION_API static Destroy& destroy;

		struct Get
		{
			virtual BlueprintPtr operator()(const std::filesystem::path& filename, bool is_static = false) = 0;
			virtual BlueprintPtr operator()(uint name) = 0;
		};
		FLAME_FOUNDATION_API static Get& get;

		struct Release
		{
			virtual void operator()(BlueprintPtr blueprint) = 0;
		};
		FLAME_FOUNDATION_API static Release& release;
	};

	struct BlueprintNodeLibrary
	{
		struct NodeTemplate
		{
			BlueprintNodeLibraryPtr					library;
			std::string								name;
			uint									name_hash = 0;
			BlueprintNodeFlags						flags = BlueprintNodeFlagNone;
			std::string								display_name;
			std::vector<BlueprintSlotDesc>			inputs;
			std::vector<BlueprintSlotDesc>			outputs;
			BlueprintNodeFunction					function = nullptr;
			BlueprintNodeLoopFunction				loop_function = nullptr;
			BlueprintNodeConstructor				constructor = nullptr;
			BlueprintNodeDestructor					destructor = nullptr;
			BlueprintNodeChangeStructureCallback	change_structure_callback = nullptr;
			BlueprintNodePreviewProvider			preview_provider = nullptr;
			bool									is_block = false;
			BlueprintNodeBeginBlockFunction			begin_block_function = nullptr;
			BlueprintNodeEndBlockFunction			end_block_function = nullptr;

			inline BlueprintNodePtr create_node(BlueprintPtr blueprint, BlueprintGroupPtr group, BlueprintNodePtr parent) const
			{
				return ((Blueprint*)blueprint)->add_node(group, parent, name, flags, display_name, inputs, outputs, function, loop_function,
					constructor, destructor, change_structure_callback, preview_provider, is_block, begin_block_function, end_block_function);
			}
		};

		std::vector<NodeTemplate>	node_templates;

		inline NodeTemplate* find_node_template(uint name) const
		{
			for (auto& t : node_templates)
			{
				if (t.name_hash == name)
					return (NodeTemplate*)&t;
			}
			return nullptr;
		}

		std::filesystem::path		filename;
		std::string					name;
		uint						name_hash = 0;
		uint						ref = 0;

		virtual ~BlueprintNodeLibrary() {}

		virtual void add_template(const std::string& name, const std::string& display_name, BlueprintNodeFlags flags = BlueprintNodeFlagNone,
			const std::vector<BlueprintSlotDesc>& inputs = {}, const std::vector<BlueprintSlotDesc>& outputs = {},
			BlueprintNodeFunction function = nullptr, BlueprintNodeConstructor constructor = nullptr, BlueprintNodeDestructor destructor = nullptr,
			BlueprintNodeChangeStructureCallback change_structure_callback = nullptr, BlueprintNodePreviewProvider preview_provider = nullptr) = 0;
		virtual void add_template(const std::string& name, const std::string& display_name, BlueprintNodeFlags flags = BlueprintNodeFlagNone,
			const std::vector<BlueprintSlotDesc>& inputs = {}, const std::vector<BlueprintSlotDesc>& outputs = {},
			BlueprintNodeLoopFunction loop_function = nullptr, BlueprintNodeConstructor constructor = nullptr, BlueprintNodeDestructor destructor = nullptr,
			BlueprintNodeChangeStructureCallback change_structure_callback = nullptr, BlueprintNodePreviewProvider preview_provider = nullptr) = 0;
		virtual void add_template(const std::string& name, const std::string& display_name, BlueprintNodeFlags flags = BlueprintNodeFlagNone,
			const std::vector<BlueprintSlotDesc>& inputs = {}, const std::vector<BlueprintSlotDesc>& outputs = {},
			bool is_block = true, BlueprintNodeBeginBlockFunction begin_block_function = nullptr, BlueprintNodeEndBlockFunction end_block_function = nullptr,
			BlueprintNodeLoopFunction loop_function = nullptr, BlueprintNodeConstructor constructor = nullptr, BlueprintNodeDestructor destructor = nullptr,
			BlueprintNodeChangeStructureCallback change_structure_callback = nullptr, BlueprintNodePreviewProvider preview_provider = nullptr) = 0;

		struct Get
		{
			virtual BlueprintNodeLibraryPtr operator()(const std::filesystem::path& filename /* L"standard" for standard */) = 0;
		};
		FLAME_FOUNDATION_API static Get& get;
	};

	struct BlueprintInstanceNode
	{
		uint									object_id;
		BlueprintNodePtr						original;
		BlueprintNodeDestructor					destructor;
		std::vector<BlueprintAttribute>			inputs;
		std::vector<BlueprintAttribute>			outputs;
		std::vector<BlueprintInstanceNode>		children;
		uint									order;
		uint									updated_frame;
	};

	struct BlueprintExecutingBlock
	{
		BlueprintExecutingBlock* parent = nullptr;
		BlueprintInstanceNode* node = nullptr;
		int		child_index = -1;
		uint	executed_times = 0;
		uint	max_execute_times = 0;
		int		loop_vector_index = -1; // in block node's inputs or outputs, index < inputs.size() means input, otherwise output
		int		block_output_index = -1; // in block node's inputs or outputs, index < inputs.size() means input, otherwise output

		inline void _break()
		{
			child_index = 99999;
			max_execute_times = 0;
		}
	};

	struct BlueprintInstanceGroup
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

		BlueprintExecutionType							execution_type;
		bool 											trigger_by_message = false;
		std::map<uint, Data>							slot_datas; // key: slot id
		BlueprintInstanceNode							root_node;
		std::map<uint, BlueprintInstanceNode*>			node_map;
		BlueprintInstanceNode*							input_node = nullptr;
		BlueprintInstanceNode*							output_node = nullptr;
		std::unordered_map<uint, BlueprintAttribute>	variables; // key: variable name hash

		std::list<BlueprintExecutingBlock>				executing_stack;

		uint											variable_updated_frame = 0;
		uint											structure_updated_frame = 0;
		uint											data_updated_frame = 0;
		float											wait_time = 0.f;

		inline BlueprintAttribute get_variable(uint name)
		{
			if (auto it = variables.find(name); it != variables.end())
				return it->second;
			return BlueprintAttribute{ nullptr, nullptr };
		}

		template<class T>
		inline T get_variable_as(uint name, T dv = T(0))
		{
			if (auto it = variables.find(name); it != variables.end())
				return *(T*)it->second.data;
			return dv;
		}

		template<class T>
		inline T get_ith_variable_as(uint idx)
		{
			*(T*)variables[idx].data;
		}

		template<class T>
		inline void set_variable_as(uint name, T v)
		{
			if (auto it = variables.find(name); it != variables.end())
				*(T*)it->second.data = v;
		}

		template<class T>
		inline void set_ith_variable_as(uint idx, T v)
		{
			*(T*)variables[idx].data = v;
		}

		inline void create_variable(uint name, TypeInfo* type, void* data)
		{
			if (auto it = variables.find(name); it != variables.end())
				type->copy(it->second.data, data);
			else
			{
				auto& arg = variables[name];
				arg.type = type;
				arg.data = arg.type->create();
				type->copy(arg.data, data);
			}
		}

		inline void reset_all_variables()
		{
			for (auto& v : variables)
			{
				if (v.second.type->tag != TagU)
					v.second.type->create(v.second.data);
			}
		}

		inline BlueprintInstanceNode* executing_node() const
		{
			if (executing_stack.empty())
				return nullptr;
			auto& current_block = executing_stack.back();
			return &current_block.node->children[current_block.child_index];
		}
	};

	// Reflect
	struct BlueprintInstance
	{
		BlueprintPtr blueprint;
		bool is_static = false;

		std::unordered_map<uint, BlueprintAttribute>		variables; // key: variable name hash
		std::unordered_map<uint, BlueprintInstanceGroup>	groups; // key: group name hash

		uint variable_updated_frame = 0;
		uint built_frame = 0;

		inline BlueprintAttribute get_variable(uint name)
		{
			if (auto it = variables.find(name); it != variables.end())
				return it->second;
			return BlueprintAttribute{ nullptr, nullptr };
		}

		template<class T>
		inline T get_variable_as(uint name)
		{
			if (auto it = variables.find(name); it != variables.end())
				return *(T*)it->second.data;
			return T(0);
		}

		template<class T>
		inline T get_ith_variable_as(uint idx)
		{
			*(T*)variables[idx].data;
		}

		template<class T>
		inline void set_variable_as(uint name, T v)
		{
			if (auto it = variables.find(name); it != variables.end())
				*(T*)it->second.data = v;
		}

		template<class T>
		inline void set_ith_variable_as(uint idx, T v)
		{
			*(T*)variables[idx].data = v;
		}

		inline void create_variable(uint name, TypeInfo* type, void* data)
		{
			if (auto it = variables.find(name); it != variables.end())
				type->copy(it->second.data, data);
			else
			{
				auto& arg = variables[name];
				arg.type = type;
				arg.data = arg.type->create();
				type->copy(arg.data, data);
			}
		}

		virtual ~BlueprintInstance() {}

		inline BlueprintInstanceGroup* find_group(uint name) const
		{
			auto it = groups.find(name);
			if (it == groups.end())
				return nullptr;
			return (BlueprintInstanceGroup*)&it->second;
		}
		
		virtual void build() = 0;
		virtual void prepare_executing(BlueprintInstanceGroup* group) = 0;
		virtual void run(BlueprintInstanceGroup* group) = 0;
		virtual BlueprintInstanceNode* step(BlueprintInstanceGroup* group) = 0; // return: next node
		virtual void stop(BlueprintInstanceGroup* group) = 0;
		virtual void call(BlueprintInstanceGroup* group, void** inputs, void** outputs) = 0;
		virtual void call(BlueprintInstanceGroup* group, const std::vector<std::pair<uint, void*>>& named_inputs, const std::vector<std::pair<uint, void*>>& named_outputs) = 0;

		struct Create
		{
			virtual BlueprintInstancePtr operator()(BlueprintPtr blueprint) = 0;
		};
		// Reflect static
		FLAME_FOUNDATION_API static Create& create;

		struct Destroy
		{
			virtual void operator()(BlueprintInstancePtr instance) = 0;
		};
		// Reflect static
		FLAME_FOUNDATION_API static Destroy& destroy;

		struct Get
		{
			virtual BlueprintInstancePtr operator()(uint name) = 0;
		};
		// Reflect static
		FLAME_FOUNDATION_API static Get& get;
	};

	struct BlueprintExecutionData
	{
		BlueprintInstanceGroup* group;
		BlueprintExecutingBlock* block;
	};

	struct BlueprintDebugger
	{
		std::vector<std::pair<BlueprintNodePtr, BlueprintBreakpointOption>> break_nodes;
		Listeners<void(uint, void*, void*)> callbacks;

		BlueprintInstanceGroup* debugging = nullptr;

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
		FLAME_FOUNDATION_API static Create& create;

		struct Current
		{
			virtual BlueprintDebuggerPtr operator()() = 0;
		};
		FLAME_FOUNDATION_API static Current& current;

		struct SetCurrent
		{
			virtual BlueprintDebuggerPtr operator()(BlueprintDebuggerPtr debugger) = 0;
		};
		FLAME_FOUNDATION_API static SetCurrent& set_current;
	};
}
