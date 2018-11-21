// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <flame/file.h>
#include <flame/typeinfo.h>
#include <flame/serialize.h>

#include <map>
#include <regex>

#include <Windows.h>
#include <dia2.h>
#include <atlbase.h>
#include <assert.h>

namespace flame
{
	namespace typeinfo
	{
		namespace cpp
		{
			struct EnumItemPrivate : EnumItem
			{
				std::string name;
				int value;
			};

			const char *EnumItem::name() const
			{
				return ((EnumItemPrivate*)this)->name.c_str();
			}

			int EnumItem::value() const
			{
				return ((EnumItemPrivate*)this)->value;
			}

			struct EnumTypePrivate : EnumType
			{
				std::string name;
				std::vector<std::unique_ptr<EnumItemPrivate>> items;

				inline String serialize_value(bool single, int v) const
				{
					if (single)
						return item(v)->name();

					return "";
				}
			};

			const char *EnumType::name() const
			{
				return ((EnumTypePrivate*)this)->name.c_str();
			}

			int EnumType::item_count() const
			{
				return ((EnumTypePrivate*)this)->items.size();
			}

			EnumItem *EnumType::item(int idx) const
			{
				return ((EnumTypePrivate*)this)->items[idx].get();
			}

			int EnumType::find_item(const char *name) const
			{
				auto &items = ((EnumTypePrivate*)this)->items;
				for (auto i = 0; i < items.size(); i++)
				{
					if (items[i]->name == name)
						return i;
				}
				return -1;
			}

			int EnumType::find_item(int value) const
			{
				auto &items = ((EnumTypePrivate*)this)->items;
				for (auto i = 0; i < items.size(); i++)
				{
					if (items[i]->value == value)
						return i;
				}
				return -1;
			}

			String EnumType::serialize_value(bool single, int v) const
			{
				return ((EnumTypePrivate*)this)->serialize_value(single, v);
			}

			struct VaribleInfoPrivate : VaribleInfo
			{
				VariableTag tag;
				std::string type_name;
				uint type_hash;
				std::string name;
				int offset;

				inline String serialize_value(void *src, bool is_obj, int precision) const
				{
					if (is_obj)
						src = (char*)src + offset;

					switch (tag)
					{
					case VariableTagEnumSingle:
					{
						auto e = find_enumeration(type_hash);
						return e->serialize_value(true, *(int*)src);
					}
						break;
					case VariableTagEnumMulti:
						break;
					case VariableTagVariable:
						switch (type_hash)
						{
						case cH("bool"):
							return *(bool*)src ? "true" : "false";
						case cH("uint"):
							return to_string(*(uint*)src);
						case cH("int"):
							return to_string(*(int*)src);
						case cH("float"):
							return to_string(*(float*)src, precision);
						case cH("Vec2"):
							return to_string(*(Vec2*)src, precision);
						case cH("Vec3"):
							return to_string(*(Vec3*)src, precision);
						case cH("Vec4"):
							return to_string(*(Vec3*)src, precision);
						case cH("Bvec4"):
							return to_string(*(Bvec4*)src);
						}
						break;
					}

					return "";
				}

				inline void unserialize_value(void *dst, bool is_obj, const char *_str) const
				{
					if (is_obj)
						dst = (char*)dst + offset;

					std::string str(_str);

					switch (tag)
					{
					case VariableTagEnumSingle:
					{
						auto e = find_enumeration(type_hash);
						*(int*)dst = e->find_item(str.c_str());
					}
						break;
					case VariableTagVariable:
						switch (type_hash)
						{
						case cH("bool"):
							*(bool*)dst = str == "true" ? true : false;
						case cH("uint"):
							*(uint*)dst = stoi1(str);
						case cH("int"):
							*(int*)dst = stoi1(str);
						case cH("float"):
							*(float*)dst = stof1(str);
						case cH("Vec2"):
							*(Vec2*)dst = stof2(str);
						case cH("Vec3"):
							*(Vec3*)dst = stof3(str);
						case cH("Vec4"):
							*(Vec4*)dst = stof4(str);
						case cH("Bvec4"):
							*(Bvec4*)dst = stob4(str);
						}
						break;
					}
				}
			};

