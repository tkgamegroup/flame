#include "database_private.h"

namespace flame
{
	namespace database
	{
		Error ConnectionPrivate::query(std::string_view sql, Result& result)
		{
#if USE_MYSQL
			auto eno = mysql_query(mysql_connect, sql.data());
			if (eno)
			{
				printf("%s\n", mysql_error(mysql_connect));
				auto mysql_eno = mysql_errno(mysql_connect);
				switch (mysql_eno)
				{
				case 1062:
					return ErrorDuplicated;
				}
				return ErrorUnknow;
			}
			if (mysql_field_count(mysql_connect) > 0)
			{
				auto mysql_res = mysql_store_result(mysql_connect);
				result.resize(mysql_res->row_count);
				auto num_fields = mysql_num_fields(mysql_res);
				for (auto i = 0; i < result.size(); i++)
				{
					auto mysql_row = mysql_fetch_row(mysql_res);
					auto& dst = result[i];
					dst.resize(num_fields);
					for (auto j = 0; i < num_fields; j++)
						dst[j] = mysql_row[j];
				}
				mysql_free_result(mysql_res);
			}
			return NoError;
#endif
		}

		ConnectionPtr Connection::create(std::string_view db_name)
		{
#if USE_MYSQL
			auto connect = mysql_init(nullptr);
			auto res = mysql_real_connect(connect, "localhost", "root", "123456", db_name.data(), 3306, nullptr, 0);
			if (!res)
			{
				mysql_close(connect);
				return nullptr;
			}
			assert(mysql_query(connect, "SET NAMES UTF8;") == 0);

			auto ret = new ConnectionPrivate;
			ret->mysql_connect = connect;
			return ret;
#else
			return nullptr;
#endif
		}
	}
}
