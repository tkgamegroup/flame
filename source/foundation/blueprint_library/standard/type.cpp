#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_type_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Array Size", "",
			{
				{
					.name = "Array",
					.allowed_types = { TypeInfo::get<voidptr>() }
				},
				{
					.name = "Array Type",
					.allowed_types = { TypeInfo::get<voidptr>() }
				}
			},
			{	
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				if (!inputs[0].data || !inputs[1].data)
					return;
				auto& array = *(std::vector<char>*)inputs[0].data;
				auto array_type = *(TypeInfo**)inputs[1].data;
				auto item_type = array_type->get_wrapped();
				*(uint*)outputs[0].data = array.size() / item_type->size;
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("Array Add Item", "",
			{
				{
					.name = "Array",
					.allowed_types = { TypeInfo::get<voidptr>() }
				},
				{
					.name = "Array Type",
					.allowed_types = { TypeInfo::get<voidptr>() }
				},
				{
					.name = "Item",
					.allowed_types = { TypeInfo::get<voidptr>() }
				},
			},
			{
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				if (!inputs[0].data || !inputs[1].data || !inputs[2].data)
					return;
				auto array_type = *(TypeInfo**)inputs[1].data;
				auto pitem = *(voidptr*)inputs[2].data;
				switch (array_type->tag)
				{
				case TagVD:
				{
					auto item_type = array_type->get_wrapped();
					if (item_type->pod)
					{
						auto& array = *(std::vector<char>*)inputs[0].data;
						array.resize(array.size() + item_type->size);
						memcpy(array.data() + array.size() - item_type->size, pitem, item_type->size);
					}
				}
					break;
				case TagVPU:
				{
					auto& array = *(std::vector<voidptr>*)inputs[0].data;
					array.push_back(pitem);
				}
					break;
				}
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);
	}
}
