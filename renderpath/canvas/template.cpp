// this file is auto generated

#include <flame/foundation/bp_node_template.h>

using namespace flame;

extern "C" __declspec(dllexport) void add_templates()
{
	auto module = get_module_from_address(f2v(add_templates));
	auto module_name = get_module_name(module);

	BP_Array<1, uint>::add_udt_info(*module_name.p, "1~uint", module);
	BP_Array<1, void*>::add_udt_info(*module_name.p, "1~void*", module);
	BP_Array<2, void*>::add_udt_info(*module_name.p, "2~void*", module);
	BP_Array<2, Vec<4, uchar>>::add_udt_info(*module_name.p, "2~Vec<4, uchar>", module);

	delete_mail(module_name);
}
