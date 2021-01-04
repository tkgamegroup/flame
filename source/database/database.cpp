#include <flame/foundation/foundation.h>
#include "database_private.h"

namespace flame
{
	namespace database
	{
		void ResPrivate::fetch_row()
		{
#ifdef USE_MYSQL
			row = mysql_fetch_row(mysql_res);
#endif
		}

		Error ConnectionPrivate::query(const char* sql, uint* row_count, void (*callback)(Capture& c, Res* res), const Capture& capture)
		{
#ifdef USE_MYSQL
			auto eno = mysql_query(mysql_connect, sql);
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
				auto result = mysql_store_result(mysql_connect);
				if (row_count)
					*row_count = result->row_count;
				if (callback)
				{
					auto res = new ResPrivate;
					res->row_count = result->row_count;
					res->mysql_res = result;
					callback((Capture&)capture, res);
					delete res;
					f_free(capture._data);
				}
				mysql_free_result(result);
			}
#endif
			return NoError;
		}

		ConnectionPrivate* ConnectionPrivate::create(const char* db_name)
		{
#ifdef USE_MYSQL
			auto connect = mysql_init(nullptr);
			auto res = mysql_real_connect(connect, "localhost", "root", "123456", db_name, 3306, nullptr, 0);
			if (!res)
			{
				mysql_close(connect);
				return nullptr;
			}
			fassert(mysql_query(connect, "SET NAMES UTF8;") == 0);
#endif
			auto ret = new ConnectionPrivate;
#ifdef USE_MYSQL
			ret->mysql_connect = connect;
#endif
			return ret;
		}

		Connection* Connection::create(const char* db_name)
		{
			return ConnectionPrivate::create(db_name);
		}
	}
}
