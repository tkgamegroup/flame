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
	path.make_preferred();
	if (path.empty())
		path = std::filesystem::current_path();
	auto parent_path = path.parent_path();
	auto name = ap.get_item("-name");
	auto name2 = ap.get_item("-name2");
	auto type = ap.get_item("-type");
	auto value = ap.get_item("-value");
	auto line = s2t<int>(ap.get_item("-line"));

	auto flame_path = std::filesystem::path(getenv("FLAME_PATH"));
	flame_path.make_preferred();
	bool is_internal;
	auto export_str = std::string("__declspec(dllexport)");
	{
		auto str_ppath = parent_path.wstring();
		auto str_flame_path = flame_path.wstring();
		is_internal = str_ppath.starts_with(str_flame_path);
		if (is_internal)
		{
			if (str_ppath.starts_with(str_flame_path + L"\\source\\foundation"))
				export_str = "FLAME_FOUNDATION_API";
			else if (str_ppath.starts_with(str_flame_path + L"\\source\\blueprint"))
				export_str = "FLAME_BLUEPRINT_API";
			else if (str_ppath.starts_with(str_flame_path + L"\\source\\graphics"))
				export_str = "FLAME_GRAPHICS_API";
			else if (str_ppath.starts_with(str_flame_path + L"\\source\\sound"))
				export_str = "FLAME_SOUND_API";
			else if (str_ppath.starts_with(str_flame_path + L"\\source\\physics"))
				export_str = "FLAME_PHYSICS_API";
			else if (str_ppath.starts_with(str_flame_path + L"\\source\\universe"))
				export_str = "FLAME_UNIVERSE_API";
		}
	}

	std::string class_name;
	auto get_class_name = [&](std::string_view n) {
		class_name = n;
		class_name[0] = std::toupper(class_name[0]);
		for (auto i = 0; i < class_name.size(); i++)
		{
			if (class_name[i] == '_')
				class_name[i + 1] = std::toupper(class_name[i + 1]);
		}
		SUS::strip_char(class_name, '_');
	};

	if (std::filesystem::is_directory(path))
	{
		printf("current directory: %s\n", path.string().c_str());
		std::filesystem::current_path(path);

		if (cmd.empty())
		{
			printf("what you want?\n");
			const char* cmds[] = {
				"new_general_template",
				"new_component_template",
				"new_system_template"
			};
			for (auto i = 0; i < _countof(cmds); i++)
				printf("%d %s\n", i + 1, cmds[i]);
			auto id = getch() - '1';
			if (id >= 0 && id < _countof(cmds))
			{
				cmd = cmds[id];
				printf("you choose %s\n", cmd.c_str());
			}
		}

		if (name.empty())
		{
			std::cout << "name: ";
			std::getline(std::cin, name);
		}
		if (name.empty())
			return 0;

		get_class_name(name);
		if (cmd == "new_component_template")
			class_name = 'c' + class_name;
		else if (cmd == "new_system_template")
			class_name = 's' + class_name;

		auto indent_str = is_internal ? "\t" : "";

		std::ofstream public_header_file(name + ".h");
		public_header_file << "#pragma once\n\n";
		if (cmd == "new_component_template")
		{
			if (is_internal)
				public_header_file << "#include \"../component.h\"\n\n";
			else
				public_header_file << "#include <flame/universe/component.h>\n\n";
		}
		else if (cmd == "new_system_template")
		{
			if (is_internal)
				public_header_file << "#include \"../system.h\"\n\n";
			else
				public_header_file << "#include <flame/universe/system.h>\n\n";
		}
		if (is_internal) public_header_file << "namespace flame\n{\n";
		public_header_file << indent_str << "struct " << class_name;
		if (cmd == "new_component_template")
			public_header_file << " : Component";
		else if (cmd == "new_system_template")
			public_header_file << " : System";
		public_header_file << "\n";
		public_header_file << indent_str << "{\n";
		if (cmd == "new_general_template")
			public_header_file << indent_str << "\tvirtual ~" << class_name << "() {}\n\n";
		public_header_file << indent_str << "\t" << "struct Create\n";
		public_header_file << indent_str << "\t" << "{\n";
		public_header_file << indent_str << "\t\t" << "virtual " << class_name << "Ptr operator()() = 0;\n";
		public_header_file << indent_str << "\t" << "};\n";
		public_header_file << indent_str << "\t" << export_str << " static Create& create;\n";
		public_header_file << indent_str << "};\n";
		if (is_internal) public_header_file << "}\n";
		public_header_file.close();

		std::ofstream private_header_file(name + "_private.h");
		private_header_file << "#pragma once\n\n";
		private_header_file << "#include \"" << name << ".h\"\n\n";
		if (is_internal) private_header_file << "namespace flame\n{\n";
		private_header_file << indent_str << "struct " << class_name << "Private : " << class_name << "\n";
		private_header_file << indent_str << "{\n";
		private_header_file << indent_str << "};\n";
		if (is_internal) private_header_file << "}\n";
		private_header_file.close();

		std::ofstream source_file(name + ".cpp");
		source_file << "#include \"" << name << "_private.h\"\n\n";
		if (is_internal) source_file << "namespace flame\n{\n";
		if (cmd == "new_general_template")
		{
			source_file << indent_str << class_name << "Private::~" << class_name << "Private()\n";
			source_file << indent_str << "{\n";
			source_file << indent_str << "}\n\n";
		}
		source_file << indent_str << "struct " << class_name << "Create : " << class_name << "::Create\n";
		source_file << indent_str << "{\n";
		source_file << indent_str << "\t" << class_name << "Ptr operator()() override\n";
		source_file << indent_str << "\t{\n";
		source_file << indent_str << "\t\treturn new " << class_name << "Private();\n";
		source_file << indent_str << "\t}\n";
		source_file << indent_str << "}" << class_name << "_create;\n";
		source_file << indent_str << class_name << "::Create& " << class_name << "::create = " << class_name << "_create;\n";
		if (is_internal) source_file << "}\n";
		source_file.close();

		cmd.clear();
	}
	else
	{
		struct Block
		{
			ivec2 lno = ivec2(-1);
			std::string text = "`";
			std::list<Block> children;
			std::string b1;
			std::string b2;
			Block* parent = nullptr;

			Block(const ivec2& lno) :
				lno(lno)
			{
			}

			Block(const std::string& line, const ivec2& lno = ivec2(-1)) :
				lno(lno),
				text(line)
			{
			}

			Block(const std::vector<std::string>& lines)
			{
				static std::regex scope_beg("^\\s*\\{\\s*$");
				static std::regex scope_end("^\\s*\\}\\w*;?\\s*$");

				std::function<int(Block&, int, int)> gather_block;
				gather_block = [&](Block& b, int line_id, int bracket_level) {
					for (auto i = line_id; i < lines.size(); i++)
					{
						auto& l = lines[i];
						if (std::regex_search(l, scope_beg))
						{
							if (bracket_level == 0)
							{
								auto& nb = b.children.emplace_back(ivec2(i + 1, -1));
								nb.b1 = l;
								i = gather_block(nb, i + 1, bracket_level);
							}
							else
							{
								b.children.emplace_back(l, ivec2(i + 1, -1));
								bracket_level++;
							}
						}
						else if (std::regex_search(l, scope_end))
						{
							if (bracket_level == 0)
							{
								b.lno.y = i + 1;
								b.b2 = l;
								return i;
							}
							else
							{
								b.children.emplace_back(l, ivec2(i + 1, -1));
								bracket_level--;
							}
						}
						else
						{
							bracket_level += std::count(l.begin(), l.end(), '{');
							bracket_level -= std::count(l.begin(), l.end(), '}');
							b.children.emplace_back(l, ivec2(i + 1, -1));
						}
					}
					return (int)lines.size();
				};

				gather_block(*this, 0, 0);
				init();
				lno = ivec2(1, lines.size());
			}

			void init()
			{
				for (auto it = children.begin(); it != children.end();)
				{
					it->parent = this;
					if (it->b1.empty() && it->text.size() > 1 && it->text.back() != ';')
					{
						auto it2 = it; it2++;
						if (it2 != children.end() && !it2->b1.empty())
						{
							it->children = it2->children;
							it->lno.y = it2->lno.y;
							it->b1 = it2->b1;
							it->b2 = it2->b2;
							it = children.erase(it2);
							continue;
						}
					}
					it++;
				}
				for (auto& c : children)
					c.init();
			}

			bool find(const std::regex& r, std::smatch& res, std::list<Block>::iterator& out_it)
			{
				for (auto it = children.begin(); it != children.end(); it++)
				{
					if (std::regex_search(it->text, res, r))
					{
						out_it = it;
						return true;
					}
					if (it->find(r, res, out_it))
						return true;
				}
				return false;
			}

			bool find_scope(const std::regex& r, std::smatch& res, int i, std::list<Block>::iterator& out_it)
			{
				for (auto it = children.begin(); it != children.end(); it++)
				{
					if (!it->children.empty() && it->lno.x <= i && it->lno.y >= i)
					{
						if (std::regex_search(it->text, res, r))
						{
							out_it = it;
							return true;
						}
						if (it->find_scope(r, res, i, out_it))
							return true;
					}
				}
				return false;
			}

			std::string output()
			{
				std::string ret;
				if (text != "`")
					ret += text + '\n';
				if (!b1.empty())
					ret += b1 + '\n';
				for (auto& c : children)
					ret += c.output();
				if (!b2.empty())
					ret += b2 + '\n';
				return ret;
			}

			void output_file(const std::filesystem::path& fn)
			{
				auto text = output();
				text.pop_back();
				std::ofstream file(fn);
				file << text;
				file.close();
			}
		};

		printf("current directory: %s\n", parent_path.string().c_str());
		std::filesystem::current_path(parent_path);

		std::string public_header_fn;
		std::string private_header_fn;
		std::string source_fn;
		std::smatch match;
		std::list<Block>::iterator it1, it2;
		bool ok, ok2;

		{
			auto stem = path.stem().string();
			SUS::strip_tail_if(stem, "_private");

			public_header_fn = stem + ".h";
			private_header_fn = stem + "_private.h";
			source_fn = stem + ".cpp";
			if (!std::filesystem::exists(public_header_fn) || !std::filesystem::exists(private_header_fn) || !std::filesystem::exists(source_fn))
			{
				printf("cannot find %s or %s or %s\n", public_header_fn.c_str(), private_header_fn.c_str(), source_fn.c_str());
				system("pause");
				return 0;
			}
			printf("public header: %s\nprivate header: %s\nsource: %s\n", public_header_fn.c_str(), private_header_fn.c_str(), source_fn.c_str());
		}

		auto get_indent = [](const std::string& str) {
			std::string ret;
			for (auto sh : str)
			{
				if (!std::isspace(sh))
					break;
				ret += sh;
			}
			return ret;
		};

		auto get_file_lines = [](const std::filesystem::path& fn) {
			std::ifstream file(fn);
			std::vector<std::string> lines;
			while (!file.eof())
			{
				std::string line;
				std::getline(file, line);
				SUS::rtrim(line);
				lines.push_back(line);
			}
			file.close();
			return lines;
		};

		auto get_parms = [](const std::string& value) {
			std::pair<std::string, std::string> ret;
			auto sp = SUS::split(value, ',');
			for (auto& p : sp)
			{
				if (!ret.first.empty())
					ret.first += ", ";
				ret.first += p;
				if (!ret.second.empty())
					ret.second += ", ";
				auto p2 = p.substr(0, p.find('='));
				SUS::trim(p2);
				ret.second += p2;
			}
			if (ret.first == "void")
				ret.first.clear();
			if (ret.second == "void")
				ret.second.clear();
			return ret;
		};

		auto get_type2 = [](const std::string& type)->std::string {
			if (type == "std::string")
				return "const char*";
			if (type == "std::wstring")
				return "const wchar_t*";
			return type;
		};

		auto public_header_lines = get_file_lines(public_header_fn);
		Block public_header_blocks(public_header_lines);
		auto class_found = false;

		if (line == 0)
			get_class_name(path.stem().string());
		else
		{
			if (public_header_blocks.find_scope(std::regex("\\bstruct\\s+(\\w+)\\b"), match, line, it1))
			{
				class_name = match[1].str();
				class_found = true;
			}
			else
			{
				printf("cannot find class on line %d\n", line);
				system("pause");
				return 0;
			}
		}

		if (!class_found)
		{
			if (public_header_blocks.find(std::regex("\\bstruct\\s+" + class_name + "\\b"), match, it1))
				class_found = true;
		}

		if (!class_found)
		{
			printf("cannot find class %s\n", path.stem().string().c_str());
			return 0;
		}

		printf("class: %s\n", class_name.c_str());

		auto private_header_lines = get_file_lines(private_header_fn);
		auto source_lines = get_file_lines(source_fn);

		std::vector<std::string> anchor;
		if (line != 0)
		{
			auto p = line - 1;
			while (p >= 0)
			{
				std::string str;
				auto fn = path.filename().string();
				if (fn == public_header_fn)
					str = public_header_lines[p];
				else if (fn == private_header_fn)
					str = private_header_lines[p];
				else if (fn == source_fn)
					str = source_lines[p];
				if (str.size() > 1)
				{
					if (std::regex_search(str, match, std::regex("([\\w:\\*\\&]+)\\s+(\\w+)\\s*=")))
					{
						anchor.resize(3);
						anchor[0] = "A";
						anchor[1] = match[1].str();
						anchor[2] = match[2].str();
					}
					else if (std::regex_search(str, match, std::regex("([\\w:\\*\\&]+)\\s+(\\w+)\\(")))
					{
						anchor.resize(3);
						anchor[0] = "F";
						anchor[1] = match[1].str();
						anchor[2] = match[2].str();
					}
					break;
				}
				p--;
			}
		}

		Block private_header_blocks(private_header_lines);
		Block source_blocks(source_lines);

		if (cmd.empty())
		{
			const char* cmds[] = {
				"new_attribute",
				"new_function",
				"alter_item",
				"remove_item",
				"new_virtual_static"
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

		if (cmd == "new_attribute")
		{
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

			auto type2 = get_type2(type);

			if (public_header_blocks.find(std::regex("\\bstruct\\s+" + class_name + "\\b"), match, it1))
			{
				auto& list = it1->children;
				auto indent = get_indent(it1->b1) + '\t';
				ok2 = false;
				ok = false;
				if (!anchor.empty() && anchor[0] == "A")
				{
					if (it1->find(std::regex("virtual void set_" + anchor[2] + "\\("), match, it2))
					{
						it2++;
						ok = true;
					}
				}
				if (!ok)
				{
					it2 = list.end();
					ok = true;
				}
				if (ok)
				{
					list.emplace(it2, std::format(
						"{0}virtual {1} get_{2}() const = 0;\n"
						"{0}virtual void set_{2}({1} v) = 0;\n",
						indent, type2, name));
					ok2 = true;
				}
				if (ok2)
					public_header_blocks.output_file(public_header_fn);
			}

			if (private_header_blocks.find(std::regex("\\bstruct\\s+" + class_name + "Private\\s*:\\s*" + class_name), match, it1))
			{
				auto& list = it1->children;
				auto indent = get_indent(it1->b1) + '\t';
				ok2 = false;
				ok = false;
				if (!anchor.empty() && anchor[0] == "A")
				{
					if (it1->find(std::regex("" + anchor[1] + "\\s+" + anchor[2] + "\\s*="), match, it2))
					{
						it2++;
						ok = true;
					}
				}
				if (!ok)
				{
					it2 = list.begin();
					ok = true;
				}
				if (ok)
				{
					list.emplace(it2, std::format("{0}{1} {2}{3};\n", indent, type, name, !value.empty() ? " = " + value : ""));
					ok2 = true;
				}
				ok = false;
				if (!anchor.empty() && anchor[0] == "A")
				{
					if (it1->find(std::regex("void set_" + anchor[2] + "\\("), match, it2))
					{
						it2++;
						ok = true;
					}
				}
				if (!ok)
				{
					it2 = list.end();
					ok = true;
				}
				if (ok)
				{
					if (type == type2)
					{
						list.emplace(it2, std::format(
							"{0}{1} get_{2}() const override {{ return {2}; }}\n"
							"{0}void set_{2}({1} v) override;\n",
							indent, type2, name));
					}
					else
					{
						list.emplace(it2, std::format(
							"{0}{1} get_{2}() const override {{ return {2}.c_str(); }}\n"
							"{0}void set_{2}({1} v) override {{ set_{2}({3}(v)); }}\n"
							"{0}void set_{2}(const {3}& v);\n",
							indent, type2, name, type));
					}
					ok2 = true;
				}
				if (ok2)
					private_header_blocks.output_file(private_header_fn);
			}

			if (source_blocks.find(std::regex(class_name + "Private::\\~" + class_name + "Private\\(\\)"), match, it1))
			{
				auto range = it1->parent;
				auto& list = range->children;
				auto indent = get_indent(range->b1) + '\t';
				ok2 = false;
				ok = false;
				if (!anchor.empty() && anchor[0] == "A")
				{
					if (range->find(std::regex("void\\s+" + class_name + "Private::set_" + anchor[2] + "\\("), match, it2))
					{
						it2++;
						ok = true;
					}
				}
				if (!ok)
				{
					it2 = it1;
					it2++;
					ok = true;
				}
				if (ok)
				{
					if (type == type2)
					{
						list.emplace(it2, std::format(
							"\n"
							"{0}void {3}Private::set_{2}({1} v)\n"
							"{0}{{\n"
							"{0}\t{2} = v;\n"
							"{0}}}\n",
							indent, type2, name, class_name));
					}
					else
					{
						list.emplace(it2, std::format(
							"\n"
							"{0}void {3}Private::set_{2}(const {1}& v)\n"
							"{0}{{\n"
							"{0}\t{2} = v;\n"
							"{0}}}\n",
							indent, type, name, class_name));
					}
					ok2 = true;
				}
				if (ok2)
					source_blocks.output_file(source_fn);
			}
		}
		else if (cmd == "new_function")
		{
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
			auto [parms1, parms2] = get_parms(value);

			if (is_public && public_header_blocks.find(std::regex("\\bstruct\\s+" + class_name + "\\b"), match, it1))
			{
				auto& list = it1->children;
				auto indent = get_indent(it1->b1) + '\t';
				ok2 = false;
				ok = false;
				if (!anchor.empty() && anchor[0] == "F")
				{
					if (it1->find(std::regex("\\b" + anchor[1] + "\\s+" + anchor[2] + "\\("), match, it2))
					{
						it2++;
						ok = true;
					}
				}
				if (!ok)
				{
					it2 = list.end();
					ok = true;
				}
				if (ok)
				{
					list.emplace(it2, std::format("{0}{4} {1} {2}({3}){5}{6};",
						indent, type, name, parms1,
						(is_static ? (export_str + " static") : "virtual"),
						(is_const ? " const" : ""), (is_static ? "" : " = 0")));
					ok2 = true;
				}
				if (ok2)
					public_header_blocks.output_file(public_header_fn);
			}

			if (!is_static && private_header_blocks.find(std::regex("\\bstruct\\s+" + class_name + "Private\\s*:\\s*" + class_name), match, it1))
			{
				auto& list = it1->children;
				auto indent = get_indent(it1->b1) + '\t';
				ok2 = false;
				ok = false;
				if (!anchor.empty() && anchor[0] == "F")
				{
					if (it1->find(std::regex("" + anchor[1] + "\\s+" + anchor[2] + "\\("), match, it2))
					{
						it2++;
						ok = true;
					}
				}
				if (!ok)
				{
					it2 = list.end();
					ok = true;
				}
				if (ok)
				{
					list.emplace(it2, std::format("{0}{1} {2}({3}){4}{5};",
						indent, type, name, parms1, (is_const ? " const" : ""), (is_override ? " override" : "")));
					ok2 = true;
				}
				if (ok2)
					private_header_blocks.output_file(private_header_fn);
			}

			if (source_blocks.find(std::regex(class_name + "Private::\\~" + class_name + "Private\\(\\)"), match, it1))
			{
				auto range = it1->parent;
				auto& list = range->children;
				auto indent = get_indent(range->b1) + '\t';
				ok2 = false;
				ok = false;
				if (!anchor.empty() && anchor[0] == "F")
				{
					if (range->find(std::regex(anchor[1] + "\\s+" + class_name + "Private::" + anchor[2] + "\\("), match, it2))
					{
						it2++;
						ok = true;
					}
				}
				if (!ok)
				{
					it2 = it1;
					it2++;
					ok = true;
				}
				if (ok)
				{
					list.emplace(it2, std::format(
						"\n"
						"{0}{1} {4}{2}({3}){5}\n"
						"{0}{{\n\n"
						"{0}}}\n",
						indent, type, name, parms2,
						class_name + (is_static ? "::" : "Private::"),
						(is_const ? " const" : "")));
					ok2 = true;
				}
				if (ok2)
					source_blocks.output_file(source_fn);
			}
		}
		else if (cmd == "alter_item")
		{
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
			if (value.empty())
			{
				std::cout << "value: ";
				std::getline(std::cin, value);
			}

			if (private_header_blocks.find(std::regex("\\bstruct\\s+" + class_name + "Private\\s*:\\s*" + class_name), match, it1))
			{
				std::regex reg_attr = std::regex("([\\w:\\*\\&]+)\\s+" + name + "\\s*=\\s*\\w+;");
				if (it1->find(reg_attr, match, it2))
				{
					if (type.empty())
						type = match[1].str();

					it2->text = std::regex_replace(it2->text, reg_attr, type + " " + name2 + " = " + value + ";");
					std::regex reg1 = std::regex("[\\w:\\*\\&]+\\s+get_" + name + "\\(");
					if (it1->find(reg1, match, it2))
						it2->text = std::regex_replace(it2->text, reg1, type + " get_" + name2 + "(");
					std::regex reg2 = std::regex("void\\s+set_" + name + "\\([\\w:\\*\\&]+");
					if (it1->find(reg2, match, it2))
						it2->text = std::regex_replace(it2->text, reg2, "void set_" + name2 + "(" + type);

					private_header_blocks.output_file(private_header_fn);

					if (public_header_blocks.find(std::regex("\\bstruct\\s+" + class_name + "\\b"), match, it1))
					{
						ok2 = false;
						std::regex reg1 = std::regex("virtual\\s+[\\w:\\*\\&]+\\s+get_" + name + "\\(");
						if (it1->find(reg1, match, it2))
						{
							it2->text = std::regex_replace(it2->text, reg1, "virtual " + type + " get_" + name2 + "(");
							ok2 = true;
						}
						std::regex reg2 = std::regex("virtual\\s+void\\s+set_" + name + "\\([\\w:\\*\\&]+");
						if (it1->find(reg2, match, it2))
						{
							it2->text = std::regex_replace(it2->text, reg2, "virtual void set_" + name2 + "(" + type);
							ok2 = true;
						}
						if (ok2)
							public_header_blocks.output_file(public_header_fn);
					}

					ok2 = false;
					std::regex reg3 = std::regex("[\\w:\\*\\&]+\\s+" + class_name + "Private::get_" + name + "\\(");
					if (source_blocks.find(reg3, match, it1))
					{
						it1->text = std::regex_replace(it1->text, reg3, type + " " + class_name + "Private::get_" + name2 + "(");
						ok2 = true;
					}
					std::regex reg4 = std::regex("void\\s+" + class_name + "Private::set_" + name + "\\([\\w:\\*\\&]+");
					if (source_blocks.find(reg4, match, it1))
					{
						it1->text = std::regex_replace(it1->text, reg4, "void " + class_name + "Private::set_" + name2 + "(" + type);
						ok2 = true;
					}
					if (ok2)
						source_blocks.output_file(source_fn);

					return 0;
				}
				std::regex reg_func = std::regex("([\\w:\\*\\&]+)\\s+" + name + "\\(([\\w:\\*\\&,\\s]+)\\)");
				if (it1->find(reg_func, match, it2))
				{
					if (type.empty())
						type = match[1].str();
					if (value.empty())
						value = match[2].str();
					auto [parms1, parms2] = get_parms(value);

					it2->text = std::regex_replace(it2->text, reg_func, type + " " + name2 + "(" + parms1 + ")");

					private_header_blocks.output_file(private_header_fn);

					if (public_header_blocks.find(std::regex("\\bstruct\\s+" + class_name + "\\b"), match, it1))
					{
						ok2 = false;
						if (it1->find(reg_func, match, it2))
						{
							it2->text = std::regex_replace(it2->text, reg_func, type + " " + name2 + "(" + parms1 + ")");
							ok2 = true;
						}
						if (ok2)
							public_header_blocks.output_file(public_header_fn);
					}

					std::regex reg1 = std::regex("[\\w:\\*\\&]+\\s+" + class_name + "(Private)?::" + name + "\\([\\w:\\*\\&,\\s]+\\)");
					if (source_blocks.find(reg1, match, it1))
					{
						it1->text = std::regex_replace(it1->text, reg1, type + " " + class_name + (match[1].matched ? "Private" : "::") +
							name2 + "(" + parms2 + ")");
						ok2 = true;
					}
					if (ok2)
						source_blocks.output_file(source_fn);

					return 0;
				}
			}
		}
		else if (cmd == "remove_item")
		{
			if (name.empty())
			{
				std::cout << "name: ";
				std::getline(std::cin, name);
			}
			if (name.empty())
				return 0;

			if (private_header_blocks.find(std::regex("\\bstruct\\s+" + class_name + "Private\\s*:\\s*" + class_name), match, it1))
			{
				if (it1->find(std::regex("[\\w:\\*\\&]+\\s+" + name + "\\s*=|;"), match, it2))
				{
					it2->parent->children.erase(it2);

					if (it1->find(std::regex("[\\w:\\*\\&]+\\s+get_" + name + "\\("), match, it2))
						it2->parent->children.erase(it2);
					std::regex reg1 = std::regex("void\\s+set_" + name + "\\(");
					for (auto i = 0; i < 2; i++)
					{
						if (it1->find(reg1, match, it2))
							it2->parent->children.erase(it2);
					}

					private_header_blocks.output_file(private_header_fn);

					if (public_header_blocks.find(std::regex("\\bstruct\\s+" + class_name + "\\b"), match, it1))
					{
						ok2 = false;
						if (it1->find(std::regex("[\\w:\\*\\&]+\\s+get_" + name + "\\("), match, it2))
						{
							it2->parent->children.erase(it2);
							ok2 = true;
						}
						if (it1->find(std::regex("void\\s+set_" + name + "\\([\\w:\\*\\&]+"), match, it2))
						{
							it2->parent->children.erase(it2);
							ok2 = true;
						}
						if (ok2)
							public_header_blocks.output_file(public_header_fn);
					}

					ok2 = false;
					if (source_blocks.find(std::regex("[\\w:\\*\\&]+\\s+" + class_name + "Private::get_" + name + "\\("), match, it1))
					{
						it1->parent->children.erase(it1);
						ok2 = true;
					}
					if (source_blocks.find(std::regex("void\\s+" + class_name + "Private::set_" + name + "\\("), match, it1))
					{
						it1->parent->children.erase(it1);
						ok2 = true;
					}
					if (ok2)
						source_blocks.output_file(source_fn);

					return 0;
				}
				if (it1->find(std::regex("[\\w:\\*\\&]+\\s+" + name + "\\("), match, it2))
				{
					it2->parent->children.erase(it2);

					private_header_blocks.output_file(private_header_fn);

					if (public_header_blocks.find(std::regex("\\bstruct\\s+" + class_name + "\\b"), match, it1))
					{
						ok2 = false;
						if (it1->find(std::regex("\\b" + name + "\\("), match, it2))
						{
							it2->parent->children.erase(it2);
							ok2 = true;
						}
						if (ok2)
							public_header_blocks.output_file(public_header_fn);
					}

					ok2 = false;
					if (source_blocks.find(std::regex(class_name + "(Private)?::" + name + "\\("), match, it1))
					{
						it1->parent->children.erase(it1);
						ok2 = true;
					}
					if (ok2)
						source_blocks.output_file(source_fn);

					return 0;
				}
			}
		}
		else if (cmd == "new_virtual_static")
		{
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
				std::cout << "parameters: ";
				std::getline(std::cin, value);
			}
			if (value.empty())
				return 0;

			auto name2 = name;
			name2[0] = std::toupper(name2[0]);

			if (public_header_blocks.find(std::regex("\\bstruct\\s+" + class_name + "\\b"), match, it1))
			{
				auto& list = it1->children;
				auto indent = get_indent(it1->b1) + '\t';
				ok2 = false;
				ok = false;
				if (!ok)
				{
					it2 = list.end();
					ok = true;
				}
				if (ok)
				{
					list.emplace(it2, std::format(
						"{0}struct {1}\n"
						"{0}{{\n"
						"{0}\tvirtual {2} operator()({3}) = 0;\n"
						"{0}}};\n"
						"{0}{4} static {1}& {5};",
						indent, name2, type, value, export_str, name));
					ok2 = true;
				}
				if (ok2)
					public_header_blocks.output_file(public_header_fn);
			}

			if (source_blocks.find(std::regex(class_name + "Private::\\~" + class_name + "Private\\(\\)"), match, it1))
			{
				auto range = it1->parent;
				auto& list = range->children;
				auto indent = get_indent(range->b1) + '\t';
				ok2 = false;
				ok = false;
				if (!ok)
				{
					it2 = it1;
					it2++;
					ok = true;
				}
				if (ok)
				{
					list.emplace(it2, std::format(
						"\n"
						"{0}struct {1}{2} : {1}::{2}\n"
						"{0}{{\n"
						"{0}\t{3} operator()({4}) override\n"
						"{0}\t{{\n"
						"{0}\t}}\n"
						"{0}}}{6};\n"
						"{0}{1}::{2}& {1}::{5} = {6};\n",
						indent, class_name, name2, type, value, name, class_name + "_" + name));
					ok2 = true;
				}
				if (ok2)
					source_blocks.output_file(source_fn);
			}
		}
	}
	return 0;
}
