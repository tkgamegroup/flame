#pragma once

#include <flame/magic_macros.h>

#define DECLARE(x) decltype(x) x;
#define SET_TO_CAPTURE(x) CONCATENATE(__capture__, __LINE__).x = x;
#define GET_FROM_CAPTURE(x) auto& x = CONCATENATE(__capture__, __LINE__).x;

#define PACK_CAPTURE struct CONCATENATE(__CAPTURE__, FOR_EACH(CONCATENATE, ARGS)){FOR_EACH(DECLARE, ARGS)}CONCATENATE(__capture__, __LINE__);FOR_EACH(SET_TO_CAPTURE, ARGS)
#define EXPAND_CAPTURE auto& CONCATENATE(__capture__, __LINE__) = *(CONCATENATE(__CAPTURE__, FOR_EACH(CONCATENATE, ARGS))*)c; FOR_EACH(GET_FROM_CAPTURE, ARGS)
