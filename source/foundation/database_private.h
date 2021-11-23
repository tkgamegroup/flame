#pragma once

#include "database.h"

#if USE_MYSQL
#include <mysql.h>
#endif

namespace flame
{
	namespace database
	{
		struct ConnectionPrivate : Connection
		{
#if USE_MYSQL
			MYSQL* mysql_connect;
#endif
			~ConnectionPrivate();

			Error query(std::string_view sql, Result& result) override;
		};
	}
}
