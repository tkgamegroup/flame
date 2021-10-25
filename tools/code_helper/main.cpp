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
	auto ppath = path.parent_path();
	auto name = ap.get_item("-name");
	auto name2 = ap.get_item("-name2");
	auto type = ap.get_item("-type");
	auto value = ap.get_item("-value");
	auto line = sto<int>(ap.get_item("-line"));

	auto flame_path = std::filesystem::path(getenv("FLAME_PATH"));
	flame_path.make_preferred();
	auto is_internal = ppath.string().starts_with(flame_path.string());

	std::string class_name;
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

	if (std::filesystem::is_directory(path))
	{
		printf("current directory: %s\n", path.string().c_str());

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
			source_file << indent_str << class_name << "* " << class_name << "::create(void* parms)\n\t{\n";
			source_file << indent_str << "\treturn new " << class_name << "Private();\n";
			source_file << indent_str << "}\n";
			if (is_internal) source_file << "}\n";
			source_file.close();

			cmd.clear();
		}
	}
	else
	{
		struct Block
		{
			std::string text = "`";
			std::list<Block> children;
			std::string b1;
			std::string b2;
			Block* parent = nullptr;

			Block() {}

			Block(const std::string& line) :
				text(line)
			{
			}

			Block(const std::vector<std::string>& lines)
			{
				std::function<int(Block&, int, int)> gather_block;
				gather_block = [&](Block& b, int line_id, int bracket_level) {
					for (auto i = line_id; i < lines.size(); i++)
					{
						auto& l = lines[i];
						auto tl = l;
						SUS::ltrim(tl);
						if (tl == "{")
						{
							if (bracket_level == 0)
							{
								auto& nb = b.children.emplace_back();
								nb.b1 = l;
								i = gather_block(nb, i + 1, bracket_level);
							}
							else
							{
								b.children.emplace_back(l);
								bracket_level++;
							}
						}
						else if (tl == "}" || tl == "};")
						{
							if (bracket_level == 0)
							{
								b.b2 = l;
								return i;
							}
							else
							{
								b.children.emplace_back(l);
								bracket_level--;
							}
						}
						else
						{
							bracket_level += std::count(l.begin(), l.end(), '{');
							bracket_level -= std::count(l.begin(), l.end(), '}');
							b.children.emplace_back(l);
						}
					}
					return (int)lines.size();
				};

				gather_block(*this, 0, 0);
				init();
			}

			void init()
			{
				for (auto it = children.begin(); it != children.end();)
				{
					it->parent = this;
					if (it->children.empty() && it->text.size() > 1 && it->text.back() != ';')
					{
						auto it2 = it; it2++;
						if (it2 != children.end() && !it2->children.empty())
						{
							it->children = it2->children;
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

			void output_file(const std::string& fn)
			{
				auto text = output();
				text.pop_back();
				std::ofstream file(fn);
				file << text;
				file.close();
			}
		};

		printf("current directory: %s\n", ppath.string().c_str());
		std::filesystem::current_path(ppath);

		std::string public_header_fn;
		std::string private_header_fn;
		std::string source_fn;
		std::smatch match;
		std::list<Block>::iterator it1, it2;
		bool ok;

		{
			auto fn = path.stem().string();
			SUS::cut_tail_if(fn, "_private");

			get_class_name(fn);

			public_header_fn = fn + ".h";
			private_header_fn = fn + "_private.h";
			source_fn = fn + ".cpp";
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
			for (auto ch : str)
			{
				if (!std::isspace(ch))
					break;
				ret += ch;
			}
			return ret;
		};

		auto get_file_lines = [](const std::string& fn) {
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

		auto public_header_lines = get_file_lines(public_header_fn);

		Block public_header_blocks(public_header_lines);
		if (public_header_blocks.find(std::regex("^\\s*struct\\s+(c|s)?" + class_name + "\\b"), match, it1))
		{
			class_name = match[1].str() + class_name;
			printf("class: %s\n", class_name.c_str());

			auto private_header_lines = get_file_lines(private_header_fn);
			auto source_lines = get_file_lines(source_fn);

			std::vector<std::string> anchor;
			if (line != 0)
			{
				auto p = line - 1;
				while (p >= 0)
				{
					auto& str = private_header_lines[p];
					if (str.size() > 1)
					{
						if (std::regex_search(str, match, std::regex("^\\s*([\\w:]+)\\s+(\\w+)\\s*=")))
						{
							anchor.resize(3);
							anchor[0] = "A";
							anchor[1] = match[1].str();
							anchor[2] = match[2].str();
						}
						else if (std::regex_search(str, match, std::regex("^\\s*([\\w:]+)\\s+(\\w+)\\(")))
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
				if (value.empty())
					return 0;

				if (public_header_blocks.find(std::regex("^\\s*struct\\s+" + class_name + "\\b"), match, it1))
				{
					auto& list = it1->children;
					auto indent = get_indent(it1->b1) + '\t';
					ok = false;
					if (anchor.empty() || anchor[0] != "A")
					{
						if (it1->find(std::regex("^\\s*\\w+\\s+static\\s+" + class_name + "\\s*\\*\\s+create\\("), match, it2))
							ok = true;
					}
					else
					{
						if (it1->find(std::regex("^\\s*virtual void set_" + anchor[2] + "\\("), match, it2))
						{
							it2++;
							ok = true;
						}
					}
					if (ok)
					{
						list.emplace(it2, std::format(
							"{0}virtual {1} get_{2}() const = 0;\n"
							"{0}virtual void set_{2}({1} v) = 0;\n",
							indent, type, name));
					}
					public_header_blocks.output_file(public_header_fn);
				}

				if (private_header_blocks.find(std::regex("^\\s*struct\\s+" + class_name + "Private\\s*:\\s*" + class_name), match, it1))
				{
					auto& list = it1->children;
					auto indent = get_indent(it1->b1) + '\t';
					ok = false;
					if (anchor.empty() || anchor[0] != "A")
					{
						it2 = list.begin();
						ok = true;
					}
					else
					{
						if (it1->find(std::regex("^\\s*" + anchor[1] + "\\s+" + anchor[2] + "\\s*="), match, it2))
						{
							it2++;
							ok = true;
						}
					}
					if (ok)
						list.emplace(it2, std::format("{0}{1} {2} = {3};\n", indent, type, name, value));
					ok = false;
					if (anchor.empty() || anchor[0] != "A")
					{
						it2 = list.end();
						ok = true;
					}
					else
					{
						if (it1->find(std::regex("^\\s*void set_" + anchor[2] + "\\("), match, it2))
						{
							it2++;
							ok = true;
						}
					}
					if (ok)
					{
						list.emplace(it2, std::format(
							"{0}{1} get_{2}() const override { return {2}; }\n"
							"{0}void set_{2}({1} v) override;\n",
							indent, type, name));
					}
					private_header_blocks.output_file(private_header_fn);
				}

				if (source_blocks.find(std::regex("^\\s*" + class_name + "\\s*\\*\\s+" + class_name + "::create\\("), match, it1))
				{
					auto range = it1->parent;
					auto& list = range->children;
					auto indent = get_indent(range->b1) + '\t';
					ok = false;
					if (anchor.empty() || anchor[0] != "A")
					{
						it2 = it1;
						it2--;
						ok = true;
					}
					else
					{
						if (range->find(std::regex("^\\s*void\\s+" + class_name + "Private::set_" + anchor[2] + "\\("), match, it2))
						{
							it2++;
							ok = true;
						}
					}
					if (ok)
					{
						list.emplace(it2, std::format(
							"\n"
							"{0}void {3}Private::set_{2}({1} v)\n"
							"{0}{\n"
							"{0}\t{2} = v;\n"
							"{0}}\n",
							indent, type, name, class_name));
					}
					source_blocks.output_file(source_fn);
				}

				name.clear();
				type.clear();
				value.clear();
			}
			else if (cmd == "remove_attribute")
			{
				if (name.empty())
				{
					std::cout << "name: ";
					std::getline(std::cin, name);
				}
				if (name.empty())
					return 0;

				if (public_header_blocks.find(std::regex("^\\s*struct\\s+" + class_name + "\\b"), match, it1))
				{
					if (it1->find(std::regex("^\\s*virtual\\s+[\\w:]+\\s+get_" + name + "\\("), match, it2))
						it2->parent->children.erase(it2);
					if (it1->find(std::regex("^\\s*virtual\\s+void\\s+set_" + name + "\\([\\w:]+"), match, it2))
						it2->parent->children.erase(it2);
				}
				public_header_blocks.output_file(public_header_fn);

				if (private_header_blocks.find(std::regex("^\\s*struct\\s+" + class_name + "Private\\s*:\\s*" + class_name), match, it1))
				{
					if (it1->find(std::regex("^\\s*[\\w:]+\\s+" + name + "\\s*="), match, it2))
						it2->parent->children.erase(it2);
					if (it1->find(std::regex("^\\s*[\\w:]+\\s+get_" + name + "\\("), match, it2))
						it2->parent->children.erase(it2);
					if (it1->find(std::regex("^\\s*void\\s+set_" + name + "\\("), match, it2))
						it2->parent->children.erase(it2);
				}
				private_header_blocks.output_file(private_header_fn);

				if (source_blocks.find(std::regex("^\\s*[\\w:]+\\s+" + class_name + "Private::get_" + name + "\\("), match, it1))
					it1->parent->children.erase(it1);
				if (source_blocks.find(std::regex("^\\s*void\\s+" + class_name + "Private::set_" + name + "\\("), match, it1))
					it1->parent->children.erase(it1);
				source_blocks.output_file(source_fn);

				name.clear();
			}
			else if (cmd == "alter_attribute")
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

				if (private_header_blocks.find(std::regex("^\\s*struct\\s+" + class_name + "Private\\s*:\\s*" + class_name), match, it1))
				{
					std::regex reg1 = std::regex("^\\s*[\\w:]+\\s+" + name + "\\s*=");
					if (it1->find(reg1, match, it2))
					{
						if (type.empty())
						{
							std::smatch res;
							if (std::regex_search(it2->text, res, std::regex("^[\\w:]+")))
								type = res[0].str();
						}
						if (!type.empty())
						{
							it2->text = std::regex_replace(it2->text, reg1, type + " " + name2 + " = ");
							{
								std::regex reg1 = std::regex("^\\s*[\\w:]+\\s+get_" + name + "\\(");
								if (it1->find(reg1, match, it2))
									it2->text = std::regex_replace(it2->text, reg1, type + " get_" + name2 + "(");
								std::regex reg2 = std::regex("^\\s*void\\s+set_" + name + "\\([\\w:]+");
								if (it1->find(reg2, match, it2))
									it2->text = std::regex_replace(it2->text, reg2, "void set_" + name2 + "(" + type);
							}

							private_header_blocks.output_file(private_header_fn);
						}
					}
				}

				if (!type.empty())
				{
					if (public_header_blocks.find(std::regex("^\\s*struct\\s+" + class_name + "\\b"), match, it1))
					{
						std::regex reg1 = std::regex("^\\s*virtual\\s+[\\w:]+\\s+get_" + name + "\\(");
						if (it1->find(reg1, match, it2))
							it2->text = std::regex_replace(it2->text, reg1, "virtual " + type + " get_" + name2 + "(");
						std::regex reg2 = std::regex("^\\s*virtual\\s+void\\s+set_" + name + "\\([\\w:]+");
						if (it1->find(reg2, match, it2))
							it2->text = std::regex_replace(it2->text, reg2, "virtual void set_" + name2 + "(" + type);
					}
					public_header_blocks.output_file(public_header_fn);
				}

				if (!type.empty())
				{
					{
						std::regex reg = std::regex("^\\s*[\\w:]+\\s+" + class_name + "Private::get_" + name + "\\(");
						if (source_blocks.find(reg, match, it1))
							it1->text = std::regex_replace(it1->text, reg, type + " " + class_name + "Private::get_" + name2 + "(");
					}
					{
						std::regex reg = std::regex("^\\s*void\\s+" + class_name + "Private::set_" + name + "\\([\\w:]+");
						if (source_blocks.find(reg, match, it1))
							it1->text = std::regex_replace(it1->text, reg, "void " + class_name + "Private::set_" + name2 + "(" + type);
					}
					source_blocks.output_file(source_fn);
				}

				name.clear();
				name2.clear();
				type.clear();
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

				if (is_public && public_header_blocks.find(std::regex("^\\s*struct\\s+" + class_name + "\\b"), match, it1))
				{
					auto& list = it1->children;
					auto indent = get_indent(it1->b1) + '\t';
					ok = false;
					if (anchor.empty() || anchor[0] != "F")
					{
						if (it1->find(std::regex("^\\s*\\w+\\s+static\\s+" + class_name + "\\s*\\*\\s+create\\("), match, it2))
							ok = true;
					}
					else
					{
						if (it1->find(std::regex("\\b" + anchor[1] + "\\s+" + anchor[2] + "\\("), match, it2))
						{
							it2++;
							ok = true;
						}
					}
					if (ok)
					{
						list.emplace(it2, std::format("{0}{4} {1} {2}({3}){5}{6};",
							indent, type, name, parameters1,
							(is_static ? (is_internal ? "FLAME_UNIVERSE_EXPORTS static" : "__declspec(dllexport) static") : "virtual"),
							(is_const ? " const" : ""), (is_static ? "" : " = 0")));
					}
					public_header_blocks.output_file(public_header_fn);
				}

				if (!is_static && private_header_blocks.find(std::regex("^\\s*struct\\s+" + class_name + "Private\\s*:\\s*" + class_name), match, it1))
				{
					auto& list = it1->children;
					auto indent = get_indent(it1->b1) + '\t';
					ok = false;
					if (anchor.empty() || anchor[0] != "F")
					{
						it2 = list.end();
						ok = true;
					}
					else
					{
						if (it1->find(std::regex("^\\s*" + anchor[1] + "\\s+" + anchor[2] + "\\("), match, it2))
						{
							it2++;
							ok = true;
						}
					}
					if (ok)
					{
						list.emplace(it2, std::format("{0}{1} {2}({3}){4}{5}",
							indent, type, name, parameters1, (is_const ? " const" : ""), (is_override ? " override" : "")));
					}
					private_header_blocks.output_file(private_header_fn);
				}

				if (source_blocks.find(std::regex("^\\s*" + class_name + "\\s*\\*\\s+" + class_name + "::create\\("), match, it1))
				{
					auto range = it1->parent;
					auto& list = range->children;
					auto indent = get_indent(range->b1) + '\t';
					ok = false;
					if (anchor.empty() || anchor[0] != "F")
					{
						it2 = it1;
						it2--;
						ok = true;
					}
					else
					{
						if (range->find(std::regex("^\\s*" + anchor[1] + "\\s+" + class_name + "Private::" + anchor[2] + "\\("), match, it2))
						{
							it2++;
							ok = true;
						}
					}
					if (ok)
					{
						list.emplace(it2, std::format(
							"\n"
							"{0}{1} {4}{2}({3}){5}\n"
							"{0}{\n"
							"{0}}\n",
							indent, type, name, parameters2,
							class_name + (is_static ? "::" : "Private::"),
							(is_const ? "const" : "")));
					}
					source_blocks.output_file(source_fn);
				}

				name.clear();
				type.clear();
				value.clear();
			}
			else if (cmd == "remove_function")
			{
				if (name.empty())
				{
					std::cout << "name: ";
					std::getline(std::cin, name);
				}
				if (name.empty())
					return 0;

				if (public_header_blocks.find(std::regex("^\\s*struct\\s+" + class_name + "\\b"), match, it1))
				{
					if (it1->find(std::regex("\\b" + name + "\\("), match, it2))
						it2->parent->children.erase(it2);
					public_header_blocks.output_file(public_header_fn);
				}

				if (private_header_blocks.find(std::regex("^\\s*struct\\s+" + class_name + "Private\\s*:\\s*" + class_name), match, it1))
				{
					if (it1->find(std::regex("\\b" + name + "\\("), match, it2))
						it2->parent->children.erase(it2);
					it2->parent->children.erase(it2);
				}

				if (source_blocks.find(std::regex(class_name + "(Private)?::" + name + "\\("), match, it1))
				{
					it1->parent->children.erase(it1);
					source_blocks.output_file(source_fn);
				}

				name.clear();
			}

			cmd.clear();
		}
		else
		{
			printf("cannot find class\n");
			return 0;
		}
	}
	return 0;
}
