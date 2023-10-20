#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_type_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("To Float", "",
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				*(float*)outputs[0].data = inputs[0].type->as_float(inputs[0].data);
			}
		);

		library->add_template("To Int", "",
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<int>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				*(int*)outputs[0].data = inputs[0].type->as_int(inputs[0].data);
			}
		);

		library->add_template("To Uint", "",
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				*(uint*)outputs[0].data = inputs[0].type->as_uint(inputs[0].data);
			}
		);


		library->add_template("Array Random Sample", "",
			{
				{
					.name = "Array",
					.allowed_types = { TypeInfo::get<voidptr>() }
				},
				{
					.name = "Value To Match",
					.allowed_types = { TypeInfo::get<float>(), TypeInfo::get<int>(), TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "Index",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto array_type = inputs[0].type;
				if (array_type && is_vector(array_type->tag))
				{
					auto parray = (std::vector<char>*)inputs[0].data;
					auto item_type = array_type->get_wrapped();
					int size = parray->size() / item_type->size;
					auto pmatch = inputs[1].data;
					switch (item_type->tag)
					{
					case TagD:
					{
						auto ti = (TypeInfo_Data*)item_type;
						if (ti->data_type == DataInt || ti->data_type == DataFloat)
						{
							auto& array = *(std::vector<uint>*)parray;
							std::vector<uint> temp;
							for (auto i = 0; i < size; i++)
							{
								if (memcmp(&array[i], pmatch, sizeof(uint)) == 0)
									temp.push_back(i);
							}
							if (!temp.empty())
								*(uint*)outputs[0].data = temp[linearRand(0, (int)temp.size() - 1)];
							else
								*(uint*)outputs[0].data = 0;
						}
						else
							*(uint*)outputs[0].data = 0;
					}
						break;
					default:
						*(uint*)outputs[0].data = 0;
					}
				}
				else
					*(uint*)outputs[0].data = 0;
			}
		);
	}
}
