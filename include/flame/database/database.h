#pragma once

#ifdef FLAME_DATABASE_MODULE
#define FLAME_DATABASE_EXPORTS __declspec(dllexport)
#else
#define FLAME_DATABASE_EXPORTS __declspec(dllimport)
#endif

#include <flame/foundation/foundation.h>

namespace flame
{
	namespace database
	{
		enum Error
		{
			NoError,
			ErrorDuplicated,
			ErrorUnknow
		};

		struct Res
		{
			uint row_count;
			char** row;

			virtual void fetch_row() = 0 ;
		};

		struct Connection
		{
			virtual Error query(const char* sql, uint* row_count = nullptr, void (*callback)(Capture& c, Res* res) = nullptr, const Capture& capture = Capture()) = 0;
			inline Error query_fmt(const char* fmt, ...)
			{
				char buf[1024];
				va_list ap;
				va_start(ap, &fmt);
				vsprintf(buf, fmt, ap);
				va_end(ap);
				return query(buf);
			}
			inline Error query_fmt(uint* row_count, const char* fmt, ...)
			{
				char buf[1024];
				va_list ap;
				va_start(ap, &fmt);
				vsprintf(buf, fmt, ap);
				va_end(ap);
				return query(buf, row_count);
			}
			inline Error query_fmt(void (*callback)(Capture& c, Res* res), const Capture& capture, const char* fmt, ...)
			{
				char buf[1024];
				va_list ap;
				va_start(ap, &fmt);
				vsprintf(buf, fmt, ap);
				va_end(ap);
				return query(buf, nullptr, callback, capture);
			}
			
			FLAME_DATABASE_EXPORTS static Connection* create(const char* db_name);
		};
	}
}
