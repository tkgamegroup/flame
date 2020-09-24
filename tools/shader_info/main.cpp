#include <flame/serialize.h>

#include <spirv_glsl.hpp>

#include <functional>

using namespace flame;

int main(int argc, char** args)
{
	auto file = get_file_content(args[1]);
	auto glsl = spirv_cross::CompilerGLSL((uint*)file.c_str(), file.size() / sizeof(uint));
	auto resources = glsl.get_shader_resources();

	std::function<void(int indent, const spirv_cross::SPIRType&)> print_type;
	print_type = [&](int indent, const spirv_cross::SPIRType& t) {
		auto end = 0;
		for (auto i = 0; i < t.member_types.size(); i++)
		{
			auto off = glsl.type_struct_member_offset(t, i);
			auto size = glsl.get_declared_struct_member_size(t, i);
			printf("%*s%s: %d %d %s\n", indent, "", glsl.get_member_name(t.self, i).c_str(), off, size, end != off ? "ALIGNMENT DISMATCH" : "");
			end += size;
			print_type(indent + 2, glsl.get_type(t.member_types[i]));
		}
	};

	for (auto& r : resources.uniform_buffers)
	{
		printf("%s:\n", r.name.c_str());
		print_type(2, glsl.get_type(r.base_type_id));
		printf("\n");
	}
	printf("\n");
	for (auto& r : resources.storage_buffers)
	{
		printf("%s:\n", r.name.c_str());
		print_type(2, glsl.get_type(r.base_type_id));
		printf("\n");
	}

	return 0;
}