			VariableTag VaribleInfo::tag() const
			{
				return ((VaribleInfoPrivate*)this)->tag;
			}

			const char *VaribleInfo::type_name() const
			{
				return ((VaribleInfoPrivate*)this)->type_name.c_str();
			}

			uint VaribleInfo::type_hash() const
			{
				return ((VaribleInfoPrivate*)this)->type_hash;
			}

			const char *VaribleInfo::name() const
			{
				return ((VaribleInfoPrivate*)this)->name.c_str();
			}

			int VaribleInfo::offset() const
			{
				return ((VaribleInfoPrivate*)this)->offset;
			}

			String VaribleInfo::serialize_value(void *src, bool is_obj, int precision) const
			{
				return ((VaribleInfoPrivate*)this)->serialize_value(src, is_obj, precision);
			}

			struct UDTPrivate : UDT
			{
				std::string name;
				std::vector<std::unique_ptr<VaribleInfoPrivate>> items;

				int find_pos;

				inline UDTPrivate()
				{
					find_pos = 0;
				}

				inline int find_item_i(const char *name)
				{
					if (items.empty())
						return -1;

					auto p = find_pos;
					while (true)
					{
						if (items[p]->name == name)
						{
							find_pos++;
							if (find_pos >= items.size())
								find_pos = 0;
							return p;
						}
						find_pos++;
						if (find_pos == p)
							return -1;
						if (find_pos >= items.size())
							find_pos = 0;
					}
					return -1;
				}
			};

			const char *UDT::name() const
			{
				return ((UDTPrivate*)this)->name.c_str();
			}

			int UDT::item_count() const
			{
				return ((UDTPrivate*)this)->items.size();
			}

			VaribleInfo *UDT::item(int idx) const
			{
				return ((UDTPrivate*)this)->items[idx].get();
			}

			int UDT::find_item_i(const char *name) const
			{
				return ((UDTPrivate*)this)->find_item_i(name);
			}

			static std::map<unsigned int, std::unique_ptr<cpp::EnumTypePrivate>> enums;
			static std::map<unsigned int, std::unique_ptr<cpp::UDTPrivate>> udts;

			int enumeration_count()
			{
				return enums.size();
			}

			cpp::EnumType *enumeration(int idx)
			{
				return enums[idx].get();
			}

			cpp::EnumType *find_enumeration(unsigned int name_hash)
			{
				auto it = enums.find(name_hash);
				return it == enums.end() ? nullptr : it->second.get();
			}

			int udt_count()
			{
				return udts.size();
			}

			cpp::UDT *udt(int idx)
			{
				return udts[idx].get();
			}

			cpp::UDT *find_udt(unsigned int name_hash)
			{
				auto it = udts.find(name_hash);
				return it == udts.end() ? nullptr : it->second.get();
			}
		}

		int initialize_collecting()
		{
			return FAILED(CoInitialize(NULL));
		}

		static const char* name_base_type[] = {
			"<NoType>",                         // btNoType = 0,
			"void",                             // btVoid = 1,
			"char",                             // btChar = 2,
			"wchar_t",                          // btWChar = 3,
			"signed char",
			"unsigned char",
			"int",                              // btInt = 6,
			"unsigned int",                     // btUInt = 7,
			"float",                            // btFloat = 8,
			"<BCD>",                            // btBCD = 9,
			"bool",                              // btBool = 10,
			"short",
			"unsigned short",
			"long",                             // btLong = 13,
			"unsigned long",                    // btULong = 14,
			"__int8",
			"__int16",
			"__int32",
			"__int64",
			"__int128",
			"unsigned __int8",
			"unsigned __int16",
			"unsigned __int32",
			"unsigned __int64",
			"unsigned __int128",
			"<currency>",                       // btCurrency = 25,
			"<date>",                           // btDate = 26,
			"VARIANT",                          // btVariant = 27,
			"<complex>",                        // btComplex = 28,
			"<bit>",                            // btBit = 29,
			"BSTR",                             // btBSTR = 30,
			"HRESULT",                          // btHresult = 31
			"char16_t",                         // btChar16 = 32
			"char32_t"                          // btChar32 = 33
		};

