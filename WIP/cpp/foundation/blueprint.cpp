// generate cpp codes that do the 'update' job and save to a file
// then use that file to compile a dll
// note: need filename first and all nodes need to have code

void BPPrivate::tobin()
{
	if (filename.empty())
	{
		printf("you need to do 'save' first\n");
		return;
	}
	if (update_list.empty())
	{
		printf("no nodes or didn't call 'initialize'\n");
		return;
	}

	std::string code;

	std::vector<std::pair<std::wstring, bool>> using_modules = {
		{L"flame_graphics.dll", false},
		{L"flame_sound.dll", false},
		{L"flame_physics.dll", false},
		{L"flame_network.dll", false},
		{L"flame_universe.dll", false}
	};

	for (auto& n : nodes)
	{
		for (auto& m : using_modules)
		{
			if (m.first == n->udt->module_name())
				m.second = true;
		}
	}

	code += "#include <flame/foundation/foundation.h>\n";
	if (using_modules[0].second)
		code += "#include <flame/graphics/all.h>\n";

	code += "\nusing namespace flame;\n\n";

	auto define_variable = [](const std::string & id_prefix, VariableInfo * v) {
		auto id = id_prefix + v->name();
		auto type = std::string(v->type()->name());
		if (v->type()->tag() == TypeTagPointer)
			type += "*";
		return type + " " + id + ";\n";
	};

	auto set_variable_value = [](const std::string & id_prefix, VariableInfo * v, const CommonData & data) {
		auto id = id_prefix + v->name();
		std::string value;
		switch (v->type()->name_hash())
		{
		case cH("bool"):
			value = data.v.i[0] ? "true" : "false";
			break;
		case cH("uint"):
			value = to_stdstring((uint)data.v.i[0]);
			break;
		case cH("int"):
			value = to_stdstring(data.v.i[0]);
			break;
		case cH("Ivec2"):
			value = "Ivec2(" + to_stdstring(data.v.i[0]) + ", " + to_stdstring(data.v.i[1]) + ")";
			break;
		case cH("Ivec3"):
			value = "Ivec3(" + to_stdstring(data.v.i[0]) + ", " + to_stdstring(data.v.i[1]) + ", " + to_stdstring(data.v.i[2]) + ")";
			break;
		case cH("Ivec4"):
			value = "Ivec4(" + to_stdstring(data.v.i[0]) + ", " + to_stdstring(data.v.i[1]) + ", " + to_stdstring(data.v.i[2]) + ", " + to_stdstring(data.v.i[3]) + ")";
			break;
		case cH("float"):
			value = to_stdstring(data.v.f[0]);
			break;
		case cH("Vec2"):
			value = "Vec2(" + to_stdstring(data.v.f[0]) + ", " + to_stdstring(data.v.f[1]) + ")";
			break;
		case cH("Vec3"):
			value = "Vec3(" + to_stdstring(data.v.f[0]) + ", " + to_stdstring(data.v.f[1]) + ", " + to_stdstring(data.v.f[2]) + ")";
			break;
		case cH("Vec4"):
			value = "Vec4(" + to_stdstring(data.v.f[0]) + ", " + to_stdstring(data.v.f[1]) + ", " + to_stdstring(data.v.f[2]) + ", " + to_stdstring(data.v.f[3]) + ")";
			break;
		case cH("Bvec2"):
			value = "Bvec2(" + to_stdstring(data.v.b[0]) + ", " + to_stdstring(data.v.b[1]) + ")";
			break;
		case cH("Bvec3"):
			value = "Bvec3(" + to_stdstring(data.v.b[0]) + ", " + to_stdstring(data.v.b[1]) + ", " + to_stdstring(data.v.b[2]) + ")";
			break;
		case cH("Bvec4"):
			value = "Bvec4(" + to_stdstring(data.v.b[0]) + ", " + to_stdstring(data.v.b[1]) + ", " + to_stdstring(data.v.b[2]) + ", " + to_stdstring(data.v.b[3]) + ")";
			break;
		case cH("void"):
			value = "nullptr";
			break;
		}
		return id + " = " + value + ";\n";
	};

	for (auto& n : update_list)
	{
		auto id_prefix = n->id + "_";
		for (auto& input : n->inputs)
			code += define_variable(id_prefix, input->variable_info);
		for (auto& output : n->outputs)
			code += define_variable(id_prefix, output->variable_info);
	}

	code += "\n __declspec(dllexport) void initialize()\n{\n";
	for (auto& n : update_list)
	{
		auto id_prefix = n->id + "_";
		for (auto& input : n->inputs)
			code += "\t" + set_variable_value(id_prefix, input->variable_info, input->data);
		for (auto& output : n->outputs)
			code += "\t" + set_variable_value(id_prefix, output->variable_info, output->data);
	}
	code += "}\n";

	code += "\n __declspec(dllexport) void update()\n{\n";
	std::regex reg_variable(R"(\b([\w]+)\$\w*\b)");
	for (auto& n : update_list)
	{
		auto id_prefix = n->id + "_";
		auto udt = n->udt;
		auto fmt_id = id_prefix + "$1";

		for (auto& input : n->inputs)
		{
			auto link = input->link;
			if (link)
			{
				auto dst_id = id_prefix + input->variable_info->name();
				code += "\t" + dst_id + " = " + link->node->id + "_" + link->variable_info->name() + ";\n";
			}
		}

		std::string str(n->update_function->code());
		str = std::regex_replace(str, reg_variable, fmt_id);
		code += str + "\n";
		code += "\n";
	}
	code += "}\n";

	code += "\n";

	auto cpp_filename = ext_replace(filename, L".cpp");
	std::ofstream file(cpp_filename);
	file << code;
	file.close();

	std::vector<std::wstring> libraries;
	libraries.push_back(L"flame_foundation.lib");
	for (auto& m : using_modules)
	{
		if (m.second)
			libraries.push_back(m.first);
	}

	auto output = compile_to_dll({ cpp_filename }, libraries, ext_replace(filename, L".dll"));

	printf("compiling:\n%s\n\n", output.v);
}