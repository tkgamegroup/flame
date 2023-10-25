#include "sheet.h"

namespace flame
{
	struct SheetPrivate : Sheet
	{
		void clear_rows() override;
		void insert_column(const std::string& name, TypeInfo* type, int idx = -1, const std::string& default_value = "") override;
		void alter_column(uint idx, const std::string& new_name, TypeInfo* new_type, const std::string& default_value) override;
		void reorder_columns(uint target_column_index, int new_index) override;
		void remove_column(uint idx) override;
		void insert_row(int idx = -1) override;
		void remove_row(uint idx) override;

		void save(const std::filesystem::path& path) override;
	};
}
