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

#include <vector>
#include <memory>
#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include <rapidxml_print.hpp>

namespace flame
{
	struct XmlAttributePrivate : XmlAttribute
	{
		std::string name;
		std::string value;
	};

	const std::string &XmlAttribute::name() const
	{
		return ((XmlAttributePrivate*)this)->name;
	}

	const std::string &XmlAttribute::value() const
	{
		return ((XmlAttributePrivate*)this)->value;
	}

	void XmlAttribute::set_name(const std::string &name)
	{
		((XmlAttributePrivate*)this)->name = name;
	}

	void XmlAttribute::set_value(const std::string &value)
	{
		((XmlAttributePrivate*)this)->value = value;
	}

	struct XmlNodePrivate : XmlNode
	{
		std::string name;
		std::string value;

		std::vector<std::unique_ptr<XmlAttributePrivate>> attrs;
		std::vector<std::unique_ptr<XmlNodePrivate>> nodes;

		int attr_find_pos;
		int node_find_pos;

		inline XmlNodePrivate()
		{
			CDATA = false;
			attr_find_pos = 0;
			node_find_pos = 0;
		}

		inline XmlAttribute *new_attr(const std::string &name, const std::string &value)
		{
			return insert_attr(-1, name, value);
		}

		inline XmlAttribute *insert_attr(int idx, const std::string &name, const std::string &value)
		{
			if (idx == -1)
				idx = attrs.size();
			auto a = new XmlAttributePrivate;
			a->name = name;
			a->value = value;
			attrs.emplace(attrs.begin() + idx, a);
			return a;
		}

		inline void remove_attr(int idx)
		{
			attrs.erase(attrs.begin() + idx);
		}

		inline void remove_attr(XmlAttribute *a)
		{
			for (auto it = attrs.begin(); it != attrs.end(); it++)
			{
				if (it->get() == a)
				{
					attrs.erase(it);
					return;
				}
			}
		}

		inline void clear_attrs()
		{
			attrs.clear();
		}

		inline XmlAttribute *find_attr(const std::string &name)
		{
			if (attrs.empty())
				return nullptr;

			auto p = attr_find_pos;
			while (true)
			{
				if (attrs[attr_find_pos]->name == name)
				{
					auto t = attr_find_pos;
					attr_find_pos++;
					if (attr_find_pos >= attrs.size())
						attr_find_pos = 0;
					return attrs[t].get();
				}
				attr_find_pos++;
				if (attr_find_pos >= attrs.size())
					attr_find_pos = 0;
				if (attr_find_pos == p)
					return nullptr;
			}
			return nullptr;
		}

		inline XmlNode *new_node(const std::string &name)
		{
			return insert_node(-1, name);
		}

		inline XmlNode *insert_node(int idx, const std::string &name)
		{
			if (idx == -1)
				idx = nodes.size();
			auto n = new XmlNodePrivate;
			n->name = name;
			nodes.emplace(nodes.begin() + idx, n);
			return n;
		}

		inline void remove_node(int idx)
		{
			nodes.erase(nodes.begin() + idx);
		}

		inline void remove_node(XmlNode *n)
		{
			for (auto it = nodes.begin(); it != nodes.end(); it++)
			{
				if (it->get() == n)
				{
					nodes.erase(it);
					return;
				}
			}
		}

		inline void clear_nodes()
		{
			nodes.clear();
		}

		inline XmlNode *find_node(const std::string &name)
		{
			if (nodes.empty())
				return nullptr;

			auto p = node_find_pos;
			while (true)
			{
				if (nodes[node_find_pos]->name == name)
				{
					auto t = node_find_pos;
					node_find_pos++;
					if (node_find_pos >= nodes.size())
						node_find_pos = 0;
					return nodes[t].get();
				}
				node_find_pos++;
				if (node_find_pos == p)
					return nullptr;
				if (node_find_pos >= nodes.size())
					node_find_pos = 0;
			}
			return nullptr;
		}
	};

	const std::string &XmlNode::name() const
	{
		return ((XmlNodePrivate*)this)->name;
	}

	const std::string &XmlNode::value() const
	{
		return ((XmlNodePrivate*)this)->value;
	}

	void XmlNode::set_name(const std::string &name)
	{
		((XmlNodePrivate*)this)->name = name;
	}

	void XmlNode::set_value(const std::string &value)
	{
		((XmlNodePrivate*)this)->value = value;
	}

	XmlAttribute *XmlNode::new_attr(const std::string &name, const std::string &value)
	{
		return ((XmlNodePrivate*)this)->new_attr(name, value);
	}

