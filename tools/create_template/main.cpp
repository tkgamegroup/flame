#include <flame/serialize.h>

#include <iostream>

using namespace flame;

int main(int argc, char **args)
{
	if (argc >= 2)
	{
		auto what = std::string(args[1]);
		if (what == "component_source")
		{
			std::string name;
			if (argc > 2)
				name = args[2];
			else
			{
				std::cout << "name: ";
				std::cin >> name;
			}
			auto class_name = name;
			class_name[0] = std::toupper(name[0]);
			for (auto i = 0; i < name.size(); i++)
			{
				if (class_name[i] == '_')
					class_name[i + 1] = std::toupper(class_name[i + 1]);
			}
			SUS::remove_ch(class_name, '_');
			class_name = "c" + class_name;

			auto internal = std::filesystem::exists("../component.h");

			std::ofstream public_header_file(name + ".h");
			public_header_file << "#pragma once\n\n";
			if (internal)
				public_header_file << "#include \"../component.h\"\n\n";
			else
				public_header_file << "#include <flame/universe/component.h>\n\n";
			public_header_file << "namespace flame\n{\n";
			public_header_file << "\tstruct " << class_name << " : Component\n\t{\n";
			public_header_file << "\t\tinline static auto type_name = \"" << (internal ? "flame::" : "") << class_name << "\";\n";
			public_header_file << "\t\tinline static auto type_hash = ch(type_name);\n\n";
			public_header_file << "\t\t" << class_name << "() :\n";
			public_header_file << "\t\t\tComponent(type_name, type_hash)\n\t\t{\n\t\t}\n\n";
			public_header_file << "\t\tFLAME_UNIVERSE_EXPORTS static cImage* create(void* parms = nullptr);\n";
			public_header_file << "\t};\n}\n";
			public_header_file.close();

			std::ofstream private_header_file(name + "_private.h");
			private_header_file << "#pragma once\n\n";
			private_header_file << "#include \"" << name << ".h\"\n\n";
			private_header_file << "namespace flame\n{\n";
			private_header_file << "\tstruct " << class_name << "Private : " << class_name << "\n\t{\n";
			private_header_file << "\t};\n}\n";
			private_header_file.close();

			std::ofstream source_file(name + ".cpp");
			source_file << "#include \"" << name << "_private.h\"\n\n";
			source_file << "namespace flame\n{\n";
			source_file << "\t" << class_name << "* " << class_name << "::create(void* parms)\n\t{\n";
			source_file << "\t\treturn new " << class_name << "Private();\n";
			source_file << "\t}\n}\n";
			source_file.close();
		}
	}
	return 0;
}
