#pragma once

#include "foundation.h"

namespace flame
{
	struct BlueprintSlot
	{
		std::string name;
		uint name_hash = 0;
		std::vector<TypeInfo*> allowed_types; // output slot can only have one type
		TypeInfo* type = nullptr;
		void* data = nullptr;
	};

	struct BlueprintArgument
	{
		TypeInfo* type;
		void* data;
	};

	typedef void(*BlueprintNodeFunction)(const BlueprintArgument* inputs, BlueprintArgument* outputs);
	typedef void(*BlueprintNodeInputSlotChangedCallback)(const TypeInfo** input_types, TypeInfo** output_types);

	struct BlueprintNode
	{
		BlueprintGroupPtr group;
		std::string name;
		uint name_hash = 0;
		std::vector<BlueprintSlot> inputs;
		std::vector<BlueprintSlot> outputs;
		BlueprintNodeFunction function = nullptr;
		BlueprintNodeInputSlotChangedCallback input_changed_callback = nullptr;

		vec2 position;
		bool collapsed = false;

		inline const BlueprintSlot* find_input(uint name) const
		{
			for (auto& i : inputs)
			{
				if (i.name_hash == name)
					return &i;
			}
			return nullptr;
		}

		inline const BlueprintSlot* find_output(uint name) const
		{
			for (auto& o : outputs)
			{
				if (o.name_hash == name)
					return &o;
			}
			return nullptr;
		}
	};

	struct BlueprintLink
	{
		BlueprintNodePtr		from_node;
		const BlueprintSlot*	from_slot;
		BlueprintNodePtr		to_node;
		const BlueprintSlot*	to_slot;
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
	};

	// Reflect ctor
	struct Blueprint
	{
		// Reflect
		std::vector<std::unique_ptr<BlueprintGroupT>> groups;

		uint dirty_frame;

		std::filesystem::path filename;
		uint ref = 0;

		virtual BlueprintNodePtr	add_node(BlueprintGroupPtr group /*null means the main group*/, const std::string& name, 
			const std::vector<BlueprintSlot>& inputs = {}, const std::vector<BlueprintSlot>& outputs = {}, BlueprintNodeFunction function = nullptr) = 0;
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

	struct BlueprintInstance
	{
		BlueprintPtr blueprint;

		virtual void set_group(uint group_name) = 0;
		virtual void run() = 0;
		virtual void step() = 0;

		struct Create
		{
			virtual BlueprintInstancePtr operator()(BlueprintPtr blueprint) = 0;
		};
		// Reflect static
		FLAME_FOUNDATION_API static Create& create;
	};
}
