#include <flame/foundation/foundation.h>

#include <iostream>
#include <conio.h>
#include <functional>

using namespace flame;

int main(int argc, char** args)
{
	auto ap = parse_args(argc, args);
	auto cmd = ap.get_item("-cmd");
	auto path = std::filesystem::path(ap.get_item("-path"));
	if (path.empty())
		path = std::filesystem::current_path();
	else
		std::filesystem::current_path(path);
	path.make_preferred();
	auto ppath = path.parent_path();
	auto name = ap.get_item("-name");
	auto name2 = ap.get_item("-name2");
	auto type = ap.get_item("-type");
	auto value = ap.get_item("-value");

	struct Block
	{
		int type = 0; // 0 - no brackets, 1 - brackets, 2 - brackets end with semicolon
		std::string line;
		std::list<Block> children;
		Block* parent = nullptr;

		Block() {}

		Block(const std::string& line) :
			line(line)
		{
		}

		Block(int _place_holder, const std::string& fn)
		{
			std::ifstream file(fn);
			std::vector<std::string> lines;
			while (!file.eof())
			{
				std::string line;
				std::getline(file, line);
				SUS::trim(line);
				lines.push_back(line + '\n');
			}
			file.close();
			lines.back().pop_back();

			std::function<int(Block& b, int idx)> gather_block;
			gather_block = [&](Block& b, int idx) {
				for (auto i = idx; i < lines.size(); i++)
				{
					auto& l = lines[i];
					if (l.size() == 2 && l == "{\n")
					{
						auto& nb = b.children.emplace_back();
						nb.type = 1;
						i = gather_block(nb, i + 1);
					}
					else if (l.size() == 2 && l == "}\n")
					{
						b.type = 1;
						return i;
					}
					else if (l.size() == 3 && l == "};\n")
					{
						b.type = 2;
						return i;
					}
					else
						b.children.emplace_back(l);
				}
				return (int)lines.size();
			};

			gather_block(*this, 0);
			init();
		}

		void init()
		{
			for (auto it = children.begin(); it != children.end();)
			{
				it->parent = this;
				auto t = it; t++;
				if (it->line.size() > 2 && *(it->line.end() - 2) != ';' && t->type != 0)
				{
					it->children = t->children;
					it->type = t->type;
					it = children.erase(t);
				}
				else
					it++;
			}
			for (auto& c : children)
				c.init();
		}

		bool find(const std::regex& r, std::smatch& res, std::list<Block>::iterator& out_it)
		{
			for (auto it = children.begin(); it != children.end(); it++)
			{
				if (std::regex_search(it->line, res, r))
				{
					out_it = it;
					return true;
				}
				if (it->find(r, res, out_it))
					return true;
			}
			return false;
		}

		std::string output(int indent)
		{
			std::string ret;
			std::string indent_str;
			for (auto i = 0; i < indent; i++)
				indent_str += "\t";
			if (!line.empty())
				ret += indent_str + line;
			if (type != 0)
			{
				ret += indent_str + "{\n";
				indent++;
			}
			for (auto& c : children)
				ret += c.output(indent);
			if (type != 0)
			{
				if (type == 2)
					ret += indent_str + "};\n";
				else
					ret += indent_str + "}\n";
			}
			return ret;
		}

		void output_file(const std::string& fn)
		{
			std::ofstream file(fn);
			file << output(0);
			file.close();
		}
	};


	bool is_internal;
	{
		auto flame_path = std::filesystem::path(getenv("FLAME_PATH"));
		flame_path.make_preferred();
		auto str1 = flame_path.string();
		auto str2 = ppath.string();
		is_internal = str2.size() >= str1.size() && str2.compare(0, str1.size(), str1) == 0;
	}
	std::string class_name;
	std::string public_header_fn;
	std::string private_header_fn;
	std::string source_fn;
	std::smatch match;
	std::list<Block>::iterator it1, it2;

	auto get_class_name = [&](const std::string& n) {
		class_name = n;
		class_name[0] = std::toupper(class_name[0]);
		for (auto i = 0; i < class_name.size(); i++)
		{
			if (class_name[i] == '_')
				class_name[i + 1] = std::toupper(class_name[i + 1]);
		}
		SUS::remove_ch(class_name, '_');
	};

	auto init_vars = [&]() {
		auto fn = path.stem().string();
		SUS::cut_tail_if(fn, "_private");

		get_class_name(fn);

		public_header_fn = ppath.string() + "/" + fn + ".h";
		private_header_fn = ppath.string() + "/" + fn + "_private.h";
		source_fn = ppath.string() + "/" + fn + ".cpp";
		if (!std::filesystem::exists(public_header_fn) || !std::filesystem::exists(private_header_fn) || !std::filesystem::exists(source_fn))
		{
			printf("cannot find %s or %s or %s\n", public_header_fn.c_str(), private_header_fn.c_str(), source_fn.c_str());
			system("pause");
			return false;
		}
		printf("public header: %s\nprivate header: %s\nsource: %s\n", public_header_fn.c_str(), private_header_fn.c_str(), source_fn.c_str());
		return true;
	};

	if (cmd.empty())
	{
		const char* cmds[] = {
			"new_template",
			"new_attribute",
			"remove_attribute",
			"alter_attribute",
			"new_function",
			"remove_function"
		};
		printf("what you want?\n");
		for (auto i = 0; i < _countof(cmds); i++)
			printf("%d %s\n", i, cmds[i]);
		auto id = getch() - '0';
		if (id >= 0 && id < _countof(cmds))
		{
			cmd = cmds[id];
			printf("you choose %s\n", cmd.c_str());
		}
	}

	if (cmd == "new_template")
	{
		const char* templates[] = {
			"general",
			"component",
			"system"
		};
		printf("what template?\n");
		for (auto i = 0; i < _countof(templates); i++)
			printf("%d %s\n", i, templates[i]);
		auto id = getch() - '0';
		if (id >= 0 && id < _countof(templates))
		{
			cmd = "new_" + std::string(templates[id]);
			printf("you choose %s\n", cmd.c_str());
		}
	}
	if (cmd == "new_template")
	{
		printf("current directory: %s\n", path.string().c_str());

		if (name.empty())
		{
			std::cout << "name: ";
			std::getline(std::cin, name);
		}
		if (name.empty())
			return 0;

		get_class_name(name);
		class_name = "c" + class_name;

		auto indent_str = is_internal ? "\t" : "";

		std::ofstream public_header_file(name + ".h");
		public_header_file << "#pragma once\n\n";
		if (is_internal)
			public_header_file << "#include \"../component.h\"\n\n";
		else
			public_header_file << "#include <flame/universe/component.h>\n\n";
		if (is_internal) public_header_file << "namespace flame\n{\n";
		public_header_file << indent_str << "struct " << class_name << " : Component\n\t{\n";
		public_header_file << indent_str << "\tinline static auto type_name = \"" << (is_internal ? "flame::" : "") << class_name << "\";\n";
		public_header_file << indent_str << "\tinline static auto type_hash = ch(type_name);\n\n";
		public_header_file << indent_str << "\t" << class_name << "() : Component(type_name, type_hash)\n\t\t{\n\t\t}\n\n";
		public_header_file << indent_str << "\t" << (is_internal ? "FLAME_UNIVERSE_EXPORTS" : "__declspec(dllexport)") << " static " << class_name << "* create(void* parms = nullptr);\n";
		public_header_file << indent_str << "};\n";
		if (is_internal) public_header_file << "}\n";
		public_header_file.close();

		std::ofstream private_header_file(name + "_private.h");
		private_header_file << "#pragma once\n\n";
		private_header_file << "#include \"" << name << ".h\"\n\n";
		if (is_internal) private_header_file << "namespace flame\n{\n";
		private_header_file << indent_str << "struct " << class_name << "Private : " << class_name << "\n\t{\n";
		private_header_file << indent_str << "};\n";
		if (is_internal) private_header_file << "}\n";
		private_header_file.close();

		std::ofstream source_file(name + ".cpp");
		source_file << "#include \"" << name << "_private.h\"\n\n";
		if (is_internal) source_file << "namespace flame\n{\n";
		source_file << indent_str<< class_name << "* " << class_name << "::create(void* parms)\n\t{\n";
		source_file << indent_str << "\treturn new " << class_name << "Private();\n";
		source_file << indent_str << "}\n";
		if (is_internal) source_file << "}\n";
		source_file.close();
	}
	else if (cmd == "new_attribute")
	{
		if (!init_vars())
			return 0;

		class_name = "c" + class_name;

		new_attribute_process:

		if (name.empty())
		{
			std::cout << "name: ";
			std::getline(std::cin, name);
		}
		if (name.empty())
			return 0;
		if (type.empty())
		{
			std::cout << "type: ";
			std::getline(std::cin, type);
		}
		if (type.empty())
			return 0;
		if (value.empty())
		{
			std::cout << "value: ";
			std::getline(std::cin, value);
		}
		if (value.empty())
			return 0;

		Block public_header_blocks(0, public_header_fn);
		if (public_header_blocks.find(std::regex("^struct\\s+" + class_name + "\\s*:\\s*Component"), match, it1))
		{
			if (it1->find(std::regex("^\\w+\\s+static\\s+" + class_name + "\\s*\\*\\s+create\\("), match, it2))
			{
				auto& list = it2->parent->children;
				list.emplace(it2, "virtual " + type + " get_" + name + "() const = 0;\n");
				list.emplace(it2, "virtual void set_" + name + "(" + type + " v) = 0;\n");
				list.emplace(it2, "\n");
			}
		}
		public_header_blocks.output_file(public_header_fn);

		Block private_header_blocks(0, private_header_fn);
		if (private_header_blocks.find(std::regex("^struct\\s+" + class_name + "Private\\s*:\\s*" + class_name), match, it1))
		{
			auto& list = it1->children;
			it2 = it1->children.begin();
			for (; it2 != list.end(); it2++)
			{
				if (it2->type != 0 || it2->line == "\n")
					break;
			}
			list.emplace(it2, type + " " + name + " = " + value + ";\n");
			list.emplace_back(type + " get_" + name + "() const override { return " + name + "; }\n");
			list.emplace_back("void set_" + name + "(" + type + " v) override;\n");
		}
		private_header_blocks.output_file(private_header_fn);

		Block source_blocks(0, source_fn);
		if (source_blocks.find(std::regex("^" + class_name + "\\s*\\*\\s+" + class_name + "::create\\("), match, it1))
		{
			auto& list = it1->parent->children;
			auto& nb = *list.emplace(it1, "void " + class_name + "Private::set_" + name + "(" + type + " v)\n");
			nb.type = 1;
			nb.children.emplace_back(name + " = v;\n");
			list.emplace(it1, "\n");
		}
		source_blocks.output_file(source_fn);

		name.clear();
		type.clear();
		value.clear();
		goto new_attribute_process;
	}
	else if (cmd == "remove_attribute")
	{
		if (!init_vars())
			return 0;

		class_name = "c" + class_name;

		remove_attribute_process:

		if (name.empty())
		{
			std::cout << "name: ";
			std::getline(std::cin, name);
		}
		if (name.empty())
			return 0;

		Block public_header_blocks(0, public_header_fn);
		if (public_header_blocks.find(std::regex("^struct\\s+" + class_name + "\\s*:\\s*Component"), match, it1))
		{
			if (it1->find(std::regex("^virtual\\s+\\w+\\s+get_" + name + "\\("), match, it2))
				it2->parent->children.erase(it2);
			if (it1->find(std::regex("^virtual\\s+void\\s+set_" + name + "\\(\\w+"), match, it2))
				it2->parent->children.erase(it2);
		}
		public_header_blocks.output_file(public_header_fn);

		Block private_header_blocks(0, private_header_fn);
		if (private_header_blocks.find(std::regex("^struct\\s+" + class_name + "Private\\s*:\\s*" + class_name), match, it1))
		{
			if (it1->find(std::regex("^\\w+\\s+" + name + "\\s+=\\s+"), match, it2))
				it2->parent->children.erase(it2);
			if (it1->find(std::regex("^\\w+\\s+get_" + name + "\\("), match, it2))
				it2->parent->children.erase(it2);
			if (it1->find(std::regex("^void\\s+set_" + name + "\\(\\w+"), match, it2))
				it2->parent->children.erase(it2);
		}
		private_header_blocks.output_file(private_header_fn);

		Block source_blocks(0, source_fn);
		if (source_blocks.find(std::regex("\\w+\\s+" + class_name + "Private::get_" + name + "\\("), match, it1))
			it1->parent->children.erase(it1);
		if (source_blocks.find(std::regex("^void\\s+" + class_name + "Private::set_" + name + "\\(\\w+"), match, it1))
			it1->parent->children.erase(it1);
		source_blocks.output_file(source_fn);

		name.clear();
		goto remove_attribute_process;
	}
	else if (cmd == "alter_attribute")
	{
		if (!init_vars())
			return 0;

		class_name = "c" + class_name;

		alter_attribute_process:

		if (name.empty())
		{
			std::cout << "original name: ";
			std::getline(std::cin, name);
		}
		if (name.empty())
			return 0;
		if (name2.empty())
		{
			std::cout << "new name: ";
			std::getline(std::cin, name2);
		}
		if (name2.empty())
			name2 = name;
		if (type.empty())
		{
			std::cout << "type: ";
			std::getline(std::cin, type);
		}

		Block private_header_blocks(0, private_header_fn);
		if (private_header_blocks.find(std::regex("^struct\\s+" + class_name + "Private\\s*:\\s*" + class_name), match, it1))
		{
			Block public_header_blocks(0, public_header_fn);
			Block source_blocks(0, source_fn);

			std::regex reg1 = std::regex("^\\w+\\s+" + name + "\\s+=\\s+");
			if (it1->find(reg1, match, it2))
			{
				if (type.empty())
				{
					std::smatch res;
					if (std::regex_search(it2->line, res, std::regex("^\\w+")))
						type = res[0].str();
				}
				if (!type.empty())
				{
					it2->line = std::regex_replace(it2->line, reg1, type + " " + name2 + " = ");
					{
						std::regex reg1 = std::regex("^\\w+\\s+get_" + name + "\\(");
						if (it1->find(reg1, match, it2))
							it2->line = std::regex_replace(it2->line, reg1, type + " get_" + name2 + "(");
						std::regex reg2 = std::regex("^void\\s+set_" + name + "\\(\\w+");
						if (it1->find(reg2, match, it2))
							it2->line = std::regex_replace(it2->line, reg2, "void set_" + name2 + "(" + type);
					}

					if (public_header_blocks.find(std::regex("^struct\\s+" + class_name + "\\s*:\\s*Component"), match, it1))
					{
						std::regex reg1 = std::regex("^virtual\\s+\\w+\\s+get_" + name + "\\(");
						if (it1->find(reg1, match, it2))
							it2->line = std::regex_replace(it2->line, reg1, "virtual " + type + " get_" + name2 + "(");
						std::regex reg2 = std::regex("^virtual\\s+void\\s+set_" + name + "\\(\\w+");
						if (it1->find(reg2, match, it2))
							it2->line = std::regex_replace(it2->line, reg2, "virtual void set_" + name2 + "(" + type);
					}

					{
						std::regex reg = std::regex("\\w+\\s+" + class_name + "Private::get_" + name + "\\(");
						if (source_blocks.find(reg, match, it1))
							it1->line = std::regex_replace(it1->line, reg, type + " " + class_name + "Private::get_" + name2 + "(");
					}
					{
						std::regex reg = std::regex("^void\\s+" + class_name + "Private::set_" + name + "\\(\\w+");
						if (source_blocks.find(reg, match, it1))
							it1->line = std::regex_replace(it1->line, reg, "void " + class_name + "Private::set_" + name2 + "(" + type);
					}

					public_header_blocks.output_file(public_header_fn);
					private_header_blocks.output_file(private_header_fn);
					source_blocks.output_file(source_fn);
				}
			}
		}

		name.clear();
		name2.clear();
		type.clear();
		goto alter_attribute_process;
	}
	else if (cmd == "new_function")
	{
		if (!init_vars())
			return 0;

		class_name = "c" + class_name;

		new_function_process:

		if (name.empty())
		{
			std::cout << "name: ";
			std::getline(std::cin, name);
		}
		if (name.empty())
			return 0;
		auto is_public = false;
		auto is_static = false;
		auto is_const = false;
		auto is_override = false;
		{
			auto sp = SUS::split(name);
			if (sp.size() >= 2)
			{
				name = sp[0];
				for (auto i = 1; i < sp.size(); i++)
				{
					if (sp[i] == "public")
					{
						is_public = true;
						is_override = true;
					}
					else if (sp[i] == "static")
					{
						is_public = true;
						is_static = true;
					}
					else if (sp[i] == "const")
						is_const = true;
					else if (sp[i] == "override")
						is_override = true;
				}
			}
		}
		if (type.empty())
		{
			std::cout << "type: ";
			std::getline(std::cin, type);
		}
		if (type.empty())
			return 0;
		if (value.empty())
		{
			std::cout << "parameters: ";
			std::getline(std::cin, value);
		}
		if (value.empty())
			return 0;
		std::string parameters1;
		std::string parameters2;
		{
			auto sp = SUS::split(value, ',');
			for (auto& p : sp)
			{
				if (!parameters1.empty())
					parameters1 += ", ";
				parameters1 += p;
				if (!parameters2.empty())
					parameters2 += ", ";
				auto p2 = p.substr(0, p.find('='));
				SUS::trim(p2);
				parameters2 += p2;
			}
			if (parameters1 == "void")
				parameters1.clear();
			if (parameters2 == "void")
				parameters2.clear();
		}

		if (is_public)
		{
			Block public_header_blocks(0, public_header_fn);
			if (public_header_blocks.find(std::regex("^struct\\s+" + class_name + "\\s*:\\s*Component"), match, it1))
			{
				if (it1->find(std::regex("^\\w+\\s+static\\s+" + class_name + "\\s*\\*\\s+create\\("), match, it2))
				{
					auto& list = it2->parent->children;
					list.emplace(it2, (is_static ? (is_internal ? "FLAME_UNIVERSE_EXPORTS static " : "__declspec(dllexport) static ") : "virtual ") +
						type + " " + name + "(" + parameters1 + ")" + (is_const ? " const" : "") + " = 0;\n");
					list.emplace(it2, "\n");
				}
			}
			public_header_blocks.output_file(public_header_fn);
		}

		if (!is_static)
		{
			Block private_header_blocks(0, private_header_fn);
			if (private_header_blocks.find(std::regex("^struct\\s+" + class_name + "Private\\s*:\\s*" + class_name), match, it1))
			{
				auto& list = it1->children;
				list.emplace_back(type + " " + name + "(" + parameters1 + ")" + (is_const ? " const" : "") + (is_override ? " override" : "") + ";\n");
			}
			private_header_blocks.output_file(private_header_fn);
		}

		Block source_blocks(0, source_fn);
		if (source_blocks.find(std::regex("^" + class_name + "\\s*\\*\\s+" + class_name + "::create\\("), match, it1))
		{
			auto& list = it1->parent->children;
			auto& nb = *list.emplace(it1, type + " " + class_name + (is_static ? "::" : "Private::") + name + "(" + parameters2 + ") " + (is_const ? "const\n" : "\n"));
			nb.type = 1;
			nb.children.emplace_back("\n");
			list.emplace(it1, "\n");
		}
		source_blocks.output_file(source_fn);

		name.clear();
		type.clear();
		value.clear();
		goto new_function_process;
	}
	else if (cmd == "remove_function")
	{
		if (!init_vars())
			return 0;

		class_name = "c" + class_name;

		remove_function_process:

		if (name.empty())
		{
			std::cout << "name: ";
			std::getline(std::cin, name);
		}
		if (name.empty())
			return 0;

		Block public_header_blocks(0, public_header_fn);
		if (public_header_blocks.find(std::regex("^struct\\s+" + class_name + "\\s*:\\s*Component"), match, it1))
		{
			if (it1->find(std::regex("\\b" + name + "\\("), match, it2))
				it2->parent->children.erase(it2);
		}
		public_header_blocks.output_file(public_header_fn);

		Block private_header_blocks(0, private_header_fn);
		if (private_header_blocks.find(std::regex("^struct\\s+" + class_name + "Private\\s*:\\s*" + class_name), match, it1))
		{
			if (it1->find(std::regex("\\b" + name + "\\("), match, it2))
				it2->parent->children.erase(it2);
		}
		private_header_blocks.output_file(private_header_fn);

		Block source_blocks(0, source_fn);
		if (source_blocks.find(std::regex(class_name + "(Private)?::" + name + "\\("), match, it1))
			it1->parent->children.erase(it1);
		source_blocks.output_file(source_fn);

		name.clear();
		goto remove_function_process;
	}
	return 0;
}
