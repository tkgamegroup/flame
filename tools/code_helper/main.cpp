#include <flame/foundation/foundation.h>

#include <iostream>
#include <functional>

using namespace flame;

int main(int argc, char **args)
{
	if (argc >= 2)
	{
		auto ap = parse_args(argc, args);
		auto cmd = ap.get_item("-cmd");
		auto path = ap.get_item("-path");
		auto name = ap.get_item("-name");
		auto type = ap.get_item("-type");
		auto value = ap.get_item("-value");

		auto get_class_name = [](const std::string& n) {
			auto class_name = n;
			class_name[0] = std::toupper(class_name[0]);
			for (auto i = 0; i < class_name.size(); i++)
			{
				if (class_name[i] == '_')
					class_name[i + 1] = std::toupper(class_name[i + 1]);
			}
			SUS::remove_ch(class_name, '_');
			class_name = "c" + class_name;
			return class_name;
		};

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

			bool find(const std::regex& r, std::list<Block>::iterator& out_it)
			{
				for (auto it = children.begin(); it != children.end(); it++)
				{
					if (std::regex_search(it->line, r))
					{
						out_it = it;
						return true;
					}
					if (it->find(r, out_it))
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
		};

		auto gather_blocks = [&](std::ifstream& f, Block& top_block) {
			std::vector<std::string> lines;
			while (!f.eof())
			{
				std::string line;
				std::getline(f, line);
				SUS::trim(line);
				lines.push_back(line + '\n');
			}
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

			gather_block(top_block, 0);
			top_block.init();
		};

		if (cmd == "new_component")
		{
			if (name.empty())
			{
				std::cout << "name: ";
				std::cin >> name;
			}
			if (name.empty())
				return 0;

			auto class_name = get_class_name(name);

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
			public_header_file << "\t\t" << class_name << "() : Component(type_name, type_hash)\n";
			public_header_file << "\t\t" << (internal ? "FLAME_UNIVERSE_EXPORTS" : "__declspec(dllexport)") << " static " << class_name << "* create(void* parms = nullptr);\n";
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
		else if (cmd == "new_attribute")
		{
			if (path.empty())
				return 0;
			if (name.empty())
			{
				std::cout << "name: ";
				std::cin >> name;
			}
			if (name.empty())
				return 0;
			if (type.empty())
			{
				std::cout << "type: ";
				std::cin >> type;
			}
			if (type.empty())
				return 0;
			if (value.empty())
			{
				std::cout << "value: ";
				std::cin >> value;
			}
			if (value.empty())
				return 0;

			auto p = std::filesystem::path(path);
			auto pp = p.parent_path();

			auto fn = p.stem().string();
			SUS::cut_tail_if(fn, "_private");
			auto class_name = get_class_name(fn);
			auto public_header_fn = pp.string() + "/" + fn + ".h";
			auto private_header_fn = pp.string() + "/" + fn + "_private.h";
			auto source_fn = pp.string() + "/" + fn + ".cpp";
			if (!std::filesystem::exists(public_header_fn) || !std::filesystem::exists(private_header_fn) || !std::filesystem::exists(source_fn))
			{
				printf("cannot find %s or %s or %s\n", public_header_fn.c_str(), private_header_fn.c_str(), source_fn.c_str());
				system("pause");
				return 0;
			}

			std::ifstream public_header_ifile(public_header_fn);
			Block public_header_blocks;
			gather_blocks(public_header_ifile, public_header_blocks);
			public_header_ifile.close();
			{
				std::list<Block>::iterator it;
				if (public_header_blocks.find(std::regex("^\\w+\\s+static\\s+" + class_name + "\\s*\\*\\s+create\\("), it))
				{
					auto& list = it->parent->children;
					list.emplace(it, "virtual " + type + " get_" + name + "() const = 0;\n");
					list.emplace(it, "virtual void set_" + name + "(" + type + " v) = 0;\n");
					list.emplace(it, "\n");
				}
			}
			std::ofstream public_header_ofile(public_header_fn);
			public_header_ofile << public_header_blocks.output(0);
			public_header_ofile.close();

			std::ifstream private_header_ifile(private_header_fn);
			Block private_header_blocks;
			gather_blocks(private_header_ifile, private_header_blocks);
			private_header_ifile.close();
			{
				std::list<Block>::iterator it;
				if (private_header_blocks.find(std::regex("^struct\\s+" + class_name + "Private\\s*:\\s*" + class_name), it))
				{
					auto& list = it->children;
					it = it->children.begin();
					for (; it != list.end(); it++)
					{
						if (it->type != 0 || it->line == "\n")
							break;
					}
					list.emplace(it, type + " " + name + " = " + value + ";\n");
					list.emplace_back(type + " get_" + name + "() const override { return " + name + "; }\n");
					list.emplace_back("void set_" + name + "(" + type + " v) override;\n");
				}
			}
			auto wtf0 = private_header_blocks.output(0);
			std::ofstream private_header_ofile(private_header_fn);
			private_header_ofile << private_header_blocks.output(0);
			private_header_ofile.close();

			std::ifstream source_ifile(source_fn);
			Block source_blocks;
			gather_blocks(source_ifile, source_blocks);
			source_ifile.close();
			{
				std::list<Block>::iterator it;
				if (source_blocks.find(std::regex("^" + class_name + "\\s*\\*\\s+" + class_name + "::create\\("), it))
				{
					auto& list = it->parent->children;
					auto& nb = *list.emplace(it, "void " + class_name + "Private::set_" + name + "(" + type + " v)\n");
					nb.type = 1;
					nb.children.emplace_back(name + " = v;\n");
					list.emplace(it, "\n");
				}
			}
			std::ofstream source_ofile(source_fn);
			source_ofile << source_blocks.output(0);
			source_ofile.close();
		}
		else if (cmd == "remove_attribute")
		{
			if (path.empty())
				return 0;
			if (name.empty())
			{
				std::cout << "name: ";
				std::cin >> name;
			}
			if (name.empty())
				return 0;

			auto p = std::filesystem::path(path);
			auto pp = p.parent_path();

			auto fn = p.stem().string();
			SUS::cut_tail_if(fn, "_private");
			auto class_name = get_class_name(fn);
			auto public_header_fn = pp.string() + "/" + fn + ".h";
			auto private_header_fn = pp.string() + "/" + fn + "_private.h";
			auto source_fn = pp.string() + "/" + fn + ".cpp";
			if (!std::filesystem::exists(public_header_fn) || !std::filesystem::exists(private_header_fn) || !std::filesystem::exists(source_fn))
			{
				printf("cannot find %s or %s or %s\n", public_header_fn.c_str(), private_header_fn.c_str(), source_fn.c_str());
				system("pause");
				return 0;
			}

			//std::ifstream public_header_ifile(public_header_fn);
			//auto public_header_blocks = gather_blocks(public_header_ifile);
			//public_header_ifile.close();
			//{
			//	auto n = public_header_blocks->find(std::regex("^virtual\\s+" + type + "\\s+get_" + name + "\\("));
			//	if (n)
			//		n->type = 4;
			//	n = public_header_blocks->find(std::regex("^virtual\\s+void\\s+set_" + name + "\\("));
			//	if (n)
			//		n->type = 4;
			//}
			//auto wtf = public_header_blocks->output(0);
			//std::ofstream public_header_ofile(public_header_fn);
			//public_header_ofile << public_header_blocks->output(0);
			//public_header_ofile.close();

			//std::ifstream private_header_ifile(private_header_fn);
			//auto private_header_blocks = gather_blocks(private_header_ifile);
			//private_header_ifile.close();
			//{

			//}
			//std::ofstream private_header_ofile(private_header_fn);
			//private_header_ofile << private_header_blocks->output(0);
			//private_header_ofile.close();

			//std::ifstream source_ifile(source_fn);
			//auto source_blocks = gather_blocks(source_ifile);
			//source_ifile.close();
			//{

			//}
			//std::ofstream source_ofile(source_fn);
			//source_ofile << source_blocks->output(0);
			//source_ofile.close();
		}
	}
	return 0;
}