		static std::string get_base_type_name(IDiaSymbol *s)
		{
			DWORD baseType;
			s->get_baseType(&baseType);
			ULONGLONG len;
			s->get_length(&len);
			std::string name;
			switch (baseType)
			{
			case btUInt:
				name = "u";
			case btInt:
				switch (len)
				{
				case 1:
					name += "char";
					return name;
				case 2:
					name += "short";
					return name;
				case 4:
					name += "int";
					return name;
				case 8:
					name += "longlong";
					return name;
				}
				break;
			case btFloat:
				switch (len)
				{
				case 4:
					return "float";
				case 8:
					return "double";
				}
				break;
			}
			return name_base_type[baseType];
		}

		void collect(const wchar_t *pdb_dir, const wchar_t *pdb_prefix)
		{
			auto pdb_prefix_len = wcslen(pdb_prefix);
			std::string prefix("flame::");
			std::wstring wprefix(s2w(prefix));

			for (std::filesystem::directory_iterator end, it(pdb_dir); it != end; it++)
			{
				if (!std::filesystem::is_directory(it->status()) && it->path().extension() == L".pdb")
				{
					auto fn = it->path().filename().wstring();

					if (fn.compare(0, pdb_prefix_len, pdb_prefix) == 0)
					{
						CComPtr<IDiaDataSource> dia_source;
						if (FAILED(CoCreateInstance(CLSID_DiaSource, NULL, CLSCTX_INPROC_SERVER, __uuidof(IDiaDataSource), (void**)&dia_source)))
						{
							printf("dia not found\n");
							continue;
						}
						if (FAILED(dia_source->loadDataFromPdb(fn.c_str())))
						{
							printf("pdb failed to open\n");
							continue;
						}
						CComPtr<IDiaSession> session;
						if (FAILED(dia_source->openSession(&session)))
						{
							printf("session failed to open\n");
							continue;
						}
						CComPtr<IDiaSymbol> global;
						if (FAILED(session->get_globalScope(&global)))
						{
							printf("failed to get global\n");
							continue;
						}

						LONG l;
						ULONG ul = 0;
						IDiaEnumSymbols *symbols;
						IDiaSymbol *symbol;
						DWORD dw;
						wchar_t *pwname;
						std::regex reg_arr(prefix + R"(Array<([\w:]+)\s*(\*?)>)");

						global->findChildren(SymTagEnum, NULL, nsNone, &symbols);
						while (SUCCEEDED(symbols->Next(1, &symbol, &ul)) && (ul == 1))
						{
							symbol->get_name(&pwname);
							std::wstring wname(pwname);
							if (wname.compare(0, wprefix.size(), wprefix) == 0)
							{
								auto name = w2s(wname.c_str() + wprefix.size());
								auto hash = H(name.c_str());
								if (cpp::enums.find(hash) == cpp::enums.end())
								{
									auto e = new cpp::EnumTypePrivate;
									e->name = name;

									IDiaEnumSymbols *items;
									symbol->findChildren(SymTagNull, NULL, nsNone, &items);
									IDiaSymbol *item;
									while (SUCCEEDED(items->Next(1, &item, &ul)) && (ul == 1))
									{
										VARIANT v;
										ZeroMemory(&v, sizeof(v));
										item->get_name(&pwname);
										item->get_value(&v);

										auto i = new cpp::EnumItemPrivate;
										i->name = w2s(pwname);
										i->value = v.lVal;
										e->items.emplace_back(i);

										item->Release();
									}
									items->Release();

									cpp::enums.emplace(hash, e);
								}
							}
							symbol->Release();
						}
						symbols->Release();

						global->findChildren(SymTagUDT, NULL, nsNone, &symbols);
						while (SUCCEEDED(symbols->Next(1, &symbol, &ul)) && (ul == 1))
						{
							symbol->get_name(&pwname);
							std::wstring wname(pwname);
							if (wname.compare(0, wprefix.size(), wprefix) == 0)
							{
								auto udt_name = w2s(wname.c_str() + wprefix.size());
								auto udt_namehash = H(udt_name.c_str());

								IDiaEnumSymbols *bases;
								symbol->findChildren(SymTagBaseClass, NULL, nsNone, &bases);
								bases->get_Count((LONG*)&ul);
								if (ul == 1)
								{
									IDiaSymbol *base;
									bases->Item(0, &base);
									base->get_name(&pwname);
									base->Release();
									std::wstring wname(pwname);
									if (wname == wprefix + L"R")
									{
										if (cpp::udts.find(udt_namehash) == cpp::udts.end())
										{
											auto s = new cpp::UDTPrivate;
											s->name = udt_name;

											IDiaEnumSymbols *members;
											symbol->findChildren(SymTagData, NULL, nsNone, &members);
											IDiaSymbol *member;
											while (SUCCEEDED(members->Next(1, &member, &ul)) && (ul == 1))
											{
												member->get_name(&pwname);
												std::wstring wname(pwname);
												if (wname.size() > 1 && wname.back() == L'$')
												{
													auto double_$ = wname.size() > 2 && wname[wname.size() - 2] == L'$';


													member->get_offset(&l);

													auto i = new cpp::VaribleInfoPrivate;
													i->name = w2s(wname);
													i->name.resize(i->name.size() - 1);
													i->offset = l;

													IDiaSymbol *type;
													member->get_type(&type);
													type->get_symTag(&dw);
													switch (dw)
													{
													case SymTagEnum:
													{
														i->tag = double_$ ? cpp::VariableTagEnumMulti : cpp::VariableTagEnumSingle;
														type->get_name(&pwname);
														auto type_name = w2s(pwname);
														if (type_name.compare(0, prefix.size(), prefix) == 0)
															type_name = type_name.c_str() + prefix.size();
														i->type_name = type_name;
													}
														break;
													case SymTagBaseType:
													{
														i->tag = cpp::VariableTagVariable;
														i->type_name = get_base_type_name(type);
													}
														break;
													case SymTagPointerType:
													{
														i->tag = cpp::VariableTagPointer;
														IDiaSymbol *point_type;
														type->get_type(&point_type);
														point_type->get_symTag(&dw);
														switch (dw)
														{
														case SymTagBaseType:
															i->type_name = get_base_type_name(point_type);
															break;
														case SymTagPointerType:
															assert(0);
															break;
														case SymTagUDT:
															point_type->get_name(&pwname);
															auto type_name = w2s(pwname);
															if (type_name.compare(0, prefix.size(), prefix) == 0)
																type_name = type_name.c_str() + prefix.size();
															i->type_name = type_name;
															break;
														}
														point_type->Release();
													}
														break;
													case SymTagUDT:
													{
														type->get_name(&pwname);
														auto type_name = w2s(pwname);
														std::smatch match;
														if (std::regex_search(type_name, match, reg_arr))
														{
															if (match[2].matched)
																i->tag = cpp::VariableTagArrayOfPointer;
															else
																i->tag = cpp::VariableTagArrayOfVariable;
															type_name = match[1].str().c_str();
														}
														else
															i->tag = cpp::VariableTagVariable;
														if (type_name.compare(0, prefix.size(), prefix) == 0)
															type_name = type_name.c_str() + prefix.size();
														i->type_name = type_name;
													}
														break;
													}
													type->Release();

													i->type_hash = H(i->type_name.c_str());
													s->items.emplace_back(i);
												}
												member->Release();
											}
											members->Release();

											cpp::udts.emplace(udt_namehash, s);
										}
									}
								}
								bases->Release();
							}
							symbol->Release();
						}
						symbols->Release();
					}
				}
			}
		}

