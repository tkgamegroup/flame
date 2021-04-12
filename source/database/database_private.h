#pragma once

#include "database.h"

#ifdef USE_MYSQL
#include <mysql.h>
#endif

namespace flame
{
	namespace database
	{
		struct ResPrivate : Res
		{
#ifdef USE_MYSQL
			MYSQL_RES* mysql_res;
#endif

			void fetch_row() override;
		};

		struct ConnectionPrivate : Connection
		{
#ifdef USE_MYSQL
			MYSQL* mysql_connect;
#endif
			Error query(const char* sql, uint* row_count, void (*callback)(Capture& c, ResPtr res), const Capture& capture) override;

			static ConnectionPrivate* create(const char* db_name);
		};
	}
}
