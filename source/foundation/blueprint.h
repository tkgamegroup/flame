#pragma once

#include "foundation.h"
#include "typeinfo.h"

namespace flame
{
	enum BlueprintObjectType
	{
		BlueprintObjectSlot,
		BlueprintObjectNode,
		BlueprintObjectBlock,
		BlueprintObjectGroup
	};

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

	struct BlueprintSignal
	{
		uint v;
	};

	struct BlueprintAttribute
	{
		TypeInfo* type;
		void* data;
	};

	struct BlueprintObject
	{
		union
		{
			BlueprintSlotPtr slot;
			BlueprintNodePtr node;
			BlueprintBlockPtr block;
			BlueprintGroupPtr group;
		}p;

		BlueprintObjectType type : 8;

		BlueprintObject() {}
		BlueprintObject(BlueprintSlotPtr slot)
		{
			p.slot = slot;
			type = BlueprintObjectSlot;
		}
		BlueprintObject(BlueprintNodePtr node)
		{
			p.node = node;
			type = BlueprintObjectNode;
		}
		BlueprintObject(BlueprintBlockPtr block)
		{
			p.block = block;
			type = BlueprintObjectBlock;
		}
		BlueprintObject(BlueprintGroupPtr group)
		{
			p.group = group;
			type = BlueprintObjectGroup;
		}

		inline uint get_id() const;
		inline BlueprintGroupPtr get_locate_group() const;
		inline BlueprintBlockPtr get_locate_block() const;
		inline std::vector<BlueprintSlotPtr> get_inputs() const;
		inline std::vector<BlueprintSlotPtr> get_outputs() const;
		inline BlueprintSlotPtr find_input(uint name) const;
		inline BlueprintSlotPtr find_output(uint name) const;
	};

	struct BlueprintSlotDesc
	{
		std::string name;
		uint name_hash;
		BlueprintSlotFlags flags = BlueprintSlotFlagNone;
		std::vector<TypeInfo*> allowed_types;
		std::string default_value;
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
		uint object_id;
		BlueprintObject parent;
		std::string name;
		uint name_hash = 0;
		BlueprintSlotFlags flags = BlueprintSlotFlagNone;
		std::vector<TypeInfo*> allowed_types;
		TypeInfo* type = nullptr;
		void* data = nullptr;
		std::string default_value;
		uint data_changed_frame = 0;
	};

	struct BlueprintNodePreview
	{
		uint type;
		uvec2 extent;
		void* data;
	};

	typedef void(*BlueprintNodeFunction)(BlueprintAttribute* inputs, BlueprintAttribute* outputs);
	typedef void(*BlueprintNodeConstructor)(BlueprintAttribute* inputs, BlueprintAttribute* outputs);
	typedef void(*BlueprintNodeDestructor)(BlueprintAttribute* inputs, BlueprintAttribute* outputs);
	typedef void(*BlueprintNodeInputSlotChangedCallback)(TypeInfo** input_types, TypeInfo** output_types);
	typedef void(*BlueprintNodePreviewProvider)(BlueprintAttribute* inputs, BlueprintAttribute* outputs, BlueprintNodePreview* preview);

	struct BlueprintNode
	{
		uint object_id;
		BlueprintGroupPtr group;
		BlueprintBlockPtr block;

		std::string name;
		uint name_hash = 0;
		std::string display_name;
		std::vector<std::unique_ptr<BlueprintSlotT>> inputs;
		std::vector<std::unique_ptr<BlueprintSlotT>> outputs;
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
	};

	struct BlueprintBlock
	{
		uint object_id;
		BlueprintGroupPtr group;
		uint depth;

		std::vector<BlueprintNodePtr> nodes;
		BlueprintBlockPtr parent;
		std::vector<BlueprintBlockPtr> children;

		std::unique_ptr<BlueprintSlotT> input;
		std::unique_ptr<BlueprintSlotT> output;

		vec2 position;
		Rect rect;
		bool collapsed = false;

		virtual ~BlueprintBlock() {}
	};

	struct BlueprintVariable
	{
		std::string name;
		uint name_hash = 0;
		TypeInfo* type = nullptr;
		void* data = nullptr;
		std::string default_value;
	};

	struct BlueprintGroup
	{
		uint object_id;
		BlueprintPtr blueprint;

