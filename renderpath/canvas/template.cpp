// this file is auto generated

#include <flame/foundation/bp_node_template.h>

using namespace flame;

extern "C" __declspec(dllexport) void add_templates(int level)
{
	auto module = get_module_from_address(f2v(add_templates));
	auto module_name = get_module_name(module);
	std::fs::path path(*module_name.p);
	auto filename = path.filename().wstring();

	BP_Array<1, uint>::add_udt_info(level, "1~uint", module, filename);
	BP_Array<1, void*>::add_udt_info(level, "1~void*", module, filename);
	BP_Array<2, void*>::add_udt_info(level, "2~void*", module, filename);
	BP_Array<2, Vec<4, uchar>>::add_udt_info(level, "2~Vec<4, uchar>", module, filename);

	delete_mail(module_name);
}
