#pragma once

#include "foundation.h"

namespace flame
{
	// Reflect ctor
	struct Sheet
	{
		struct Column
		{
			std::string name;
			uint name_hash;
			TypeInfo* type;
			std::string default_value;
		};

		struct Row
		{
			std::vector<void*> datas;
		};

		std::vector<Column> columns;
		std::vector<Row> rows;
		std::unordered_map<uint, uint> columns_map;

		std::filesystem::path filename;
		std::string name;
		uint name_hash;
		uint ref = 0;

		inline int find_column(uint name) const
		{
			auto it = columns_map.find(name);
			if (it == columns_map.end())
				return -1;
			return it->second;
		}

		virtual void clear_rows() = 0;
		virtual void insert_column(const std::string& name, TypeInfo* type, int idx = -1, const std::string& default_value = "") = 0;
		virtual void alter_column(uint idx, const std::string& new_name, TypeInfo* new_type, const std::string& new_default_value = "") = 0;
		virtual void reposition_columns(uint idx1, uint idx2) = 0;
		virtual void remove_column(uint idx) = 0;
		virtual void insert_row(int idx = -1) = 0;
		virtual void remove_row(uint idx) = 0;

		virtual void save(const std::filesystem::path& path = L"") = 0;

		struct Get
		{
			virtual SheetPtr operator()(const std::filesystem::path& filename) = 0;
			virtual SheetPtr operator()(uint name) = 0;
		};
		// Reflect static
		FLAME_FOUNDATION_API static Get& get;

		struct Release
		{
			virtual void operator()(SheetPtr sheet) = 0;
		};
		// Reflect static
		FLAME_FOUNDATION_API static Release& release;
	};
}