		std::string name;
		uint name_hash = 0;
		std::vector<BlueprintVariable>					variables;
		std::vector<BlueprintVariable>					inputs;
		std::vector<BlueprintVariable>					outputs;
		std::vector<std::unique_ptr<BlueprintNodeT>>	nodes;
		std::vector<std::unique_ptr<BlueprintLinkT>>	links;
		std::vector<std::unique_ptr<BlueprintBlockT>>	blocks;

		inline BlueprintNodePtr find_node(uint name) const
		{
			for (auto& n : nodes)
			{
				if (((BlueprintNode*)n.get())->name_hash == name)
					return (BlueprintNodePtr)n.get();
			}
			return nullptr;
		}

		vec2 offset;
		float scale = 1.f;

		uint structure_changed_frame = 0;
		uint data_changed_frame = 0;

		virtual ~BlueprintGroup() {}
	};

	struct BlueprintLink
	{
		uint object_id;

		BlueprintSlotPtr	from_slot;
		BlueprintSlotPtr	to_slot;

		virtual ~BlueprintLink() {}
	};

	inline uint BlueprintObject::get_id() const
	{
		switch (type)
		{
		case BlueprintObjectSlot: return ((BlueprintSlot*)p.slot)->object_id;
		case BlueprintObjectNode: return ((BlueprintNode*)p.node)->object_id;
		case BlueprintObjectBlock: return ((BlueprintBlock*)p.block)->object_id;
		case BlueprintObjectGroup: return ((BlueprintGroup*)p.group)->object_id;
		}
		return 0;
	}

	inline BlueprintBlockPtr BlueprintObject::get_locate_block() const
	{
		switch (type)
		{
		case BlueprintObjectNode: return ((BlueprintNode*)p.node)->block;
		case BlueprintObjectBlock: return ((BlueprintBlock*)p.block)->parent;
		}
		return nullptr;
	}

	inline BlueprintGroupPtr BlueprintObject::get_locate_group() const
	{
		switch (type)
		{
		case BlueprintObjectNode: return ((BlueprintNode*)p.node)->group;
		case BlueprintObjectBlock: return ((BlueprintBlock*)p.block)->group;
		}
		return nullptr;
	}

	inline std::vector<BlueprintSlotPtr> BlueprintObject::get_inputs() const
	{
		std::vector<BlueprintSlotPtr> ret;
		switch (type)
		{
		case BlueprintObjectNode: 
			for (auto& i : ((BlueprintNode*)p.node)->inputs)
				ret.push_back(i.get());
			break;
		case BlueprintObjectBlock: 
			ret.push_back(((BlueprintBlock*)p.block)->input.get());
			break;
		}
		return ret;
	}

	inline std::vector<BlueprintSlotPtr> BlueprintObject::get_outputs() const
	{
		std::vector<BlueprintSlotPtr> ret;
		switch (type)
		{
		case BlueprintObjectNode:
			for (auto& i : ((BlueprintNode*)p.node)->outputs)
				ret.push_back(i.get());
			break;
		case BlueprintObjectBlock:
			ret.push_back(((BlueprintBlock*)p.block)->output.get());
			break;
		}
		return ret;
	}

	inline BlueprintSlotPtr BlueprintObject::find_input(uint name) const
	{
		switch (type)
		{
		case BlueprintObjectNode: return ((BlueprintNode*)p.node)->find_input(name);
		case BlueprintObjectBlock: return ((BlueprintBlock*)p.block)->input.get();
		}
		return nullptr;
	}

	inline BlueprintSlotPtr BlueprintObject::find_output(uint name) const
	{
		switch (type)
		{
		case BlueprintObjectNode: return ((BlueprintNode*)p.node)->find_output(name);
		case BlueprintObjectBlock: return ((BlueprintBlock*)p.block)->output.get();
		}
		return nullptr;
	}

	// Reflect ctor
	struct Blueprint
	{
		std::vector<BlueprintVariable>					variables;