	XmlAttribute *XmlNode::insert_attr(int idx, const std::string &name, const std::string &value)
	{
		return ((XmlNodePrivate*)this)->insert_attr(idx, name, value);
	}

	void XmlNode::remove_attr(int idx)
	{
		((XmlNodePrivate*)this)->remove_attr(idx);
	}

	void XmlNode::remove_attr(XmlAttribute *a)
	{
		((XmlNodePrivate*)this)->remove_attr(a);
	}

	void XmlNode::clear_attrs()
	{
		((XmlNodePrivate*)this)->clear_attrs();
	}

	int XmlNode::attr_count() const
	{
		return ((XmlNodePrivate*)this)->attrs.size();
	}

	XmlAttribute *XmlNode::attr(int idx) const
	{
		return ((XmlNodePrivate*)this)->attrs[idx].get();
	}

	XmlAttribute *XmlNode::find_attr(const std::string &name)
	{
		return ((XmlNodePrivate*)this)->find_attr(name);
	}

	XmlNode *XmlNode::new_node(const std::string &name)
	{
		return ((XmlNodePrivate*)this)->new_node(name);
	}

	XmlNode *XmlNode::insert_node(int idx, const std::string &name)
	{
		return ((XmlNodePrivate*)this)->insert_node(idx, name);
	}

	void XmlNode::remove_node(int idx)
	{
		((XmlNodePrivate*)this)->remove_node(idx);
	}

	void XmlNode::remove_node(XmlNode *n)
	{
		((XmlNodePrivate*)this)->remove_node(n);
	}

	void XmlNode::clear_nodes()
	{
		((XmlNodePrivate*)this)->clear_nodes();
	}

	int XmlNode::node_count() const
	{
		return ((XmlNodePrivate*)this)->nodes.size();
	}

	XmlNode *XmlNode::node(int idx) const
	{
		return ((XmlNodePrivate*)this)->nodes[idx].get();
	}

	XmlNode *XmlNode::find_node(const std::string &name)
	{
		return ((XmlNodePrivate*)this)->find_node(name);
	}

	void xml_save(rapidxml::xml_document<> &doc, rapidxml::xml_node<> *dst, XmlNodePrivate *src)
	{
		for (auto &sa : src->attrs)
			dst->append_attribute(doc.allocate_attribute(sa->name.c_str(), sa->value.c_str()));

		for (auto &sn : src->nodes)
		{
			auto n = doc.allocate_node(sn->CDATA ? rapidxml::node_cdata : rapidxml::node_element,
				sn->name.c_str(), sn->value.c_str());
			dst->append_node(n);
			xml_save(doc, n, sn.get());
		}
	}

	void XmlFile::save(const std::wstring &filename) const
	{
		rapidxml::xml_document<> xml_doc;
		auto rn = xml_doc.allocate_node(rapidxml::node_element, ((XmlNodePrivate*)root_node)->name.c_str());
		xml_doc.append_node(rn);

		xml_save(xml_doc, rn, (XmlNodePrivate*)root_node);

		std::string str;
		rapidxml::print(std::back_inserter(str), xml_doc);

		std::ofstream file(filename);
		file.write(str.data(), str.size());
	}

	XmlFile *XmlFile::create(const std::string &root_name)
	{
		auto x = new XmlFile;
		auto n = new XmlNodePrivate;
		n->name = root_name;
		x->root_node = n;

		return x;
	}

	void xml_load(rapidxml::xml_node<> *src, XmlNode *dst)
	{
		for (auto a = src->first_attribute(); a; a = a->next_attribute())
			dst->new_attr(a->name(), a->value());

		dst->set_value(src->value());

		for (auto n = src->first_node(); n; n = n->next_sibling())
		{
			auto node = dst->new_node(n->name());
			node->CDATA = n->type() == rapidxml::node_cdata;
			xml_load(n, node);
		}
	}

	XmlFile *XmlFile::create_from_file(const std::wstring &filename)
	{
		auto content = get_file_content(filename);
		if (!content.first)
			return nullptr;

		auto x = new XmlFile;
		auto n = new XmlNodePrivate;
		x->root_node = n;

		rapidxml::xml_document<> xml_doc;
		xml_doc.parse<0>(content.first.get());

		auto rn = xml_doc.first_node();
		n->name = rn->name();
		xml_load(rn, x->root_node);

		return x;
	}

	void XmlFile::destroy(XmlFile *x)
	{
		delete (XmlNodePrivate*)x->root_node;
		delete x;
	}
}
