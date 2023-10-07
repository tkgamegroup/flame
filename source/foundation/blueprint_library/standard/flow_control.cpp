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

		library->add_template("If True", "",
			{
				{
					.name = "Condition",
					.allowed_types = { TypeInfo::get<bool>(), TypeInfo::get<voidptr>() }
				}
			},
			{
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			true,
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, uint* max_execute_times) {
				bool ok;
				if (inputs[0].type == TypeInfo::get<bool>())
					ok = *(bool*)inputs[1].data;
				else
					ok = (*(voidptr*)inputs[1].data) != nullptr;
				*max_execute_times = ok ? 1 : 0;
			}
		);

		library->add_template("If False", "",
			{
				{
					.name = "Condition",
					.allowed_types = { TypeInfo::get<bool>(), TypeInfo::get<voidptr>() }
				}
			},
			{
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			true,
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, uint* max_execute_times) {
				bool ok;
				if (inputs[0].type == TypeInfo::get<bool>())
					ok = *(bool*)inputs[1].data;
				else
					ok = (*(voidptr*)inputs[1].data) != nullptr;
				*max_execute_times = ok ? 0 : 1;
			}
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
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			true,
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, uint* max_execute_times) {
				*max_execute_times = *(uint*)inputs[1].data;
			}
		);

#define BRANCH_TEMPLATE(N) \
		{\
			std::vector<BlueprintSlotDesc> inputs;\
			std::vector<BlueprintSlotDesc> outputs;\
			for (auto i = 0; i < N; i++) \
			{\
				inputs.push_back({\
					.name = "Case " + std::to_string(i + 1),\
					.allowed_types = { TypeInfo::get<bool>() }\
				});\
			};\
			for (auto i = 0; i < N; i++) \
			{\
				outputs.push_back({\
					.name = "Branch " + std::to_string(i + 1),\
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }\
				});\
			};\
			outputs.push_back({\
				.name = "Else Branch",\
				.allowed_types = { TypeInfo::get<BlueprintSignal>() }\
			});\
			library->add_template(std::format("Branch {}", N), "",\
				inputs,\
				outputs,\
				[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {\
					for (auto i = 0; i < N + 1; i++) \
						(*(BlueprintSignal*)outputs[i].data).v = 0;\
					for (auto i = 0; i < N; i++) \
					{\
						if (*(bool*)inputs[i].data)\
						{\
							(*(BlueprintSignal*)outputs[i].data).v = 1;\
							return;\
						}\
					}\
					(*(BlueprintSignal*)outputs[N].data).v = 1;\
				},\
				nullptr,\
				nullptr,\
				nullptr,\
				nullptr\
			);\
		}

		BRANCH_TEMPLATE(2);
		BRANCH_TEMPLATE(3);
		BRANCH_TEMPLATE(4);
		BRANCH_TEMPLATE(5);
		BRANCH_TEMPLATE(6);
		BRANCH_TEMPLATE(7);
		BRANCH_TEMPLATE(8);

#undef BRANCH_TEMPLATE

#define SELECT_BRANCH_TEMPLATE(N) \
		{\
			std::vector<BlueprintSlotDesc> inputs;\
			std::vector<BlueprintSlotDesc> outputs;\
			inputs.push_back({\
				.name = "V",\
				.allowed_types = { TypeInfo::get<uint>() }\
			});\
			for (auto i = 0; i < N; i++) \
			{\
				inputs.push_back({\
					.name = "Case " + std::to_string(i + 1),\
					.allowed_types = { TypeInfo::get<uint>() }\
				});\
			};\
			for (auto i = 0; i < N; i++) \
			{\
				outputs.push_back({\
					.name = "Branch " + std::to_string(i + 1),\
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }\
				});\
			};\
			outputs.push_back({\
				.name = "Else Branch",\
				.allowed_types = { TypeInfo::get<BlueprintSignal>() }\
			});\
			library->add_template(std::format("Select Branch {}", N), "",\
				inputs,\
				outputs,\
				[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {\
					auto v = *(uint*)inputs[0].data;\
					for (auto i = 0; i < N + 1; i++) \
						(*(BlueprintSignal*)outputs[i].data).v = 0;\
					for (auto i = 0; i < N; i++) \
					{\
						if (v == *(uint*)inputs[i + 1].data)\
						{\
							(*(BlueprintSignal*)outputs[i].data).v = 1;\
							return;\
						}\
					}\
					(*(BlueprintSignal*)outputs[N].data).v = 1;\
				},\
				nullptr,\
				nullptr,\
				nullptr,\
				nullptr\
			);\
		}

		SELECT_BRANCH_TEMPLATE(2);
		SELECT_BRANCH_TEMPLATE(3);
		SELECT_BRANCH_TEMPLATE(4);
		SELECT_BRANCH_TEMPLATE(5);
		SELECT_BRANCH_TEMPLATE(6);
		SELECT_BRANCH_TEMPLATE(7);
		SELECT_BRANCH_TEMPLATE(8);

#undef SELECT_BRANCH_TEMPLATE

#define RAMP_BRANCH_TEMPLATE(N) \
		{\
			std::vector<BlueprintSlotDesc> inputs;\
			std::vector<BlueprintSlotDesc> outputs;\
			inputs.push_back({\
				.name = "V",\
				.allowed_types = { TypeInfo::get<float>() }\
			});\
			for (auto i = 0; i < N; i++) \
			{\
				inputs.push_back({\
					.name = "Stop " + std::to_string(i + 1),\
					.allowed_types = { TypeInfo::get<float>() }\
				});\
			};\
			for (auto i = 0; i < N; i++) \
			{\
				outputs.push_back({\
					.name = "Branch " + std::to_string(i + 1),\
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }\
				});\
			};\
			outputs.push_back({\
				.name = "Else Branch",\
				.allowed_types = { TypeInfo::get<BlueprintSignal>() }\
			});\
			library->add_template(std::format("Ramp Branch {}", N), "",\
				inputs,\
				outputs,\
				[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {\
					auto v = *(float*)inputs[0].data;\
					for (auto i = 0; i < N + 1; i++) \
						(*(BlueprintSignal*)outputs[i].data).v = 0;\
					for (auto i = 0; i < N; i++) \
					{\
						if (v < *(float*)inputs[i + 1].data)\
						{\
							(*(BlueprintSignal*)outputs[i].data).v = 1;\
							return;\
						}\
					}\
					(*(BlueprintSignal*)outputs[N].data).v = 1;\
				},\
				nullptr,\
				nullptr,\
				nullptr,\
				nullptr\
			);\
		}

		RAMP_BRANCH_TEMPLATE(2);
		RAMP_BRANCH_TEMPLATE(3);
		RAMP_BRANCH_TEMPLATE(4);
		RAMP_BRANCH_TEMPLATE(5);
		RAMP_BRANCH_TEMPLATE(6);
		RAMP_BRANCH_TEMPLATE(7);
		RAMP_BRANCH_TEMPLATE(8);

#undef RAMP_BRANCH_TEMPLATE

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
			},
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			true,
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs, uint* max_execute_times) {
				auto& time = *(float*)inputs[1].data;
				auto interval = *(float*)inputs[2].data;

				time += delta_time;
				if (time >= interval)
				{
					*(float*)inputs[0].data = 0.f;
					*max_execute_times = 1;
				}
				else
					*max_execute_times = 0;
			}
		);
	}
}