		// Reflect
		std::vector<std::unique_ptr<BlueprintGroupT>>	groups;

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
		virtual void					add_variable(BlueprintGroupPtr group /* or null for blueprint variable */, const std::string& name, TypeInfo* type, const std::string& default_value = "") = 0;
		virtual void					remove_variable(BlueprintGroupPtr group /* or null for blueprint variable */, uint name) = 0;
		virtual BlueprintNodePtr		add_node(BlueprintGroupPtr group, BlueprintBlockPtr block, const std::string& name, const std::string& display_name,
			const std::vector<BlueprintSlotDesc>& inputs = {}, const std::vector<BlueprintSlotDesc>& outputs = {},
			BlueprintNodeFunction function = nullptr, BlueprintNodeConstructor constructor = nullptr, BlueprintNodeDestructor destructor = nullptr,
			BlueprintNodeInputSlotChangedCallback input_slot_changed_callback = nullptr, BlueprintNodePreviewProvider preview_provider = nullptr) = 0;
		virtual BlueprintNodePtr		add_input_node(BlueprintGroupPtr group, BlueprintBlockPtr block, uint name) = 0;
		virtual BlueprintNodePtr		add_variable_node(BlueprintGroupPtr group, BlueprintBlockPtr block, uint variable_name) = 0;
		virtual void					remove_node(BlueprintNodePtr node) = 0;
		virtual void					set_node_block(BlueprintNodePtr node, BlueprintBlockPtr new_block) = 0;
		virtual BlueprintLinkPtr		add_link(BlueprintSlotPtr from_slot, BlueprintSlotPtr to_slot) = 0;
		virtual void					remove_link(BlueprintLinkPtr link) = 0;
		virtual BlueprintBlockPtr		add_block(BlueprintGroupPtr group, BlueprintBlockPtr parent) = 0;
		virtual void					remove_block(BlueprintBlockPtr block) = 0;
		virtual void					set_block_parent(BlueprintBlockPtr block, BlueprintBlockPtr new_parent) = 0;
		virtual BlueprintGroupPtr		add_group(const std::string& name) = 0;
		virtual void					remove_group(BlueprintGroupPtr group) = 0;
		virtual void					add_group_input(BlueprintGroupPtr group, const std::string& name, TypeInfo* type, const std::string& default_value = "") = 0;
		virtual void					remove_group_input(BlueprintGroupPtr group, uint name) = 0;
		virtual void					add_group_output(BlueprintGroupPtr group, const std::string& name, TypeInfo* type, const std::string& default_value = "") = 0;
		virtual void					remove_group_output(BlueprintGroupPtr group, uint name) = 0;

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
			std::string display_name;
			std::vector<BlueprintSlotDesc> inputs;
			std::vector<BlueprintSlotDesc> outputs;
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
		virtual void add_template(const std::string& name, const std::string& display_name, 
			const std::vector<BlueprintSlotDesc>& inputs = {}, const std::vector<BlueprintSlotDesc>& outputs = {},
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
		struct Object
		{
			uint object_id;
			BlueprintObject original;
			std::vector<BlueprintAttribute> inputs;
			std::vector<BlueprintAttribute> outputs;
			std::vector<Object> children;
			uint updated_frame;
		};

		struct Group
		{
			struct Data
			{
				BlueprintAttribute attribute;
				uint changed_frame = 0;
			};

			BlueprintInstancePtr instance;
			uint name;

			std::map<uint, Data>						slot_datas; // key: slot id
			Object										root_object;
			std::map<uint, Object*>						object_map;
			Object*										input_object = nullptr;
			Object*										output_object = nullptr;
			std::unordered_map<uint, BlueprintAttribute> variables; // key: variable name hash

			uint structure_updated_frame = 0;
			uint data_updated_frame = 0;

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
		};

		struct ExecutingBlock
		{
			Object* block_object;
			uint child_index;
			uint executed_times;
		};

		BlueprintPtr blueprint;

		std::unordered_map<uint, BlueprintAttribute> variables; // key: variable name hash
		std::unordered_map<uint, Group> groups; // key: group name hash
		Group*						executing_group = nullptr;
		std::vector<ExecutingBlock> executing_stack;

		uint built_frame;

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

		inline Object* executing_object() const
		{
			if (executing_stack.empty())
				return nullptr;
			auto& current_block = executing_stack.back();
			return &current_block.block_object->children[current_block.child_index];
		}

		virtual void build() = 0;
		virtual void prepare_executing(Group* group) = 0;
		virtual void run() = 0;
		virtual void step() = 0;
		virtual void stop() = 0;
		virtual void call(uint group_name, void** inputs, void** outputs) = 0;

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
