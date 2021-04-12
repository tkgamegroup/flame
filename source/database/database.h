#pragma once

#ifdef FLAME_DATABASE_MODULE
#define FLAME_DATABASE_EXPORTS __declspec(dllexport)
template<class T, class U>
struct FlameDatabaseTypeSelector
{
	typedef U result;
};
#else
#define FLAME_DATABASE_EXPORTS __declspec(dllimport)
template<class T, class U>
struct FlameDatabaseTypeSelector
{
	typedef T result;
};
#endif

#define FLAME_DATABASE_TYPE(name) struct name; struct name##Private; \
	typedef FlameDatabaseTypeSelector<name*, name##Private*>::result name##Ptr;

#include "../foundation/foundation.h"

namespace flame
{
	namespace database
	{
		FLAME_DATABASE_TYPE(Res);
		FLAME_DATABASE_TYPE(Connection);

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
			virtual Error query(const char* sql, uint* row_count = nullptr, void (*callback)(Capture& c, ResPtr res) = nullptr, const Capture& capture = Capture()) = 0;
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
			inline Error query_fmt(void (*callback)(Capture& c, ResPtr res), const Capture& capture, const char* fmt, ...)
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