		static const char *tag_name[] = {
			"enum",
			"varible",
			"pointer",
			"array_of_varible",
			"array_of_pointer"
		};

		void load(const wchar_t *filename)
		{
			auto xml = XmlFile::create_from_file(filename);
			if (!xml)
				return;

			auto rn = xml->root_node;

			auto n_cpp = rn->find_node("cpp");

			auto n_enums = n_cpp->find_node("enums");
			for (auto i = 0; i < n_enums->node_count(); i++)
			{
				auto n_enum = n_enums->node(i);
				if (n_enum->name() == "enum")
				{
					auto e = new cpp::EnumTypePrivate;
					e->name = n_enum->find_attr("name")->value();

					for (auto j = 0; j < n_enum->node_count(); j++)
					{
						auto n_item = n_enum->node(j);
						if (n_item->name() == "item")
						{
							auto i = new cpp::EnumItemPrivate;
							i->name = n_item->find_attr("name")->value();
							i->value = std::stoi(n_item->find_attr("value")->value());
							e->items.emplace_back(i);
						}
					}

					cpp::enums.emplace(H(e->name.c_str()), e);
				}
			}

			auto n_serializables = n_cpp->find_node("udts");
			for (auto i = 0; i < n_serializables->node_count(); i++)
			{
				auto n_udt = n_serializables->node(i);
				if (n_udt->name() == "udt")
				{
					auto u = new cpp::UDTPrivate;
					u->name = n_udt->find_attr("name")->value();

					for (auto j = 0; j < n_udt->node_count(); j++)
					{
						auto n_item = n_udt->node(j);
						if (n_item->name() == "item")
						{
							auto tag = n_item->find_attr("tag")->value();
							auto e_tag = 0;
							for (auto s : tag_name)
							{
								if (tag == s)
									break;
								e_tag++;
							}

							auto i = new cpp::VaribleInfoPrivate;
							i->tag = (cpp::VariableTag)e_tag;
							i->type_name = n_item->find_attr("type")->value();
							i->type_hash = H(i->type_name.c_str());
							i->name = n_item->find_attr("name")->value();
							i->offset = std::stoi(n_item->find_attr("offset")->value());
							u->items.emplace_back(i);
						}
					}

					cpp::udts.emplace(H(u->name.c_str()), u);
				}
			}
		}

		void save(const wchar_t *filename)
		{
			auto xml = XmlFile::create("typeinfo");
			auto rn = xml->root_node;

			auto n_cpp = rn->new_node("cpp");

			auto n_enums = n_cpp->new_node("enums");
			for (auto &e : cpp::enums)
			{
				auto n_enum = n_enums->new_node("enum");
				n_enum->new_attr("name", e.second->name);

				for (auto &i : e.second->items)
				{
					auto n_item = n_enum->new_node("item");
					n_item->new_attr("name", i->name);
					n_item->new_attr("value", std::to_string(i->value));
				}
			}

			auto n_serializables = n_cpp->new_node("udts");
			for (auto &u : cpp::udts)
			{
				auto n_udt = n_serializables->new_node("udt");
				n_udt->new_attr("name", u.second->name);

				for (auto &i : u.second->items)
				{
					auto n_item = n_udt->new_node("item");
					n_item->new_attr("tag", tag_name[i->tag]);
					n_item->new_attr("type", i->type_name);
					n_item->new_attr("name", i->name);
					n_item->new_attr("offset", std::to_string(i->offset));
				}
			}

			xml->save(filename);
			XmlFile::destroy(xml);
		}

		void clear()
		{
			cpp::enums.clear();
		}
	}
}
