#include "../../typeinfo_private.h"
#include "../../blueprint_private.h"

namespace flame
{
	void add_flow_control_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("If", "",
			{
				{
					.name = "Condition",
					.allowed_types = { TypeInfo::get<bool>(), TypeInfo::get<voidptr>() }
				}
			},
			{
				{
					.name = "True",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				},
				{
					.name = "False",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				bool ok;
				if (inputs[0].type == TypeInfo::get<bool>())
					ok = *(bool*)inputs[0].data;
				else
					ok = (*(voidptr*)inputs[0].data) != nullptr;
				(*(BlueprintSignal*)outputs[0].data).v = ok ? 1 : 0;
				(*(BlueprintSignal*)outputs[1].data).v = ok ? 0 : 1;
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("Loop", "",
			{
				{
					.name = "Times",
					.allowed_types = { TypeInfo::get<uint>() },
					.default_value = "1"
				}
			},
			{
				{
					.name = "Loop",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				(*(BlueprintSignal*)outputs[0].data).v = *(uint*)inputs[0].data;
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("Branch 2", "",
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Case 1",
					.allowed_types = { TypeInfo::get<uint>() }
				},
				{
					.name = "Case 2",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "Branch 1",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				},
				{
					.name = "Branch 2",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				},
				{
					.name = "Else",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto v = *(uint*)inputs[0].data;
				auto case1 = *(uint*)inputs[1].data;
				auto case2 = *(uint*)inputs[2].data;
				if (v == case1)
				{
					(*(BlueprintSignal*)outputs[0].data).v = 1;
					(*(BlueprintSignal*)outputs[1].data).v = 0;
					(*(BlueprintSignal*)outputs[2].data).v = 0;
				}
				else if (v == case2)
				{
					(*(BlueprintSignal*)outputs[0].data).v = 0;
					(*(BlueprintSignal*)outputs[1].data).v = 1;
					(*(BlueprintSignal*)outputs[2].data).v = 0;
				}
				else
				{
					(*(BlueprintSignal*)outputs[0].data).v = 0;
					(*(BlueprintSignal*)outputs[1].data).v = 0;
					(*(BlueprintSignal*)outputs[2].data).v = 1;
				}
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);

		library->add_template("Range Branch 2", "",
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<float>() }
				},
				{
					.name = "Stop 1",
					.allowed_types = { TypeInfo::get<float>() }
				},
				{
					.name = "Stop 2",
					.allowed_types = { TypeInfo::get<float>() }
				}
			},
			{
				{
					.name = "Branch 1",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				},
				{
					.name = "Branch 2",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				},
				{
					.name = "Else",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto v = *(float*)inputs[0].data;
				auto stop1 = *(float*)inputs[1].data;
				auto stop2 = *(float*)inputs[2].data;
				if (v < stop1)
				{
					(*(BlueprintSignal*)outputs[0].data).v = 1;
					(*(BlueprintSignal*)outputs[1].data).v = 0;
					(*(BlueprintSignal*)outputs[2].data).v = 0;
				}
				else if (v < stop2)
				{
					(*(BlueprintSignal*)outputs[0].data).v = 0;
					(*(BlueprintSignal*)outputs[1].data).v = 1;
					(*(BlueprintSignal*)outputs[2].data).v = 0;
				}
				else
				{
					(*(BlueprintSignal*)outputs[0].data).v = 0;
					(*(BlueprintSignal*)outputs[1].data).v = 0;
					(*(BlueprintSignal*)outputs[2].data).v = 1;
				}
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);



		library->add_template("Timer", "",
			{
				{
					.name = "Time",
					.flags = BlueprintSlotFlagHideInUI,
					.allowed_types = { TypeInfo::get<float>() }
				},
				{
					.name = "Interval",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "1"
				}
			},
			{
				{
					.name = "Fire",
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				}
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto& time = *(float*)inputs[0].data;
				auto interval = *(float*)inputs[1].data;

				time += delta_time;
				if (time >= interval)
				{
					(*(BlueprintSignal*)outputs[0].data).v = 1;
					*(float*)inputs[0].data = 0.f;
				}
				else
					(*(BlueprintSignal*)outputs[0].data).v = 0;
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);
	}
}
