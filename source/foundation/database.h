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
			virtual Error query(std::string_view sql) = 0;
			
			FLAME_FOUNDATION_EXPORTS static Connection* create(std::string_view db_name);
		};
	}
}
