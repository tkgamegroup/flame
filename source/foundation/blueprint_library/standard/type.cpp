#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_type_node_templates(BlueprintNodeLibraryPtr library)
	{
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
