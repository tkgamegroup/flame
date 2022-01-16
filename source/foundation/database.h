#pragma once

#include "../foundation/foundation.h"

namespace flame
{
	namespace database
	{
		FLAME_FOUNDATION_TYPE(Connection);

		enum Error
		{
			ErrorNone,
			ErrorDuplicated,
			ErrorUnknow
		};

		typedef std::vector<std::vector<std::string>> Result;

		struct Connection
		{
			virtual ~Connection() {}

			virtual Error query(std::string_view sql, Result& result) = 0;
			inline Error query(std::string_view sql)
			{
				Result result;
				return query(sql, result);
			}

			struct Create
			{
				virtual ConnectionPtr operator()(std::string_view db_name) = 0;
			};
			FLAME_FOUNDATION_API static Create& create;
		};
	}
}
