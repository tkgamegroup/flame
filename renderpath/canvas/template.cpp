// this file is auto generated

#include <flame/foundation/bp_node_template.h>

using namespace flame;

extern "C" __declspec(dllexport) void add_templates(int level)
{
	auto this_module = load_module(L"bp.dll");

	BP_Array<1, uint>::add_udt_info(level, "1~uint", this_module);
	BP_Array<1, void*>::add_udt_info(level, "1~void*", this_module);
	BP_Array<2, void*>::add_udt_info(level, "2~void*", this_module);
	BP_Array<2, Vec<4, uchar>>::add_udt_info(level, "2~Vec<4, uchar>", this_module);

	free_module(this_module);
}
