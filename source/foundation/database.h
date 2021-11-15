#pragma once

#include "../foundation/foundation.h"

namespace flame
{
	namespace database
	{
		FLAME_FOUNDATION_TYPE(Connection);

		enum Error
		{
			NoError,
			ErrorDuplicated,
			ErrorUnknow
		};

		typedef std::vector<std::vector<std::string>> Result;

		struct Connection
		{
			virtual Error query(std::string_view sql, Result& result) = 0;
			inline Error query(std::string_view sql)
			{
				Result result;
				return query(sql, result);
			}
			
			FLAME_FOUNDATION_EXPORTS static ConnectionPtr create(std::string_view db_name);
		};
	}
}
